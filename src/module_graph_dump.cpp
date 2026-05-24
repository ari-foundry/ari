#include "module_graph_dump.hpp"

#include <algorithm>
#include <sstream>
#include <vector>

namespace ari {

namespace {

std::string bool_text(bool value) {
    return value ? "true" : "false";
}

std::string module_name_text(const std::string& module_name) {
    return module_name.empty() ? "<root>" : module_name;
}

std::string visibility_text(bool is_public) {
    return is_public ? "pub" : "private";
}

std::string source_key(const ModuleMetadataSource& source) {
    return source.module_name + "\t" + source.path + "\t" + (source.is_root ? "1" : "0");
}

std::string import_key(const ModuleMetadataImport& import) {
    return import.owner_module + "\t" + import.module_name + "\t" + import.local_name +
           "\t" + import.source_path + "\t" + (import.is_public ? "1" : "0");
}

std::string item_key(const ModuleMetadataItem& item) {
    return item.module_name + "\t" + item.kind + "\t" + item.name + "\t" +
           item.origin + "\t" + (item.is_public ? "1" : "0");
}

} // namespace

std::string dump_module_graph(const ModuleMetadata& metadata, const std::string& source_name) {
    std::vector<ModuleMetadataSource> sources = metadata.sources;
    std::vector<ModuleMetadataImport> imports = metadata.imports;
    std::vector<ModuleMetadataItem> items = metadata.items;

    std::sort(sources.begin(), sources.end(), [](const auto& left, const auto& right) {
        return source_key(left) < source_key(right);
    });
    std::sort(imports.begin(), imports.end(), [](const auto& left, const auto& right) {
        return import_key(left) < import_key(right);
    });
    std::sort(items.begin(), items.end(), [](const auto& left, const auto& right) {
        return item_key(left) < item_key(right);
    });

    std::ostringstream out;
    out << "ModuleGraph source=" << source_name
        << " target=" << metadata.target_triple
        << " implicit_std=" << bool_text(metadata.implicit_std) << "\n";

    out << "  SearchPaths count=" << metadata.module_search_paths.size() << "\n";
    for (const auto& path : metadata.module_search_paths) {
        out << "    SearchPath path=" << path << "\n";
    }

    out << "  CfgFeatures count=" << metadata.cfg_features.size() << "\n";
    for (const auto& feature : metadata.cfg_features) {
        out << "    CfgFeature name=" << feature << "\n";
    }

    out << "  Sources count=" << sources.size() << "\n";
    for (std::size_t i = 0; i < sources.size(); ++i) {
        const auto& source = sources[i];
        SourceId source_id = source_id_for_name(source.path);
        out << "    Source ordinal=" << i
            << " source_id=" << source_id_text(source_id)
            << " module=" << module_name_text(source.module_name)
            << " root=" << bool_text(source.is_root)
            << " path=" << source.path << "\n";
    }

    out << "  Imports count=" << imports.size() << "\n";
    for (const auto& import : imports) {
        out << "    Import owner=" << module_name_text(import.owner_module)
            << " module=" << import.module_name
            << " local=" << import.local_name
            << " visibility=" << visibility_text(import.is_public)
            << " source=" << import.source_path << "\n";
    }

    out << "  Items count=" << items.size() << "\n";
    for (const auto& item : items) {
        out << "    Item module=" << module_name_text(item.module_name)
            << " kind=" << item.kind
            << " name=" << item.name
            << " visibility=" << visibility_text(item.is_public) << "\n";
    }
    return out.str();
}

} // namespace ari
