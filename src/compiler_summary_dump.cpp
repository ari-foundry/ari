#include "compiler_summary_dump.hpp"

#include <sstream>

namespace ari {

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
