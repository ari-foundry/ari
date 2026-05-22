#include "driver.hpp"

#include "common.hpp"
#include "c_header.hpp"
#include "compiler_summary_dump.hpp"
#include "declaration_index_dump.hpp"
#include "diagnostic_dump.hpp"
#include "ir_dump.hpp"
#include "lexer.hpp"
#include "llvm_codegen.hpp"
#include "module_cache.hpp"
#include "module_graph_dump.hpp"
#include "module_ir_replay.hpp"
#include "module_ir_summary.hpp"
#include "module_loader.hpp"
#include "module_metadata.hpp"
#include "parser.hpp"
#include "sema.hpp"
#include "source_map_dump.hpp"
#include "syntax_dump.hpp"
#include "target.hpp"
#include "token_dump.hpp"
#include "toolchain.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <sys/stat.h>

namespace ari {

static void make_executable(const std::string& path) {
    if (chmod(path.c_str(), 0755) != 0) {
        throw CompileError("cannot set executable permissions on '" + path + "': " + std::strerror(errno));
    }
}

static void write_text_file(const std::string& path, const std::string& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw CompileError("cannot open output file '" + path + "'");
    out << data;
}

static std::string read_text_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw CompileError("cannot open input file '" + path + "'");
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

static std::string shell_quote(const std::string& text) {
    std::string quoted = "'";
    for (char c : text) {
        if (c == '\'') quoted += "'\\''";
        else quoted.push_back(c);
    }
    quoted += "'";
    return quoted;
}

static std::string join_option_names(const std::vector<std::string>& options) {
    std::string joined;
    for (std::size_t i = 0; i < options.size(); ++i) {
        if (i != 0) joined += ", ";
        joined += options[i];
    }
    return joined;
}

struct ArtifactHelpRow {
    const char* option;
    const char* owner;
    const char* first_check;
    const char* purpose;
};

static const ArtifactHelpRow kArtifactHelp[] = {
    {"--emit-stage-plan", "driver", "make check-compiler-artifacts",
     "artifact ladder, layer owners, and first checks"},
    {"--emit-capability-inventory", "driver", "make check-compiler-artifacts",
     "implemented, partial, planned, and rejected compiler capabilities"},
    {"--emit-source-map", "driver", "make check-compiler-artifacts",
     "source files, byte offsets, line starts, and snippets"},
    {"--emit-tokens", "lexer", "make check-compiler-artifacts",
     "token kinds, spellings, and byte spans"},
    {"--emit-diagnostics", "diagnostics", "make check-compiler-artifacts",
     "stable error code families and normalized messages"},
    {"--emit-diagnostic-catalog", "diagnostics", "make check-compiler-artifacts",
     "diagnostic codes, layer families, and owning source files"},
    {"--emit-syntax", "parser", "make check-compiler-artifacts",
     "AST shape and parser recovery"},
    {"--emit-module-graph", "module-loader", "make check-compiler-artifacts",
     "file-backed modules, imports, visibility, and item surfaces"},
    {"--emit-declaration-index", "declaration-collector", "make check-compiler-artifacts",
     "declaration signatures, visibility, and source locations"},
    {"--emit-typed-ir", "sema", "make check-compiler-artifacts",
     "type, trait, ownership, and lowering facts"},
    {"--emit-pass-summary", "driver/sema", "make check-compiler-artifacts",
     "stage counts and pass boundaries"},
};

static void usage(std::ostream& out) {
    out << "usage: ari <input.ari> [-o output] [--check] [--emit-llvm path]\n"
           "           [--emit-obj path] [--emit-tokens path] [--emit-syntax path]\n"
           "           [--emit-diagnostics path] [--emit-source-map path]\n"
           "           [--emit-diagnostic-catalog path]\n"
           "           [--emit-capability-inventory path]\n"
           "           [--emit-module-graph path] [--emit-declaration-index path]\n"
           "           [--emit-typed-ir path] [--emit-pass-summary path]\n"
           "           [--emit-stage-plan path]\n"
           "           [--module-path path] [-I path] [--llvm-cc compiler]\n"
           "           [--target triple]\n"
           "           [--emit-c-header path]\n"
           "           [--emit-module-metadata path] [--check-module-metadata path]\n"
           "           [--emit-module-cache path] [--use-module-cache path]\n"
           "           [--no-implicit-std]\n"
           "           [-L path] [-l name] [--link name] [--shared]\n"
           "           [--test] [--test-filter name] [--cfg-feature name]\n"
           "       ari test <input.ari> [-o output] [--filter name]\n"
           "       ari --help\n"
           "       ari --list-artifacts\n"
           "       ari --explain-artifact option\n"
           "       ari --list-passes\n"
           "       ari --explain-pass name\n"
           "       ari --list-test-buckets\n"
           "       ari --explain-test-bucket name\n"
           "       ari --list-work-items\n"
           "       ari --explain-work-item name\n"
           "       ari --list-capabilities [--target triple] [--no-implicit-std]\n"
           "       ari --explain-capability name\n"
           "       ari --list-diagnostics\n"
           "       ari --explain-diagnostic code\n"
           "       ari --target-info [--target triple]\n";
}

static void list_artifacts(std::ostream& out) {
    out << "CompilerArtifacts version=1 entries="
        << (sizeof(kArtifactHelp) / sizeof(kArtifactHelp[0])) << "\n";
    for (const ArtifactHelpRow& row : kArtifactHelp) {
        out << "  option=" << row.option
            << " owner=" << row.owner
            << " first_check=\"" << row.first_check << "\""
            << " purpose=\"" << row.purpose << "\"\n";
    }
    out << "  Rule one_artifact_output=true backend_outputs_separate=true\n";
}

static std::string normalize_artifact_option(std::string option) {
    if (option.rfind("--", 0) != 0) option = "--" + option;
    return option;
}

static const ArtifactHelpRow* find_artifact_help(const std::string& option) {
    const std::string normalized = normalize_artifact_option(option);
    for (const ArtifactHelpRow& row : kArtifactHelp) {
        if (normalized == row.option) return &row;
    }
    return nullptr;
}

static void explain_artifact(std::ostream& out, const std::string& option) {
    const std::string normalized = normalize_artifact_option(option);
    const ArtifactHelpRow* row = find_artifact_help(normalized);
    if (!row) {
        throw CompileError("unknown compiler artifact option '" + normalized +
                           "'; use --list-artifacts");
    }
    out << "CompilerArtifact version=1"
        << " option=" << row->option
        << " owner=" << row->owner
        << " first_check=\"" << row->first_check << "\""
        << " purpose=\"" << row->purpose << "\"\n";
    out << "  Rule earliest_layer=true one_artifact_output=true\n";
}

int run(int argc, char** argv) {
    if (argc < 2) {
        usage(std::cerr);
        return 2;
    }
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            usage(std::cout);
            return 0;
        }
        if (arg == "--list-artifacts") {
            list_artifacts(std::cout);
            return 0;
        }
        if (arg == "--explain-artifact") {
            if (i + 1 >= argc) throw CompileError("--explain-artifact expects an artifact option");
            explain_artifact(std::cout, argv[i + 1]);
            return 0;
        }
        if (arg == "--list-diagnostics") {
            std::cout << dump_diagnostic_catalog();
            return 0;
        }
        if (arg == "--explain-diagnostic") {
            if (i + 1 >= argc) throw CompileError("--explain-diagnostic expects a diagnostic code");
            std::cout << dump_diagnostic_explanation(argv[i + 1]);
            return 0;
        }
    }

    std::string input;
    std::string output = "a.out";
    std::string llvm_output;
    std::string object_output;
    std::string token_output;
    std::string syntax_output;
    std::string diagnostic_output;
    std::string diagnostic_catalog_output;
    std::string capability_inventory_output;
    std::string source_map_output;
    std::string module_graph_output;
    std::string declaration_index_output;
    std::string typed_ir_output;
    std::string pass_summary_output;
    std::string stage_plan_output;
    std::string c_header_output;
    std::string llvm_compiler = default_llvm_compiler();
    std::string metadata_output;
    std::string metadata_check;
    std::string module_cache_output;
    std::string module_cache_input;
    std::string target_triple;
    std::vector<std::string> module_search_paths;
    std::vector<std::string> link_args;
    std::set<std::string> cfg_features;
    bool check_only = false;
    bool output_explicit = false;
    bool emit_llvm_only = false;
    bool llvm_compiler_explicit = false;
    bool shared_library = false;
    bool test_mode = false;
    bool test_subcommand = false;
    bool implicit_std = true;
    bool target_info_requested = false;
    bool list_passes_requested = false;
    bool list_test_buckets_requested = false;
    bool list_work_items_requested = false;
    bool list_capabilities_requested = false;
    std::string pass_explanation;
    std::string test_bucket_explanation;
    std::string work_item_explanation;
    std::string capability_explanation;
    int first_arg = 1;
    if (argc > 1 && std::string(argv[1]) == "test") {
        test_mode = true;
        test_subcommand = true;
        first_arg = 2;
    }
    std::vector<std::string> test_filters;
    for (int i = first_arg; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o") {
            if (i + 1 >= argc) throw CompileError("-o expects a path");
            output = argv[++i];
            output_explicit = true;
        } else if (arg == "--check") {
            check_only = true;
        } else if (arg == "--shared") {
            shared_library = true;
        } else if (arg == "--test") {
            test_mode = true;
        } else if (arg == "--test-filter" || (test_subcommand && arg == "--filter")) {
            if (i + 1 >= argc) throw CompileError(arg + " expects a test name substring");
            test_filters.push_back(argv[++i]);
        } else if (arg == "--no-implicit-std") {
            implicit_std = false;
        } else if (arg == "--cfg-feature" || arg == "--feature") {
            if (i + 1 >= argc) throw CompileError(arg + " expects a feature name");
            cfg_features.insert(argv[++i]);
        } else if (arg == "--backend") {
            throw CompileError("--backend was removed; LLVM is the only backend");
        } else if (arg == "--emit-cpp") {
            throw CompileError("--emit-cpp was removed; use --emit-llvm");
        } else if (arg == "--keep-cpp") {
            throw CompileError("--keep-cpp was removed; Ari no longer generates C++");
        } else if (arg == "--emit-llvm") {
            if (i + 1 >= argc) throw CompileError("--emit-llvm expects a path");
            llvm_output = argv[++i];
            emit_llvm_only = true;
        } else if (arg == "--emit-obj" || arg == "--emit-object") {
            if (i + 1 >= argc) throw CompileError(arg + " expects a path");
            object_output = argv[++i];
        } else if (arg == "--emit-tokens") {
            if (i + 1 >= argc) throw CompileError("--emit-tokens expects a path");
            token_output = argv[++i];
        } else if (arg == "--emit-syntax") {
            if (i + 1 >= argc) throw CompileError("--emit-syntax expects a path");
            syntax_output = argv[++i];
        } else if (arg == "--emit-diagnostics") {
            if (i + 1 >= argc) throw CompileError("--emit-diagnostics expects a path");
            diagnostic_output = argv[++i];
        } else if (arg == "--emit-diagnostic-catalog") {
            if (i + 1 >= argc) throw CompileError("--emit-diagnostic-catalog expects a path");
            diagnostic_catalog_output = argv[++i];
        } else if (arg == "--emit-capability-inventory") {
            if (i + 1 >= argc) throw CompileError("--emit-capability-inventory expects a path");
            capability_inventory_output = argv[++i];
        } else if (arg == "--emit-source-map") {
            if (i + 1 >= argc) throw CompileError("--emit-source-map expects a path");
            source_map_output = argv[++i];
        } else if (arg == "--emit-module-graph") {
            if (i + 1 >= argc) throw CompileError("--emit-module-graph expects a path");
            module_graph_output = argv[++i];
        } else if (arg == "--emit-declaration-index") {
            if (i + 1 >= argc) throw CompileError("--emit-declaration-index expects a path");
            declaration_index_output = argv[++i];
        } else if (arg == "--emit-typed-ir") {
            if (i + 1 >= argc) throw CompileError("--emit-typed-ir expects a path");
            typed_ir_output = argv[++i];
        } else if (arg == "--emit-pass-summary") {
            if (i + 1 >= argc) throw CompileError("--emit-pass-summary expects a path");
            pass_summary_output = argv[++i];
        } else if (arg == "--emit-stage-plan") {
            if (i + 1 >= argc) throw CompileError("--emit-stage-plan expects a path");
            stage_plan_output = argv[++i];
        } else if (arg == "--emit-c-header") {
            if (i + 1 >= argc) throw CompileError("--emit-c-header expects a path");
            c_header_output = argv[++i];
        } else if (arg == "--module-path") {
            if (i + 1 >= argc) throw CompileError("--module-path expects a path");
            module_search_paths.push_back(argv[++i]);
        } else if (arg == "--emit-module-metadata") {
            if (i + 1 >= argc) throw CompileError("--emit-module-metadata expects a path");
            metadata_output = argv[++i];
        } else if (arg == "--check-module-metadata") {
            if (i + 1 >= argc) throw CompileError("--check-module-metadata expects a path");
            metadata_check = argv[++i];
        } else if (arg == "--emit-module-cache") {
            if (i + 1 >= argc) throw CompileError("--emit-module-cache expects a path");
            module_cache_output = argv[++i];
        } else if (arg == "--use-module-cache") {
            if (i + 1 >= argc) throw CompileError("--use-module-cache expects a path");
            module_cache_input = argv[++i];
        } else if (arg == "-I") {
            if (i + 1 >= argc) throw CompileError("-I expects a path");
            module_search_paths.push_back(argv[++i]);
        } else if (arg.rfind("-I", 0) == 0 && arg.size() > 2) {
            module_search_paths.push_back(arg.substr(2));
        } else if (arg == "--cc") {
            throw CompileError("--cc was removed; use --llvm-cc");
        } else if (arg == "--llvm-cc") {
            if (i + 1 >= argc) throw CompileError("--llvm-cc expects a compiler path");
            llvm_compiler = argv[++i];
            llvm_compiler_explicit = true;
        } else if (arg == "--target") {
            if (i + 1 >= argc) throw CompileError("--target expects a target triple");
            target_triple = argv[++i];
        } else if (arg == "--target-info") {
            target_info_requested = true;
        } else if (arg == "--list-passes") {
            list_passes_requested = true;
        } else if (arg == "--explain-pass") {
            if (i + 1 >= argc) throw CompileError("--explain-pass expects a pass name");
            pass_explanation = argv[++i];
        } else if (arg == "--list-test-buckets") {
            list_test_buckets_requested = true;
        } else if (arg == "--explain-test-bucket") {
            if (i + 1 >= argc) throw CompileError("--explain-test-bucket expects a test bucket name");
            test_bucket_explanation = argv[++i];
        } else if (arg == "--list-work-items") {
            list_work_items_requested = true;
        } else if (arg == "--explain-work-item") {
            if (i + 1 >= argc) throw CompileError("--explain-work-item expects a work item name");
            work_item_explanation = argv[++i];
        } else if (arg == "--list-capabilities") {
            list_capabilities_requested = true;
        } else if (arg == "--explain-capability") {
            if (i + 1 >= argc) throw CompileError("--explain-capability expects a capability name");
            capability_explanation = argv[++i];
        } else if (arg == "-L") {
            if (i + 1 >= argc) throw CompileError("-L expects a path");
            link_args.push_back(std::string("-L") + argv[++i]);
        } else if (arg.rfind("-L", 0) == 0 && arg.size() > 2) {
            link_args.push_back(arg);
        } else if (arg == "-l" || arg == "--link") {
            if (i + 1 >= argc) throw CompileError(arg + " expects a library name");
            link_args.push_back(std::string("-l") + argv[++i]);
        } else if (arg.rfind("-l", 0) == 0 && arg.size() > 2) {
            link_args.push_back(arg);
        } else if (input.empty()) {
            input = arg;
        } else {
            throw CompileError("unexpected argument '" + arg + "'");
        }
    }
    const int info_command_count = (target_info_requested ? 1 : 0) +
                                   (list_passes_requested ? 1 : 0) +
                                   (!pass_explanation.empty() ? 1 : 0) +
                                   (list_test_buckets_requested ? 1 : 0) +
                                   (!test_bucket_explanation.empty() ? 1 : 0) +
                                   (list_work_items_requested ? 1 : 0) +
                                   (!work_item_explanation.empty() ? 1 : 0) +
                                   (list_capabilities_requested ? 1 : 0) +
                                   (!capability_explanation.empty() ? 1 : 0);
    if (info_command_count > 0) {
        if (info_command_count > 1) {
            throw CompileError("compiler information commands cannot be combined");
        }
        const char* command_name = target_info_requested ? "--target-info" :
                                   list_passes_requested ? "--list-passes" :
                                   !pass_explanation.empty() ? "--explain-pass" :
                                   list_test_buckets_requested ? "--list-test-buckets" :
                                   !test_bucket_explanation.empty() ? "--explain-test-bucket" :
                                   list_work_items_requested ? "--list-work-items" :
                                   !work_item_explanation.empty() ? "--explain-work-item" :
                                   list_capabilities_requested ? "--list-capabilities" :
                                   "--explain-capability";
        if (!input.empty()) throw CompileError(std::string(command_name) + " does not take an input file");
        if (target_info_requested) {
            std::cout << dump_target_info(resolve_target_info(target_triple));
        } else if (list_passes_requested) {
            std::cout << dump_compiler_pass_catalog();
        } else if (!pass_explanation.empty()) {
            std::cout << dump_compiler_pass_explanation(pass_explanation);
        } else if (list_test_buckets_requested) {
            std::cout << dump_compiler_test_bucket_catalog();
        } else if (!test_bucket_explanation.empty()) {
            std::cout << dump_compiler_test_bucket_explanation(test_bucket_explanation);
        } else if (list_work_items_requested) {
            std::cout << dump_compiler_work_item_catalog();
        } else if (!work_item_explanation.empty()) {
            std::cout << dump_compiler_work_item_explanation(work_item_explanation);
        } else if (list_capabilities_requested) {
            TargetInfo target = resolve_target_info(target_triple);
            std::cout << dump_compiler_capability_inventory(target.triple, implicit_std);
        } else {
            std::cout << dump_compiler_capability_explanation(capability_explanation);
        }
        return 0;
    }
    if (input.empty()) throw CompileError("missing input file");
    if (shared_library && test_mode) {
        throw CompileError("--test cannot be combined with --shared");
    }
    if (!test_mode && !test_filters.empty()) {
        throw CompileError("--test-filter requires --test");
    }
    std::vector<std::string> requested_artifacts;
    const std::pair<const char*, const std::string*> artifact_options[] = {
        {"--emit-tokens", &token_output},
        {"--emit-syntax", &syntax_output},
        {"--emit-diagnostics", &diagnostic_output},
        {"--emit-diagnostic-catalog", &diagnostic_catalog_output},
        {"--emit-capability-inventory", &capability_inventory_output},
        {"--emit-source-map", &source_map_output},
        {"--emit-module-graph", &module_graph_output},
        {"--emit-declaration-index", &declaration_index_output},
        {"--emit-typed-ir", &typed_ir_output},
        {"--emit-pass-summary", &pass_summary_output},
        {"--emit-stage-plan", &stage_plan_output},
    };
    for (const auto& option : artifact_options) {
        if (!option.second->empty()) requested_artifacts.push_back(option.first);
    }

    if (check_only && (output_explicit || emit_llvm_only || shared_library ||
                       !object_output.empty() ||
                       !c_header_output.empty() || !source_map_output.empty() ||
                       !diagnostic_catalog_output.empty() ||
                       !capability_inventory_output.empty() ||
                       !module_graph_output.empty() || !declaration_index_output.empty() ||
                       !typed_ir_output.empty() ||
                       !pass_summary_output.empty() || !stage_plan_output.empty() ||
                       llvm_compiler_explicit || !link_args.empty())) {
        throw CompileError("--check cannot be combined with backend output or linking options");
    }
    TargetInfo target = resolve_target_info(target_triple);
    if (!object_output.empty() && emit_llvm_only) {
        throw CompileError("--emit-obj cannot be combined with --emit-llvm");
    }
    if (!object_output.empty() && !link_args.empty()) {
        throw CompileError("--emit-obj cannot be combined with linker options");
    }
    if ((!source_map_output.empty() || !diagnostic_catalog_output.empty() ||
         !capability_inventory_output.empty() ||
         !module_graph_output.empty() ||
         !declaration_index_output.empty() ||
         !typed_ir_output.empty() || !pass_summary_output.empty() ||
         !stage_plan_output.empty()) &&
        (output_explicit || emit_llvm_only || !object_output.empty() ||
         !c_header_output.empty() || llvm_compiler_explicit || shared_library || test_mode ||
         !metadata_output.empty() || !metadata_check.empty() ||
         !module_cache_output.empty() || !module_cache_input.empty() || !link_args.empty())) {
        throw CompileError("compiler artifact outputs cannot be combined with backend, module-cache, or linking options: " +
                           join_option_names(requested_artifacts));
    }
    if (requested_artifacts.size() > 1) {
        throw CompileError("artifact outputs cannot be combined: " +
                           join_option_names(requested_artifacts));
    }
    if (!diagnostic_catalog_output.empty()) {
        write_text_file(diagnostic_catalog_output, dump_diagnostic_catalog());
        std::cout << "wrote " << diagnostic_catalog_output << " (diagnostic catalog)\n";
        return 0;
    }
    if (!capability_inventory_output.empty()) {
        write_text_file(capability_inventory_output,
                        dump_compiler_capability_inventory(target.triple, implicit_std));
        std::cout << "wrote " << capability_inventory_output << " (compiler capability inventory)\n";
        return 0;
    }
    if (!stage_plan_output.empty()) {
        write_text_file(stage_plan_output,
                        dump_compiler_stage_plan(input, target.triple, implicit_std,
                                                 module_search_paths.size(), cfg_features.size()));
        std::cout << "wrote " << stage_plan_output << " (compiler stage plan)\n";
        return 0;
    }
    if (!token_output.empty()) {
        if (check_only || output_explicit || emit_llvm_only || !object_output.empty() ||
            !c_header_output.empty() || llvm_compiler_explicit || shared_library || test_mode ||
            !metadata_output.empty() || !metadata_check.empty() ||
            !module_cache_output.empty() || !module_cache_input.empty() ||
            !module_search_paths.empty() || !link_args.empty()) {
            throw CompileError("--emit-tokens cannot be combined with checking, backend, module, or linking options");
        }
        std::vector<Token> tokens = lex_source(read_text_file(input));
        write_text_file(token_output, dump_tokens(tokens, input));
        std::cout << "wrote " << token_output << " (token dump)\n";
        return 0;
    }
    if (!syntax_output.empty()) {
        if (check_only || output_explicit || emit_llvm_only || !object_output.empty() ||
            !c_header_output.empty() || llvm_compiler_explicit || shared_library || test_mode ||
            !metadata_output.empty() || !metadata_check.empty() ||
            !module_cache_output.empty() || !module_cache_input.empty() ||
            !module_search_paths.empty() || !link_args.empty()) {
            throw CompileError("--emit-syntax cannot be combined with checking, backend, module, or linking options");
        }
        std::vector<Token> tokens = lex_source(read_text_file(input));
        Program syntax = parse_tokens(std::move(tokens), cfg_features, target.triple);
        write_text_file(syntax_output, dump_syntax(syntax, input));
        std::cout << "wrote " << syntax_output << " (syntax dump)\n";
        return 0;
    }
    if (!diagnostic_output.empty()) {
        if (check_only || output_explicit || emit_llvm_only || !object_output.empty() ||
            !c_header_output.empty() || llvm_compiler_explicit || shared_library || test_mode ||
            !metadata_output.empty() || !metadata_check.empty() ||
            !module_cache_output.empty() || !module_cache_input.empty() || !link_args.empty()) {
            throw CompileError("--emit-diagnostics cannot be combined with checking, backend, module-cache, or linking options");
        }
        std::string diagnostic_artifact = "diagnostic ok\n";
        try {
            ModuleLoadOptions diagnostic_load_options;
            diagnostic_load_options.module_search_paths = std::move(module_search_paths);
            diagnostic_load_options.cfg_features = cfg_features;
            diagnostic_load_options.target_triple = target.triple;
            diagnostic_load_options.implicit_std = implicit_std;
            ModuleLoadResult loaded = parse_file_with_module_metadata(input, std::move(diagnostic_load_options));
            Program program = std::move(loaded.program);
            SemaOptions sema_options;
            sema_options.require_main = true;
            sema_options.implicit_std = implicit_std;
            sema_options.cfg_features = cfg_features;
            sema_options.target_triple = target.triple;
            (void)check_program(program, std::move(sema_options));
        } catch (const CompileError& error) {
            diagnostic_artifact =
                dump_diagnostic_message("error", classify_diagnostic_code(error.what()), error.what(), input);
        }
        write_text_file(diagnostic_output, diagnostic_artifact);
        std::cout << "wrote " << diagnostic_output << " (diagnostic dump)\n";
        return 0;
    }

    std::size_t pass_summary_token_count = 0;
    if (!pass_summary_output.empty()) {
        pass_summary_token_count = lex_source(read_text_file(input)).size();
    }

    ModuleLoadOptions load_options;
    load_options.module_search_paths = std::move(module_search_paths);
    load_options.cfg_features = cfg_features;
    load_options.target_triple = target.triple;
    load_options.implicit_std = implicit_std;
    ModuleCache input_cache;
    if (!module_cache_input.empty()) {
        input_cache = read_module_cache_file(module_cache_input);
        require_matching_module_cache_inputs(input_cache, input, load_options, module_cache_input);
        load_options.input_cache = &input_cache;
    }
    ModuleLoadResult loaded = parse_file_with_module_metadata(input, std::move(load_options));
    if (!module_cache_input.empty()) {
        require_matching_module_metadata(input_cache.metadata, loaded.metadata, module_cache_input);
    }
    if (!metadata_check.empty()) {
        ModuleMetadata expected = read_module_metadata_file(metadata_check);
        require_matching_module_metadata(expected, loaded.metadata, metadata_check);
    }
    if (!metadata_output.empty()) {
        write_text_file(metadata_output, serialize_module_metadata(loaded.metadata));
        std::cout << "wrote " << metadata_output << " (module metadata)\n";
    }
    if (!source_map_output.empty()) {
        std::vector<SourceMapDumpFile> files;
        for (const auto& source : loaded.metadata.sources) {
            files.push_back(SourceMapDumpFile{
                source.module_name,
                source.path,
                read_text_file(source.path),
                source.is_root,
            });
        }
        write_text_file(source_map_output, dump_source_map(input, std::move(files)));
        std::cout << "wrote " << source_map_output << " (source map dump)\n";
        return 0;
    }
    if (!module_graph_output.empty()) {
        write_text_file(module_graph_output, dump_module_graph(loaded.metadata, input));
        std::cout << "wrote " << module_graph_output << " (module graph dump)\n";
        return 0;
    }
    Program program = std::move(loaded.program);
    if (!declaration_index_output.empty()) {
        write_text_file(declaration_index_output, dump_declaration_index(program, loaded.metadata, input));
        std::cout << "wrote " << declaration_index_output << " (declaration index dump)\n";
        return 0;
    }
    SemaOptions sema_options;
    sema_options.require_main = !shared_library && !test_mode && !check_only && object_output.empty();
    sema_options.test_mode = test_mode;
    sema_options.implicit_std = implicit_std;
    sema_options.cfg_features = cfg_features;
    sema_options.cached_ir_function_names =
        module_cache_ir_function_names(loaded.cached_ir_functions);
    sema_options.test_filters = std::move(test_filters);
    sema_options.target_triple = target.triple;
    IrProgram ir = check_program(program, std::move(sema_options));
    std::vector<IrFunction> cached_ir_functions =
        replay_module_cache_ir_functions(loaded.cached_ir_functions, program);
    for (auto& fn : cached_ir_functions) ir.functions.push_back(std::move(fn));
    for (const auto& warning : ir.warnings) {
        std::cerr << warning << "\n";
    }
    if (!typed_ir_output.empty()) {
        write_text_file(typed_ir_output, dump_ir_program(ir, input));
        std::cout << "wrote " << typed_ir_output << " (typed IR dump)\n";
        return 0;
    }
    if (!pass_summary_output.empty()) {
        write_text_file(pass_summary_output,
                        dump_compiler_pass_summary(input, pass_summary_token_count, program, loaded.metadata, ir));
        std::cout << "wrote " << pass_summary_output << " (compiler pass summary)\n";
        return 0;
    }
    if (!module_cache_output.empty()) {
        attach_module_cache_ir_summaries(loaded.cache, ir);
        write_text_file(module_cache_output, serialize_module_cache(loaded.cache));
        std::cout << "wrote " << module_cache_output << " (module cache)\n";
    }
    if (check_only) {
        return 0;
    }
    if (!c_header_output.empty()) {
        std::string header = emit_c_header(ir);
        write_text_file(c_header_output, header);
        std::cout << "wrote " << c_header_output << " (C header)\n";
    }
    const bool object_library_output = !object_output.empty();
    LlvmEmitOptions llvm_options;
    llvm_options.shared_library = shared_library || object_library_output;
    llvm_options.target_triple = target.triple;
    std::string llvm = emit_llvm_ir(ir, llvm_options);
    std::string llvm_path = llvm_output.empty() ? output + ".ll" : llvm_output;
    write_text_file(llvm_path, llvm);
    if (emit_llvm_only) {
        std::cout << "wrote " << llvm_path << " (" << llvm.size() << " bytes)\n";
        return 0;
    }

    std::string command = shell_quote(llvm_compiler) + " ";
    if (object_library_output) {
        command += "-c -fPIC ";
    } else if (shared_library) {
        command += "-shared -fPIC ";
    }
    if (!target_triple.empty()) command += shell_quote("--target=" + target.triple) + " ";
    command += shell_quote(llvm_path) + " -o " + shell_quote(object_output.empty() ? output : object_output);
    if (object_output.empty()) {
        if (target.unix) command += " -pthread";
        for (const auto& arg : link_args) command += " " + shell_quote(arg);
    }
    int status = std::system(command.c_str());
    if (llvm_output.empty()) {
        std::remove(llvm_path.c_str());
    }
    if (status != 0) throw CompileError("LLVM backend failed while producing output; install clang or pass --llvm-cc");
    if (!object_output.empty()) {
        std::cout << "wrote " << object_output << " (LLVM object)\n";
        return 0;
    }
    make_executable(output);
    std::cout << "wrote " << output << " (LLVM backend)\n";
    return 0;
}

} // namespace ari
