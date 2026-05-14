#include "module_metadata.hpp"

#include "common.hpp"
#include "module_path.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace ari {

namespace {

constexpr int kModuleMetadataVersion = 2;

std::string escape_field(const std::string& text) {
    std::string escaped;
    for (char c : text) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '\t': escaped += "\\t"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            default: escaped.push_back(c); break;
        }
    }
    return escaped;
}

std::string unescape_field(const std::string& text, const std::string& display_path, std::size_t line) {
    std::string unescaped;
    for (std::size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c != '\\') {
            unescaped.push_back(c);
            continue;
        }
        if (i + 1 >= text.size()) {
            throw CompileError("invalid module metadata '" + display_path + "' at line " +
                               std::to_string(line) + ": dangling escape");
        }
        char next = text[++i];
        switch (next) {
            case '\\': unescaped.push_back('\\'); break;
            case 't': unescaped.push_back('\t'); break;
            case 'n': unescaped.push_back('\n'); break;
            case 'r': unescaped.push_back('\r'); break;
            default:
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line) + ": unknown escape");
        }
    }
    return unescaped;
}

std::vector<std::string> split_metadata_line(const std::string& line,
                                             const std::string& display_path,
                                             std::size_t line_number) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    for (;;) {
        std::size_t split = line.find('\t', start);
        std::string field = split == std::string::npos
            ? line.substr(start)
            : line.substr(start, split - start);
        fields.push_back(unescape_field(field, display_path, line_number));
        if (split == std::string::npos) break;
        start = split + 1;
    }
    return fields;
}

bool parse_bool_field(const std::string& value, const std::string& display_path, std::size_t line) {
    if (value == "0") return false;
    if (value == "1") return true;
    throw CompileError("invalid module metadata '" + display_path + "' at line " +
                       std::to_string(line) + ": expected boolean 0 or 1");
}

std::string bool_key(bool value) {
    return value ? "1" : "0";
}

std::string visibility_text(bool is_public) {
    return is_public ? "pub" : "private";
}

std::string display_module_name(const std::string& module_name) {
    return module_name.empty() ? "<root>" : module_name;
}

std::string source_key(const ModuleMetadataSource& source) {
    return source.module_name + "\t" + source.path + "\t" + bool_key(source.is_root);
}

std::string source_display(const ModuleMetadataSource& source) {
    std::string text = "'" + display_module_name(source.module_name) + "' at '" + source.path + "'";
    if (source.is_root) text += " (root)";
    return text;
}

std::string find_source_content_mismatch(const std::vector<ModuleMetadataSource>& expected,
                                         const std::vector<ModuleMetadataSource>& actual) {
    std::map<std::string, ModuleMetadataSource> expected_by_key;
    std::map<std::string, ModuleMetadataSource> actual_by_key;
    for (const auto& source : expected) expected_by_key.emplace(source_key(source), source);
    for (const auto& source : actual) actual_by_key.emplace(source_key(source), source);

    for (const auto& entry : actual_by_key) {
        auto expected_it = expected_by_key.find(entry.first);
        if (expected_it == expected_by_key.end()) continue;
        const ModuleMetadataSource& expected_source = expected_it->second;
        const ModuleMetadataSource& actual_source = entry.second;
        if (expected_source.content_hash != actual_source.content_hash) {
            return "source " + source_display(actual_source) + " content hash changed from '" +
                   (expected_source.content_hash.empty() ? "<none>" : expected_source.content_hash) +
                   "' to '" +
                   (actual_source.content_hash.empty() ? "<none>" : actual_source.content_hash) + "'";
        }
    }
    return "";
}

std::string import_key(const ModuleMetadataImport& import) {
    return import.owner_module + "\t" + import.module_name + "\t" + import.local_name +
           "\t" + import.source_path + "\t" + bool_key(import.is_public);
}

std::string import_display(const ModuleMetadataImport& import) {
    return "'" + import.module_name + " as " + import.local_name + "' in module '" +
           display_module_name(import.owner_module) + "' from '" + import.source_path +
           "' (" + visibility_text(import.is_public) + ")";
}

std::string item_key(const ModuleMetadataItem& item) {
    return item.module_name + "\t" + item.kind + "\t" + item.name + "\t" + bool_key(item.is_public);
}

std::string item_display(const ModuleMetadataItem& item) {
    return item.kind + " '" + item.name + "' in module '" +
           display_module_name(item.module_name) + "' (" + visibility_text(item.is_public) + ")";
}

