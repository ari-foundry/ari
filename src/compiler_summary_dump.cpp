#include "compiler_summary_dump.hpp"

#include "common.hpp"

#include <sstream>

namespace ari {

namespace {

struct StagePlanRow {
    const char* name;
    const char* owner;
    const char* artifact;
    const char* first_check;
    const char* proves;
};

struct CapabilityRow {
    const char* name;
    const char* status;
    const char* owner;
    const char* first_check;
    const char* proves;
};

static const CapabilityRow kCapabilityRows[] = {
    {"functions", "implemented", "parser/sema/backend", "make check-functions",
     "function declarations, calls, returns, and main entry points"},
    {"locals-and-assignment", "implemented", "parser/sema", "make check-variables",
     "let, var, assignment, and local value flow"},
    {"control-flow", "implemented", "parser/sema/backend", "make check-control-flow",
     "if, while, break, continue, init-while-next, and range loops"},
    {"integer-and-bool-scalars", "implemented", "lexer/sema/backend", "make check-operators",
     "signed and unsigned integers, bool values, casts, comparisons, and bit operations"},
    {"ownership-checks", "implemented", "ownership/control-flow", "focused ownership fixtures",
     "own, drop, move state, and temporary borrow rejection"},
    {"non-generic-enums-and-match", "implemented", "parser/sema/backend", "make check-match",
     "one-word tagged enums and statement match lowering"},
    {"inline-modules-and-use", "implemented", "module-loader/sema", "make check-modules",
     "inline modules, pub visibility, use imports, and qualified paths"},
    {"file-backed-modules", "partial", "module-loader", "make check-compiler-artifacts",
     "module source loading, graph artifacts, metadata, and remaining project policy hardening"},
    {"prelude-io-formatting", "implemented", "prelude/sema/backend", "focused format fixtures",
     "compiler-known print, println, and formatting hooks"},
    {"extern-c-ffi", "implemented", "parser/sema/backend", "make check-ffi",
     "extern C declarations, explicit link names, and C symbol calls"},
    {"shared-library-output", "implemented", "driver/backend/toolchain", "focused --shared fixture",
     "LLVM shared library emission and exported Ari symbols"},
    {"generic-function-calls", "partial", "sema/backend", "make check-generics",
     "simple function-call monomorphization without full generic aggregate coverage"},
    {"structs-and-field-layout", "partial", "parser/sema/backend", "make check-structs",
     "struct syntax and selected layout paths, with broader layout rules still hardening"},
    {"trait-resolution", "partial", "type/trait-semantics", "make check-traits",
     "trait declarations and impl surfaces before full deterministic dispatch coverage"},
    {"typed-ir-and-llvm-path", "implemented", "sema/llvm-backend", "make check-compiler-artifacts",
     "typed IR, LLVM text, object output, and executable emission on the LLVM path"},
    {"diagnostic-artifacts", "partial", "diagnostics/driver", "make check-compiler-artifacts",
     "stable diagnostic codes, catalog, source fields, and transitional message classification"},
    {"source-identity-artifacts", "partial", "driver/source-loader", "make check-compiler-artifacts",
     "source maps, byte offsets, line tables, snippets, and normalized artifact paths"},
    {"compiler-capability-inventory", "implemented", "driver", "make check-compiler-artifacts",
     "machine-readable public compiler surface for contributor triage"},
    {"hir-artifact", "planned", "lowering/resolver", "future check-compiler-artifacts",
     "lowered syntax and resolver-facing node shapes before typed IR"},
    {"generic-aggregate-monomorphization", "planned", "type/sema/backend", "future generic aggregate fixtures",
     "generic structs, enums, aliases, and nested payload layout at compiler scale"},
    {"function-parameter-patterns", "planned", "parser/sema", "future parser and sema fixtures",
     "patterns in function parameter positions"},
    {"runtime-strings-and-floats", "planned", "runtime/backend", "future std and backend fixtures",
     "owned runtime strings and complete float value semantics"},
    {"raw-pointers-and-allocation-zones", "planned", "type/ownership", "future ownership diagnostics",
     "explicit pointer operations, allocation-zone rules, and diagnostics"},
    {"general-iterator-protocol", "planned", "traits/sema/backend", "future iterator fixtures",
     "iterator dispatch beyond compiler-known range loops"},
    {"class-keyword", "rejected", "parser/policy", "make check-compiler-development",
     "Ari uses structs, enums, functions, and traits instead of class syntax"},
    {"interface-keyword", "rejected", "parser/policy", "make check-compiler-development",
     "Ari uses trait for abstraction boundaries instead of interface syntax"},
};

static constexpr std::size_t capability_row_count() {
    return sizeof(kCapabilityRows) / sizeof(kCapabilityRows[0]);
}

static std::string quote_field(const char* text) {
    std::string escaped = "\"";
    for (const char* cursor = text; *cursor != '\0'; ++cursor) {
        if (*cursor == '"' || *cursor == '\\') escaped.push_back('\\');
        escaped.push_back(*cursor);
    }
    escaped.push_back('"');
    return escaped;
}

static const CapabilityRow* find_capability_row(const std::string& name) {
    for (const CapabilityRow& row : kCapabilityRows) {
        if (name == row.name) return &row;
    }
    return nullptr;
}

} // namespace

std::string dump_compiler_stage_plan(const std::string& source_name,
                                     const std::string& target_triple,
                                     bool implicit_std,
                                     std::size_t module_search_path_count,
                                     std::size_t cfg_feature_count) {
    static const StagePlanRow rows[] = {
        {"StagePlan", "driver", "--emit-stage-plan", "make check-compiler-artifacts",
         "artifact ladder, layer owners, and first checks"},
        {"CapabilityInventory", "driver", "--emit-capability-inventory", "make check-compiler-artifacts",
         "implemented, partial, planned, and rejected compiler capabilities"},
        {"SourceMap", "driver", "--emit-source-map", "make check-compiler-artifacts",
         "source files, byte offsets, line starts, and snippets"},
        {"Tokens", "lexer", "--emit-tokens", "make check-compiler-artifacts",
         "token kinds, spellings, and byte spans"},
        {"Diagnostics", "diagnostics", "--emit-diagnostics", "make check-compiler-artifacts",
         "stable error code families and normalized messages"},
        {"DiagnosticCatalog", "diagnostics", "--emit-diagnostic-catalog", "make check-compiler-artifacts",
         "diagnostic codes, layer families, and owning source files"},
        {"Syntax", "parser", "--emit-syntax", "make check-compiler-artifacts",
         "AST shape and parser recovery"},
        {"ModuleGraph", "module-loader", "--emit-module-graph", "make check-compiler-artifacts",
         "file-backed modules, imports, visibility, and item surfaces"},
        {"DeclarationIndex", "declaration-collector", "--emit-declaration-index", "make check-compiler-artifacts",
         "declaration signatures, visibility, and source locations"},
        {"FutureHIR", "lowering/resolver", "planned --emit-hir", "future check-compiler-artifacts",
         "lowered syntax and resolver-facing node shapes"},
        {"TypedIR", "sema", "--emit-typed-ir", "make check-compiler-artifacts",
         "type, trait, ownership, and lowering facts"},
        {"PassSummary", "driver/sema", "--emit-pass-summary", "make check-compiler-artifacts",
         "stage counts and pass boundaries"},
        {"LLVM", "llvm-backend", "--emit-llvm", "focused --emit-llvm",
         "backend lowering after earlier artifacts match"},
        {"Object", "toolchain", "--emit-obj", "focused --emit-obj",
         "object emission and exported symbol surface"},
        {"Executable", "toolchain", "-o", "focused linked run",
         "final runtime behavior"},
    };

    std::ostringstream out;
    out << "CompilerStagePlan source=" << source_name
        << " target=" << target_triple
        << " implicit_std=" << (implicit_std ? "true" : "false")
        << " module_search_paths=" << module_search_path_count
        << " cfg_features=" << cfg_feature_count << "\n";
    for (std::size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); ++i) {
        const StagePlanRow& row = rows[i];
        out << "  " << (i + 1) << " " << row.name
            << " owner=" << row.owner
            << " artifact=" << row.artifact
            << " first_check=\"" << row.first_check << "\""
            << " proves=\"" << row.proves << "\"\n";
    }
    out << "  DevelopmentGate source_identity=required diagnostics=required "
        << "modules=required artifacts=ordered executable=last\n";
    return out.str();
}

