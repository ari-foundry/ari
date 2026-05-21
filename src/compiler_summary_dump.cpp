#include "compiler_summary_dump.hpp"

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

} // namespace

std::string dump_compiler_stage_plan(const std::string& source_name,
                                     const std::string& target_triple,
                                     bool implicit_std,
                                     std::size_t module_search_path_count,
                                     std::size_t cfg_feature_count) {
    static const StagePlanRow rows[] = {
        {"StagePlan", "driver", "--emit-stage-plan", "make check-compiler-artifacts",
         "artifact ladder, layer owners, and first checks"},
        {"SourceMap", "driver", "--emit-source-map", "make check-compiler-artifacts",
         "source files, byte offsets, line starts, and snippets"},
        {"Tokens", "lexer", "--emit-tokens", "make check-compiler-artifacts",
         "token kinds, spellings, and byte spans"},
        {"Diagnostics", "diagnostics", "--emit-diagnostics", "make check-compiler-artifacts",
         "stable error code families and normalized messages"},
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