template <typename Record, typename KeyFn, typename DisplayFn>
std::string find_record_mismatch(const std::vector<Record>& expected,
                                 const std::vector<Record>& actual,
                                 const std::string& label,
                                 KeyFn key_fn,
                                 DisplayFn display_fn) {
    std::map<std::string, std::size_t> expected_counts;
    std::map<std::string, std::size_t> actual_counts;
    std::map<std::string, std::string> displays;

    for (const auto& record : expected) {
        std::string key = key_fn(record);
        ++expected_counts[key];
        displays.emplace(key, display_fn(record));
    }
    for (const auto& record : actual) {
        std::string key = key_fn(record);
        ++actual_counts[key];
        displays.emplace(key, display_fn(record));
    }

    for (const auto& entry : actual_counts) {
        auto expected_it = expected_counts.find(entry.first);
        std::size_t expected_count = expected_it == expected_counts.end() ? 0 : expected_it->second;
        if (entry.second > expected_count) {
            return "current source graph has new " + label + " " + displays[entry.first];
        }
    }
    for (const auto& entry : expected_counts) {
        auto actual_it = actual_counts.find(entry.first);
        std::size_t actual_count = actual_it == actual_counts.end() ? 0 : actual_it->second;
        if (entry.second > actual_count) {
            return "metadata still lists " + label + " " + displays[entry.first] +
                   " missing from the current source graph";
        }
    }
    return "";
}

std::string find_cfg_mismatch(const std::set<std::string>& expected,
                              const std::set<std::string>& actual) {
    for (const auto& feature : actual) {
        if (!expected.count(feature)) {
            return "current source graph has new cfg feature '" + feature + "'";
        }
    }
    for (const auto& feature : expected) {
        if (!actual.count(feature)) {
            return "metadata still lists cfg feature '" + feature +
                   "' missing from the current source graph";
        }
    }
    return "";
}

std::string find_search_path_mismatch(const std::vector<std::string>& expected,
                                      const std::vector<std::string>& actual) {
    std::size_t shared_count = std::min(expected.size(), actual.size());
    for (std::size_t i = 0; i < shared_count; ++i) {
        if (expected[i] != actual[i]) {
            return "module search path #" + std::to_string(i + 1) + " changed from '" +
                   expected[i] + "' to '" + actual[i] + "'";
        }
    }
    if (actual.size() > expected.size()) {
        return "current source graph has new module search path '" + actual[expected.size()] + "'";
    }
    if (expected.size() > actual.size()) {
        return "metadata still lists module search path '" + expected[actual.size()] +
               "' missing from the current source graph";
    }
    return "";
}

std::string find_module_metadata_mismatch(const ModuleMetadata& expected,
                                          const ModuleMetadata& actual) {
    std::string detail = find_search_path_mismatch(expected.module_search_paths, actual.module_search_paths);
    if (!detail.empty()) return detail;

    detail = find_cfg_mismatch(expected.cfg_features, actual.cfg_features);
    if (!detail.empty()) return detail;

    if (expected.target_triple != actual.target_triple) {
        return "target option changed from '" + expected.target_triple + "' to '" + actual.target_triple + "'";
    }

    if (expected.implicit_std != actual.implicit_std) {
        return "implicit_std option changed from " + bool_key(expected.implicit_std) +
               " to " + bool_key(actual.implicit_std);
    }

    detail = find_record_mismatch(expected.sources, actual.sources, "source", source_key, source_display);
    if (!detail.empty()) return detail;

    detail = find_source_content_mismatch(expected.sources, actual.sources);
    if (!detail.empty()) return detail;

    detail = find_record_mismatch(expected.imports, actual.imports, "import", import_key, import_display);
    if (!detail.empty()) return detail;

    detail = find_record_mismatch(expected.items, actual.items, "item", item_key, item_display);
    if (!detail.empty()) return detail;

    return "metadata records changed order or duplicate counts";
}

std::string type_ref_summary(const TypeRef& type) {
    std::string name;
    switch (type.qualifier) {
        case TypeQualifier::Value: break;
        case TypeQualifier::Own: name += "own "; break;
        case TypeQualifier::Ref: name += "ref "; break;
        case TypeQualifier::MutRef: name += "ref mut "; break;
        case TypeQualifier::Ptr: name += "ptr "; break;
    }
    if (type.is_dyn_object) name += "dyn ";
    if (type.name == "Array") {
        name += "[" + type_ref_summary(type.args.empty() ? TypeRef{} : type.args[0]) +
                ", " + std::to_string(type.array_size) + "]";
        if (type.nullable) name += "?";
        return name;
    }
    if (type.is_macro_invocation) {
        name += type.name;
        name += "!(...)";
        if (type.nullable) name += "?";
        return name;
    }
    name += type.name;
    if (!type.args.empty()) {
        name += "[";
        for (std::size_t i = 0; i < type.args.size(); ++i) {
            if (i > 0) name += ", ";
            name += type_ref_summary(type.args[i]);
        }
        name += "]";
    }
    if (type.has_associated_projection) {
        name += "::";
        name += type.associated_projection;
    }
    if (type.nullable) name += "?";
    return name;
}

