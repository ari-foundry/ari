#include "driver.hpp"

#include "common.hpp"
#include "c_header.hpp"
#include "llvm_codegen.hpp"
#include "module_cache.hpp"
#include "module_ir_replay.hpp"
#include "module_ir_summary.hpp"
#include "module_loader.hpp"
#include "module_metadata.hpp"
#include "parser.hpp"
#include "sema.hpp"
#include "target.hpp"
#include "toolchain.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
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

static std::string shell_quote(const std::string& text) {
    std::string quoted = "'";
    for (char c : text) {
        if (c == '\'') quoted += "'\\''";
        else quoted.push_back(c);
    }
    quoted += "'";
    return quoted;
}

static void usage() {
    std::cerr << "usage: ari <input.ari> [-o output] [--check] [--emit-llvm path]\n"
                 "           [--emit-obj path]\n"
                 "           [--module-path path] [-I path] [--llvm-cc compiler]\n"
                 "           [--target triple]\n"
                 "           [--emit-c-header path]\n"
                 "           [--emit-module-metadata path] [--check-module-metadata path]\n"
                 "           [--emit-module-cache path] [--use-module-cache path]\n"
                 "           [--no-implicit-std]\n"
                 "           [-L path] [-l name] [--link name] [--shared]\n"
                 "           [--test] [--cfg-feature name]\n";
}

int run(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 2;
    }

    std::string input;
    std::string output = "a.out";
    std::string llvm_output;
    std::string object_output;
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
    bool implicit_std = true;
    for (int i = 1; i < argc; ++i) {
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
    if (input.empty()) throw CompileError("missing input file");
    if (shared_library && test_mode) {
        throw CompileError("--test cannot be combined with --shared");
    }
    if (check_only && (output_explicit || emit_llvm_only || shared_library ||
                       !object_output.empty() ||
                       !c_header_output.empty() || llvm_compiler_explicit || !link_args.empty())) {
        throw CompileError("--check cannot be combined with backend output or linking options");
    }
    TargetInfo target = resolve_target_info(target_triple);
    if (!object_output.empty() && emit_llvm_only) {
        throw CompileError("--emit-obj cannot be combined with --emit-llvm");
    }
    if (!object_output.empty() && !link_args.empty()) {
        throw CompileError("--emit-obj cannot be combined with linker options");
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
    Program program = std::move(loaded.program);
    SemaOptions sema_options;
    sema_options.require_main = !shared_library && !test_mode && !check_only && object_output.empty();
    sema_options.test_mode = test_mode;
    sema_options.implicit_std = implicit_std;
    sema_options.cfg_features = cfg_features;
    sema_options.cached_ir_function_names =
        module_cache_ir_function_names(loaded.cached_ir_functions);
    sema_options.target_triple = target.triple;
    IrProgram ir = check_program(program, std::move(sema_options));
    std::vector<IrFunction> cached_ir_functions =
        replay_module_cache_ir_functions(loaded.cached_ir_functions, program);
    for (auto& fn : cached_ir_functions) ir.functions.push_back(std::move(fn));
    for (const auto& warning : ir.warnings) {
        std::cerr << warning << "\n";
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