std::string dump_compiler_capability_inventory(const std::string& target_triple,
                                               bool implicit_std) {
    std::ostringstream out;
    out << "CompilerCapabilityInventory version=1"
        << " target=" << target_triple
        << " implicit_std=" << (implicit_std ? "true" : "false")
        << " entries=" << capability_row_count() << "\n";
    for (const CapabilityRow& row : kCapabilityRows) {
        out << "  capability=" << row.name
            << " status=" << row.status
            << " owner=" << row.owner
            << " first_check=" << quote_field(row.first_check)
            << " proves=" << quote_field(row.proves) << "\n";
    }
    return out.str();
}

std::string dump_compiler_capability_explanation(const std::string& capability_name) {
    const CapabilityRow* row = find_capability_row(capability_name);
    if (!row) {
        throw CompileError("unknown compiler capability '" + capability_name +
                           "'; use --list-capabilities");
    }

    std::ostringstream out;
    out << "CompilerCapability version=1"
        << " capability=" << row->name
        << " status=" << row->status
        << " owner=" << row->owner
        << " first_check=" << quote_field(row->first_check)
        << " proves=" << quote_field(row->proves) << "\n";
    out << "  Rule status_values=[implemented, partial, planned, rejected]"
        << " ordinary_compiler_work=true\n";
    return out.str();
}

std::string dump_compiler_pass_summary(const std::string& source_name,
                                       std::size_t token_count,
                                       const Program& program,
                                       const ModuleMetadata& metadata,
                                       const IrProgram& ir) {
    std::ostringstream out;
    out << "CompilerPassSummary source=" << source_name
        << " target=" << ir.target_triple << "\n";
    out << "  Lex root_tokens=" << token_count << " includes_end=true\n";
    out << "  Syntax functions=" << program.functions.size()
        << " structs=" << program.structs.size()
        << " enums=" << program.enums.size()
        << " traits=" << program.traits.size()
        << " impls=" << program.impls.size()
        << " type_aliases=" << program.type_aliases.size()
        << " consts=" << program.constants.size()
        << " modules=" << program.modules.size()
        << " imports=" << program.module_imports.size()
        << " uses=" << program.uses.size()
        << " item_macros=" << program.item_macros.size() << "\n";
    out << "  Modules sources=" << metadata.sources.size()
        << " imports=" << metadata.imports.size()
        << " items=" << metadata.items.size()
        << " search_paths=" << metadata.module_search_paths.size()
        << " cfg_features=" << metadata.cfg_features.size()
        << " implicit_std=" << (metadata.implicit_std ? "true" : "false") << "\n";
    out << "  Sema functions=" << ir.functions.size()
        << " externs=" << ir.extern_functions.size()
        << " warnings=" << ir.warnings.size()
        << " require_main=" << (ir.require_main ? "true" : "false") << "\n";
    return out.str();
}

} // namespace ari