void add_item(ModuleMetadata& metadata,
              std::string module_name,
              std::string kind,
              std::string name,
              bool is_public) {
    metadata.items.push_back(ModuleMetadataItem{
        std::move(module_name),
        std::move(kind),
        std::move(name),
        is_public,
    });
}

void write_line(std::ostringstream& out, const std::vector<std::string>& fields) {
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) out << '\t';
        out << escape_field(fields[i]);
    }
    out << '\n';
}

} // namespace

void collect_module_metadata_source(ModuleMetadata& metadata,
                                    const std::string& path,
                                    std::string content_hash,
                                    const std::vector<std::string>& module_path,
                                    const Program& program,
                                    bool is_root) {
    std::string module_name = join_qualified_path(module_path);
    metadata.sources.push_back(ModuleMetadataSource{module_name, path, std::move(content_hash), is_root});

    for (const auto& decl : program.modules) {
        add_item(metadata, decl.module_name, "module", decl.name, decl.is_public);
    }
    for (const auto& decl : program.uses) {
        std::string name = decl.is_glob ? decl.path + "::*" : decl.path + " as " + decl.alias;
        add_item(metadata, decl.module_name, "use", std::move(name), decl.is_public);
    }
    for (const auto& decl : program.constants) {
        add_item(metadata, decl.module_name, "const", decl.name, decl.is_public);
    }
    for (const auto& decl : program.functions) {
        std::string kind = decl.is_extern ? "extern-" + decl.extern_abi + "-fn" : (decl.meta ? "meta-fn" : "fn");
        add_item(metadata, decl.module_name, std::move(kind), decl.name, decl.is_public);
    }
    for (const auto& decl : program.structs) {
        add_item(metadata, decl.module_name, "struct", decl.name, decl.is_public);
    }
    for (const auto& decl : program.enums) {
        add_item(metadata, decl.module_name, "enum", decl.name, decl.is_public);
    }
    for (const auto& decl : program.traits) {
        add_item(metadata, decl.module_name, "trait", decl.name, decl.is_public);
    }
    for (const auto& decl : program.impls) {
        std::string name = decl.has_trait
            ? type_ref_summary(decl.trait_type) + " for " + type_ref_summary(decl.for_type)
            : type_ref_summary(decl.for_type);
        add_item(metadata, decl.module_name, decl.has_trait ? "trait-impl" : "impl", std::move(name), decl.is_public);
    }
}

void add_module_metadata_import(ModuleMetadata& metadata,
                                const ModuleImport& import,
                                const std::string& source_path) {
    metadata.imports.push_back(ModuleMetadataImport{
        import.module_name,
        import.name,
        import.local_name,
        source_path,
        import.is_public,
    });
}

std::string module_metadata_source_hash(const std::string& source) {
    std::uint64_t hash = 14695981039346656037ull;
    for (unsigned char c : source) {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= 1099511628211ull;
    }

    std::ostringstream out;
    out << std::hex << std::setw(16) << std::setfill('0') << hash;
    return out.str();
}

std::string serialize_module_metadata(const ModuleMetadata& metadata) {
    std::ostringstream out;
    out << "ari-module-metadata-v" << kModuleMetadataVersion << "\n";
    for (const auto& path : metadata.module_search_paths) {
        write_line(out, {"search", path});
    }
    for (const auto& feature : metadata.cfg_features) {
        write_line(out, {"cfg", feature});
    }
    if (!metadata.target_triple.empty()) {
        write_line(out, {"option", "target", metadata.target_triple});
    }
    write_line(out, {"option", "implicit_std", metadata.implicit_std ? "1" : "0"});
    for (const auto& source : metadata.sources) {
        write_line(out, {"source", source.module_name, source.path, source.is_root ? "1" : "0", source.content_hash});
    }
    for (const auto& import : metadata.imports) {
        write_line(out, {
            "import",
            import.owner_module,
            import.module_name,
            import.local_name,
            import.source_path,
            import.is_public ? "1" : "0",
        });
    }
    for (const auto& item : metadata.items) {
        write_line(out, {"item", item.module_name, item.kind, item.name, item.is_public ? "1" : "0"});
    }
    return out.str();
}

