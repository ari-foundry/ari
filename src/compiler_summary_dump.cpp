#include "compiler_summary_dump.hpp"

#include "common.hpp"

#include <algorithm>
#include <sstream>
#include <vector>

namespace ari {

namespace {

struct StagePlanRow {
    const char* name;
    const char* owner;
    const char* artifact;
    const char* first_check;
    const char* proves;
};

struct CompilerPassRow {
    const char* name;
    const char* layer;
    const char* owner;
    const char* input;
    const char* output;
    const char* artifact;
    const char* first_check;
    const char* purpose;
};

struct CompilerTestBucketRow {
    const char* name;
    const char* path;
    const char* kind;
    const char* owner;
    const char* first_check;
    const char* use_for;
};

struct CompilerWorkItemRow {
    const char* name;
    const char* priority;
    const char* area;
    const char* first_files;
    const char* first_artifact;
    const char* first_check;
    const char* done_when;
};

struct CapabilityRow {
    const char* name;
    const char* status;
    const char* owner;
    const char* first_check;
    const char* proves;
};

static const CompilerPassRow kPassRows[] = {
    {"source-load", "frontend", "driver/module-loader", "root path plus module paths",
     "source files with stable identities", "--emit-source-map", "make check-compiler-artifacts",
     "load files, preserve paths, and make byte offsets reviewable"},
    {"lexer", "frontend", "lexer", "source bytes",
     "tokens with byte spans", "--emit-tokens", "make check-compiler-artifacts",
     "prove token boundaries before parser behavior is involved"},
    {"parser", "frontend", "parser", "tokens",
     "AST plus parse recovery diagnostics", "--emit-syntax", "make check-compiler-artifacts",
     "prove source syntax shape before name or type resolution"},
    {"module-loader", "frontend", "module-loader", "root AST and module paths",
     "module graph and source metadata", "--emit-module-graph", "make check-compiler-artifacts",
     "prove file-backed module edges, visibility surfaces, and imports"},
    {"declaration-collector", "frontend", "declaration-collector", "module ASTs",
     "declaration index", "--emit-declaration-index", "make check-compiler-artifacts",
     "prove item signatures and visibility before expression checking"},
    {"resolved-index", "middle", "sema", "AST, modules, declarations, and cfg features",
     "resolved functions, locals, calls, enum cases, and pattern bindings", "--emit-resolved-index",
     "make check-compiler-artifacts",
     "separate resolver-level facts from full typed IR and backend lowering"},
    {"sema", "middle", "sema", "AST, modules, declarations, and cfg features",
     "typed IR plus warnings", "--emit-typed-ir", "make check-compiler-artifacts",
     "prove names, types, traits, ownership, and lowering facts"},
    {"pass-summary", "middle", "driver/sema", "tokens, AST, metadata, and typed IR",
     "stage counts", "--emit-pass-summary", "make check-compiler-artifacts",
     "summarize cross-pass shape without dumping every detail"},
    {"llvm-backend", "backend", "llvm-backend", "typed IR",
     "LLVM IR text", "--emit-llvm", "focused --emit-llvm",
     "prove backend lowering after frontend artifacts already match"},
    {"object-emission", "backend", "toolchain", "LLVM IR text",
     "object file", "--emit-obj", "focused --emit-obj",
     "prove the LLVM driver emits target objects with expected symbols"},
    {"executable-link", "backend", "toolchain", "LLVM IR or object file plus link args",
     "executable or shared library", "-o or --shared", "focused linked run",
     "prove final runtime behavior only after earlier artifacts are stable"},
};

static const CompilerTestBucketRow kTestBucketRows[] = {
    {"feature-ok", "tests/cases/<feature>/ok/", "positive-language",
     "feature owner", "build/ari path/to/case.ari --check",
     "valid language behavior that should check, emit LLVM, link, or run"},
    {"feature-errors", "tests/cases/<feature>/errors/", "negative-language",
     "feature owner", "focused failing build/ari invocation",
     "invalid source that should be rejected by the earliest compiler layer"},
    {"compiler-artifact-ok", "tests/cases/compiler-development/artifact/ok/", "golden-artifact",
     "artifact owner", "make check-compiler-artifacts",
     "deterministic compiler text artifacts that should compare cleanly"},
    {"compiler-artifact-errors", "tests/cases/compiler-development/artifact/errors/", "golden-error-artifact",
     "artifact/diagnostic owner", "make check-compiler-artifacts",
     "diagnostic artifacts or text-comparator mismatch reports that should compare cleanly"},
    {"tooling", "tests/tools/", "tooling-smoke",
     "tool owner", "make check-tools",
     "lint, LSP, editor, or other helper-tool behavior outside the core compiler pipeline"},
    {"docs", "docs/", "documentation",
     "docs owner", "make check-language-docs",
     "reader-facing language, compiler, tooling, and contributor documentation"},
};

static const CompilerWorkItemRow kWorkItemRows[] = {
    {"source-identity-hardening", "P0", "source/diagnostics",
     "src/driver.cpp, src/source_map_dump.cpp, lexer/parser diagnostic call sites",
     "--emit-source-map", "make check-compiler-artifacts",
     "diagnostics and artifacts preserve stable files, byte offsets, line columns, and snippets"},
    {"diagnostic-code-data-model", "P0", "diagnostics",
     "src/diagnostic_dump.cpp plus lexer/parser/module/sema throw sites",
     "--emit-diagnostics", "make check-compiler-artifacts",
     "expected failures carry stable code families, source fields, and reviewable messages"},
    {"test-classification", "P0", "tests",
     "tests/README.md, tests/Makefile, docs/dev/compiler-test-authoring.md",
     "--list-test-buckets", "python3 tests/check_compiler_test_bucket_cli.py",
     "new fixtures land in the closest behavior bucket with a small first check"},
    {"module-project-ergonomics", "P1", "modules",
     "src/module_loader.cpp, src/module_metadata.cpp, src/module_path.cpp",
     "--emit-module-graph", "make check-modules or make check-compiler-artifacts",
     "multi-file Ari projects fail closed with clear module, visibility, and cache diagnostics"},
    {"parser-declaration-artifacts", "P1", "frontend",
     "src/parser.cpp, src/ast.hpp, src/declaration_index_dump.cpp",
     "--emit-syntax and --emit-declaration-index", "make check-compiler-artifacts",
     "syntax and declaration surfaces expose enough data before sema changes behavior"},
    {"generic-aggregate-stress", "P1", "types/generics",
     "src/type_semantics.cpp, src/type_inference.cpp, src/sema.cpp, src/ir.hpp",
     "--emit-typed-ir", "make check-generics",
     "generic structs, enums, aliases, and nested payloads lower without one-off escapes"},
    {"trait-dispatch-maturity", "P2", "traits",
     "src/trait_semantics.cpp, src/type_semantics.cpp, src/sema.cpp",
     "--emit-typed-ir", "make check-traits",
     "trait selection for compiler-shaped code is deterministic and diagnosable"},
    {"ownership-fact-visibility", "P2", "ownership",
     "src/ownership_semantics.cpp, src/borrow_semantics.cpp, src/move_semantics.cpp",
     "future --emit-ownership-facts", "focused ownership fixtures",
     "move, borrow, drop, and branch-state decisions can be reviewed before backend output"},
    {"resolver-index-hardening", "P2", "frontend/lowering",
     "src/declaration_index_dump.cpp, src/ir_dump.cpp",
     "--emit-declaration-index", "make check-compiler-artifacts",
     "resolver-facing source and module shape stays visible between parser AST and typed IR"},
    {"backend-artifact-normalization", "P2", "backend",
     "src/llvm_codegen.cpp, src/toolchain.cpp, src/symbol_mangle.cpp",
     "--emit-llvm and --emit-obj", "focused --emit-llvm/--emit-obj",
     "ABI, symbol, object, and shared-library regressions fail before broad executable tests"},
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
    {"file-backed-modules", "implemented", "module-loader", "make check-modules",
     "module source loading, graph source ids, metadata, cache validation, cycles, ambiguity checks, and project fixtures"},
    {"prelude-io-formatting", "implemented", "prelude/sema/backend", "focused format fixtures",
     "compiler-known print, println, and formatting hooks"},
    {"extern-c-ffi", "implemented", "parser/sema/backend", "make check-ffi",
     "extern C declarations, explicit link names, and C symbol calls"},
    {"shared-library-output", "implemented", "driver/backend/toolchain", "focused --shared fixture",
     "LLVM shared library emission and exported Ari symbols"},
    {"generic-function-calls", "implemented", "sema/backend", "make check-generics",
     "declarations, explicit/inferred calls, deterministic specializations, module-qualified calls, function pointer specialization, trait bounds, and diagnostics"},
    {"structs-and-field-layout", "implemented", "parser/sema/backend", "make check-structs",
     "named structs, tuple structs, field access/assignment, aggregate layout/codegen, match destructuring, and diagnostics"},
    {"trait-resolution", "implemented", "type/trait-semantics", "make check-traits",
     "minimum static trait subset, impl conformance, deterministic dispatch, bounds, and trait diagnostics"},
    {"typed-ir-and-llvm-path", "implemented", "sema/llvm-backend", "make check-compiler-artifacts",
     "typed IR, LLVM text, object output, and executable emission on the LLVM path"},
    {"diagnostic-artifacts", "partial", "diagnostics/driver", "make check-compiler-artifacts",
     "stable diagnostic codes, catalog, source fields, and transitional message classification"},
    {"source-identity-artifacts", "partial", "driver/source-loader", "make check-compiler-artifacts",
     "source maps, byte offsets, line tables, snippets, and normalized artifact paths"},
    {"compiler-capability-inventory", "implemented", "driver", "make check-compiler-artifacts",
     "machine-readable public compiler surface for contributor triage"},
    {"resolver-facing-artifact", "implemented", "declaration-collector", "make check-compiler-artifacts",
     "module imports, uses, declarations, signatures, visibility, and stable source locations before typed IR"},
    {"generic-aggregate-monomorphization", "implemented", "type/sema/backend", "make check-generics",
     "generic structs, enums, aliases, nested payload layout, ownership, and compiler-shaped fixtures"},
    {"structural-capability-parameters", "partial", "parser/type/trait-diagnostics", "targeted parser diagnostic fixture",
     "parameter-local, generic-bound, alias-bound, and generic-alias-bound single/grouped has-method capability syntax for functions, impl methods, and trait method contracts, duplicate method-name rejection, static call-site method checks, trait-impl structural-bound matching, and trait-quality diagnostics without an interface keyword"},
    {"union-by-fields", "partial", "parser/type/ownership/backend", "make check-structs",
     "discriminant-linked union fields with stable selector validation, enum and bool selector construction via field: arm(payload) and compatibility field: arm => payload, struct-payload shorthand field: arm { ... }, bool false/true arm coverage, direct and nested selector inference, same-literal selector checks, direct field match reading, and selector/payload mutation rejection"},
    {"function-parameter-patterns", "planned", "parser/sema", "future parser and sema fixtures",
     "patterns in function parameter positions"},
    {"runtime-strings-and-floats", "planned", "runtime/backend", "future std and backend fixtures",
     "owned runtime strings and complete float value semantics"},
    {"raw-pointers-and-allocation-zones", "planned", "type/ownership", "future ownership diagnostics",
     "explicit pointer operations, allocation-zone rules, and diagnostics"},
    {"general-iterator-protocol", "planned", "traits/sema/backend", "future iterator fixtures",
     "iterator dispatch beyond compiler-known range loops"},
    {"class-keyword", "rejected", "parser/policy", "make check-errors",
     "Ari uses structs, enums, functions, and traits instead of class syntax"},
    {"interface-keyword", "rejected", "parser/policy", "make check-errors",
     "Ari uses trait for abstraction boundaries instead of interface syntax"},
};

static constexpr std::size_t capability_row_count() {
    return sizeof(kCapabilityRows) / sizeof(kCapabilityRows[0]);
}

static constexpr std::size_t pass_row_count() {
    return sizeof(kPassRows) / sizeof(kPassRows[0]);
}

static constexpr std::size_t test_bucket_row_count() {
    return sizeof(kTestBucketRows) / sizeof(kTestBucketRows[0]);
}

static constexpr std::size_t work_item_row_count() {
    return sizeof(kWorkItemRows) / sizeof(kWorkItemRows[0]);
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

static std::string bool_text(bool value) {
    return value ? "true" : "false";
}

static std::string module_name_text(const std::string& module_name) {
    return module_name.empty() ? "<root>" : module_name;
}

static std::string source_summary_key(const ModuleMetadataSource& source) {
    return source.module_name + "\t" + source.path + "\t" + (source.is_root ? "1" : "0");
}

static std::string import_summary_key(const ModuleMetadataImport& import) {
    return import.owner_module + "\t" + import.module_name + "\t" +
           import.local_name + "\t" + import.source_path + "\t" +
           (import.is_public ? "1" : "0");
}

static const CapabilityRow* find_capability_row(const std::string& name) {
    for (const CapabilityRow& row : kCapabilityRows) {
        if (name == row.name) return &row;
    }
    return nullptr;
}

static const CompilerPassRow* find_pass_row(const std::string& name) {
    for (const CompilerPassRow& row : kPassRows) {
        if (name == row.name) return &row;
    }
    return nullptr;
}

static const CompilerTestBucketRow* find_test_bucket_row(const std::string& name) {
    for (const CompilerTestBucketRow& row : kTestBucketRows) {
        if (name == row.name) return &row;
    }
    return nullptr;
}

static const CompilerWorkItemRow* find_work_item_row(const std::string& name) {
    for (const CompilerWorkItemRow& row : kWorkItemRows) {
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
        {"ResolvedIndex", "sema", "--emit-resolved-index", "make check-compiler-artifacts",
         "resolved functions, locals, calls, enum cases, and pattern bindings"},
        {"TypedIR", "sema", "--emit-typed-ir", "make check-compiler-artifacts",
         "type, trait, ownership, and lowering facts"},
        {"PassSummary", "driver/sema", "--emit-pass-summary", "make check-compiler-artifacts",
         "stage counts and pass boundaries"},
        {"CHeader", "abi-header", "--emit-c-header", "make check-compiler-artifacts",
         "C-compatible aggregate and extern surface"},
        {"LLVM", "llvm-backend", "--emit-llvm", "focused --emit-llvm",
         "backend lowering after earlier artifacts match"},
        {"LLVMFragment", "llvm-backend", "--emit-llvm-fragment", "focused --emit-llvm",
         "requested LLVM function fragments for backend review"},
        {"Object", "toolchain", "--emit-obj", "focused --emit-obj",
         "object emission and exported symbol surface"},
        {"SymbolInventory", "toolchain", "--emit-symbols", "focused --emit-obj/--shared",
         "requested object or shared-library symbol inventory"},
        {"SharedLibrary", "toolchain", "--shared", "focused shared symbol inventory",
         "shared-library export and dynamic symbol surface"},
        {"RuntimeOutput", "toolchain/runtime", "-o", "focused linked run",
         "captured stdout/stderr runtime behavior"},
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
        << "modules=required artifacts=ordered runtime_output=last\n";
    return out.str();
}

std::string dump_compiler_pass_catalog() {
    std::ostringstream out;
    out << "CompilerPassCatalog version=1 entries=" << pass_row_count() << "\n";
    for (const CompilerPassRow& row : kPassRows) {
        out << "  pass=" << row.name
            << " layer=" << row.layer
            << " owner=" << row.owner
            << " input=" << quote_field(row.input)
            << " output=" << quote_field(row.output)
            << " artifact=" << quote_field(row.artifact)
            << " first_check=" << quote_field(row.first_check)
            << " purpose=" << quote_field(row.purpose) << "\n";
    }
    out << "  Rule one_pass_owner=true earliest_artifact_first=true executable_last=true\n";
    return out.str();
}

std::string dump_compiler_pass_explanation(const std::string& pass_name) {
    const CompilerPassRow* row = find_pass_row(pass_name);
    if (!row) {
        throw CompileError("unknown compiler pass '" + pass_name + "'; use --list-passes");
    }

    std::ostringstream out;
    out << "CompilerPass version=1"
        << " pass=" << row->name
        << " layer=" << row->layer
        << " owner=" << row->owner
        << " input=" << quote_field(row->input)
        << " output=" << quote_field(row->output)
        << " artifact=" << quote_field(row->artifact)
        << " first_check=" << quote_field(row->first_check)
        << " purpose=" << quote_field(row->purpose) << "\n";
    out << "  Rule one_pass_owner=true earliest_artifact_first=true executable_last=true\n";
    return out.str();
}

std::string dump_compiler_test_bucket_catalog() {
    std::ostringstream out;
    out << "CompilerTestBucketCatalog version=1 entries=" << test_bucket_row_count() << "\n";
    for (const CompilerTestBucketRow& row : kTestBucketRows) {
        out << "  bucket=" << row.name
            << " path=" << quote_field(row.path)
            << " kind=" << row.kind
            << " owner=" << quote_field(row.owner)
            << " first_check=" << quote_field(row.first_check)
            << " use_for=" << quote_field(row.use_for) << "\n";
    }
    out << "  Rule closest_behavior_bucket=true artifact_before_executable=true docs_checked=true\n";
    return out.str();
}

std::string dump_compiler_test_bucket_explanation(const std::string& bucket_name) {
    const CompilerTestBucketRow* row = find_test_bucket_row(bucket_name);
    if (!row) {
        throw CompileError("unknown compiler test bucket '" + bucket_name +
                           "'; use --list-test-buckets");
    }

    std::ostringstream out;
    out << "CompilerTestBucket version=1"
        << " bucket=" << row->name
        << " path=" << quote_field(row->path)
        << " kind=" << row->kind
        << " owner=" << quote_field(row->owner)
        << " first_check=" << quote_field(row->first_check)
        << " use_for=" << quote_field(row->use_for) << "\n";
    out << "  Rule closest_behavior_bucket=true artifact_before_executable=true docs_checked=true\n";
    return out.str();
}

std::string dump_compiler_work_item_catalog() {
    std::ostringstream out;
    out << "CompilerWorkItemCatalog version=1 entries=" << work_item_row_count() << "\n";
    for (const CompilerWorkItemRow& row : kWorkItemRows) {
        out << "  work_item=" << row.name
            << " priority=" << row.priority
            << " area=" << row.area
            << " first_files=" << quote_field(row.first_files)
            << " first_artifact=" << quote_field(row.first_artifact)
            << " first_check=" << quote_field(row.first_check)
            << " done_when=" << quote_field(row.done_when) << "\n";
    }
    out << "  Rule ordinary_compiler_work=true smallest_artifact_first=true roadmap_is_not_bootstrap=true\n";
    return out.str();
}

std::string dump_compiler_work_item_explanation(const std::string& item_name) {
    const CompilerWorkItemRow* row = find_work_item_row(item_name);
    if (!row) {
        throw CompileError("unknown compiler work item '" + item_name +
                           "'; use --list-work-items");
    }

    std::ostringstream out;
    out << "CompilerWorkItem version=1"
        << " work_item=" << row->name
        << " priority=" << row->priority
        << " area=" << row->area
        << " first_files=" << quote_field(row->first_files)
        << " first_artifact=" << quote_field(row->first_artifact)
        << " first_check=" << quote_field(row->first_check)
        << " done_when=" << quote_field(row->done_when) << "\n";
    out << "  Rule ordinary_compiler_work=true smallest_artifact_first=true roadmap_is_not_bootstrap=true\n";
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
        << " implicit_std=" << bool_text(metadata.implicit_std) << "\n";
    std::vector<ModuleMetadataSource> sources = metadata.sources;
    std::sort(sources.begin(), sources.end(), [](const auto& left, const auto& right) {
        return source_summary_key(left) < source_summary_key(right);
    });
    for (const auto& source : sources) {
        out << "    Source module=" << module_name_text(source.module_name)
            << " root=" << bool_text(source.is_root)
            << " path=" << source.path << "\n";
    }
    std::vector<ModuleMetadataImport> imports = metadata.imports;
    std::sort(imports.begin(), imports.end(), [](const auto& left, const auto& right) {
        return import_summary_key(left) < import_summary_key(right);
    });
    for (const auto& import : imports) {
        out << "    Import owner=" << module_name_text(import.owner_module)
            << " module=" << import.module_name
            << " local=" << import.local_name
            << " public=" << bool_text(import.is_public)
            << " source=" << import.source_path << "\n";
    }
    out << "  Sema functions=" << ir.functions.size()
        << " externs=" << ir.extern_functions.size()
        << " warnings=" << ir.warnings.size()
        << " require_main=" << bool_text(ir.require_main) << "\n";
    return out.str();
}

} // namespace ari