ModuleMetadata parse_module_metadata_text(const std::string& text, const std::string& display_path) {
    std::istringstream in(text);
    std::string line;
    std::size_t line_number = 0;
    ModuleMetadata metadata;
    bool saw_header = false;
    std::set<std::string> seen_sources;
    std::set<std::string> seen_imports;
    std::set<std::string> seen_items;

    while (std::getline(in, line)) {
        ++line_number;
        if (!saw_header) {
            if (line == "ari-module-metadata-v1") {
                metadata.format_version = 1;
            } else if (line == "ari-module-metadata-v2") {
                metadata.format_version = 2;
            } else {
                throw CompileError("invalid module metadata '" + display_path + "': expected ari-module-metadata-v1 or ari-module-metadata-v2 header");
            }
            saw_header = true;
            continue;
        }
        if (line.empty()) continue;
        std::vector<std::string> fields = split_metadata_line(line, display_path, line_number);
        const std::string& tag = fields[0];
        if (tag == "search") {
            if (fields.size() != 2) {
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": malformed search record");
            }
            metadata.module_search_paths.push_back(fields[1]);
        } else if (tag == "cfg") {
            if (fields.size() != 2) {
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": malformed cfg record");
            }
            metadata.cfg_features.insert(fields[1]);
        } else if (tag == "option") {
            if (fields.size() != 3) {
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": malformed option record");
            }
            if (fields[1] == "implicit_std") {
                metadata.implicit_std = parse_bool_field(fields[2], display_path, line_number);
            } else if (fields[1] == "target") {
                metadata.target_triple = fields[2];
            } else {
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": unknown option");
            }
        } else if (tag == "source") {
            std::size_t expected_fields = metadata.format_version >= 2 ? 5 : 4;
            if (fields.size() != expected_fields) {
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": malformed source record");
            }
            ModuleMetadataSource source{
                fields[1],
                fields[2],
                fields.size() == 5 ? fields[4] : "",
                parse_bool_field(fields[3], display_path, line_number),
            };
            if (!seen_sources.insert(source_key(source)).second) {
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line_number) +
                                   ": duplicate source record for " + source_display(source));
            }
            metadata.sources.push_back(std::move(source));
        } else if (tag == "import") {
            if (fields.size() != 6) {
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": malformed import record");
            }
            ModuleMetadataImport import{
                fields[1],
                fields[2],
                fields[3],
                fields[4],
                parse_bool_field(fields[5], display_path, line_number),
            };
            if (!seen_imports.insert(import_key(import)).second) {
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line_number) +
                                   ": duplicate import record for " + import_display(import));
            }
            metadata.imports.push_back(std::move(import));
        } else if (tag == "item") {
            if (fields.size() != 5) {
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": malformed item record");
            }
            ModuleMetadataItem item{
                fields[1],
                fields[2],
                fields[3],
                parse_bool_field(fields[4], display_path, line_number),
            };
            if (!seen_items.insert(item_key(item)).second) {
                throw CompileError("invalid module metadata '" + display_path + "' at line " +
                                   std::to_string(line_number) +
                                   ": duplicate item record for " + item_display(item));
            }
            metadata.items.push_back(std::move(item));
        } else {
            throw CompileError("invalid module metadata '" + display_path + "' at line " +
                               std::to_string(line_number) + ": unknown record");
        }
    }

    if (!saw_header) {
        throw CompileError("invalid module metadata '" + display_path + "': expected ari-module-metadata-v1 or ari-module-metadata-v2 header");
    }
    return metadata;
}

ModuleMetadata read_module_metadata_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw CompileError("cannot open module metadata file '" + path + "'");
    std::ostringstream ss;
    ss << in.rdbuf();
    return parse_module_metadata_text(ss.str(), path);
}

void require_matching_module_metadata(const ModuleMetadata& expected,
                                      const ModuleMetadata& actual,
                                      const std::string& path) {
    if (expected.format_version < kModuleMetadataVersion) {
        throw CompileError("module metadata '" + path +
                           "' uses ari-module-metadata-v" + std::to_string(expected.format_version) +
                           ", which does not include source content hashes; regenerate it with --emit-module-metadata");
    }
    if (serialize_module_metadata(expected) == serialize_module_metadata(actual)) return;
    std::string detail = find_module_metadata_mismatch(expected, actual);
    throw CompileError("module metadata '" + path +
                       "' does not match the current source graph: " + detail +
                       "; regenerate it with --emit-module-metadata");
}

} // namespace ari
