#include "ari_builtin.hpp"
#include "llvm_codegen.hpp"

#include "common.hpp"
#include "control_flow_semantics.hpp"
#include "enum_payload_layout.hpp"
#include "layout.hpp"
#include "slice_semantics.hpp"
#include "symbol_mangle.hpp"
#include "target.hpp"
#include "zone_runtime_layout.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

// Linux glibc and musl use this sysconf selector for online processors.
constexpr int kScNprocessorsOnln = 84;

static std::string quote_global(const std::string& name) {
    std::string out = "@\"";
    for (char c : name) {
        if (c == '\\' || c == '"') out.push_back('\\');
        out.push_back(c);
    }
    out += "\"";
    return out;
}

static std::string sanitize_local(const std::string& name) {
    std::string out;
    for (unsigned char c : name) {
        if (std::isalnum(c)) out.push_back(static_cast<char>(c));
        else out.push_back('_');
    }
    if (out.empty() || std::isdigit(static_cast<unsigned char>(out[0]))) out.insert(out.begin(), '_');
    return out;
}

static std::string basename(const std::string& name) {
    std::size_t split = name.rfind("::");
    if (split == std::string::npos) return name;
    return name.substr(split + 2);
}

static bool is_unsigned_integer_type(const IrType& type) {
    if (type.qualifier != TypeQualifier::Value) return false;
    return type.primitive == IrPrimitiveKind::U8 ||
           type.primitive == IrPrimitiveKind::U16 ||
           type.primitive == IrPrimitiveKind::U32 ||
           type.primitive == IrPrimitiveKind::U64;
}

static bool is_llvm_float_type(const IrType& type) {
    if (type.qualifier != TypeQualifier::Value) return false;
    return type.primitive == IrPrimitiveKind::F32 ||
           type.primitive == IrPrimitiveKind::F64 ||
           type.primitive == IrPrimitiveKind::F128;
}

static bool is_void_value_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Void;
}

static int float_bits(const IrType& type) {
    switch (type.primitive) {
        case IrPrimitiveKind::F32: return 32;
        case IrPrimitiveKind::F64: return 64;
        case IrPrimitiveKind::F128: return 128;
        default: return 0;
    }
}

static std::uint64_t double_bits(double value) {
    std::uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static std::string hex16(std::uint64_t value) {
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << value;
    return out.str();
}

static std::string fp128_literal_from_double(double value) {
    std::uint64_t bits = double_bits(value);
    std::uint64_t sign = bits >> 63;
    std::uint64_t exp = (bits >> 52) & 0x7ffULL;
    std::uint64_t frac = bits & 0x000fffffffffffffULL;

    if (exp == 0 && frac == 0) {
        std::uint64_t high = sign << 63;
        return "0xL" + hex16(0) + hex16(high);
    }

    int unbiased = 0;
    if (exp == 0) {
        int leading = 63;
        while (leading > 0 && ((frac >> leading) & 1ULL) == 0) --leading;
        unbiased = leading - 1074;
        frac = (frac << (52 - leading)) & 0x000fffffffffffffULL;
    } else {
        unbiased = static_cast<int>(exp) - 1023;
    }

    std::uint64_t exp128 = static_cast<std::uint64_t>(unbiased + 16383);
    std::uint64_t high = (sign << 63) | (exp128 << 48) | (frac >> 4);
    std::uint64_t low = (frac & 0xfULL) << 60;
    return "0xL" + hex16(low) + hex16(high);
}

static bool text_contains(const std::string& text, const std::string& needle) {
    return text.find(needle) != std::string::npos;
}

static std::string target_env_name(const TargetInfo& target) {
    if (target.macos) return "apple";
    if (text_contains(target.triple, "musl")) return "musl";
    if (text_contains(target.triple, "gnu")) return "gnu";
    if (text_contains(target.triple, "msvc")) return "msvc";
    if (text_contains(target.triple, "mingw")) return "mingw";
    return "unknown";
}

static int target_arch_tag(const TargetInfo& target) {
    if (target.arch == "x86_64") return 1;
    if (target.arch == "aarch64") return 2;
    if (target.arch == "riscv64") return 3;
    if (target.arch == "x86") return 4;
    if (target.arch == "arm") return 5;
    return 0;
}

static int target_os_tag(const TargetInfo& target) {
    if (target.linux) return 1;
    if (target.macos) return 2;
    if (target.windows) return 3;
    return 0;
}

static int target_env_tag(const TargetInfo& target) {
    const std::string env = target_env_name(target);
    if (env == "gnu") return 1;
    if (env == "musl") return 2;
    if (env == "msvc") return 3;
    if (env == "mingw") return 4;
    if (env == "apple") return 5;
    return 0;
}

static int target_object_format_tag(const TargetInfo& target) {
    if (target.linux) return 1;
    if (target.macos) return 2;
    if (target.windows) return 3;
    return 0;
}

static int target_debug_format_tag(const TargetInfo& target) {
    if (target.linux || target.macos) return 1;
    if (target.windows) return 2;
    return 0;
}

static int target_errno_abi_tag(const TargetInfo& target) {
    if (target.unix) return 1;
    if (target.windows) return 2;
    return 0;
}

static std::string target_os_name(const TargetInfo& target) {
    if (target.linux) return "linux";
    if (target.macos) return "macos";
    if (target.windows) return "windows";
    return "unknown";
}

static std::string float_literal(double value) {
    std::ostringstream out;
    out << std::scientific << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    return out.str();
}

static int integer_bits(const IrType& type) {
    switch (type.primitive) {
        case IrPrimitiveKind::Bool: return 1;
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::U8: return 8;
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::U16: return 16;
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::U32: return 32;
        case IrPrimitiveKind::I64:
        case IrPrimitiveKind::U64:
        case IrPrimitiveKind::Enum: return 64;
        default: return 0;
    }
}

static bool has_aggregate_enum_layout(const IrType& type) {
    return ari_has_aggregate_enum_layout(type);
}

class LlvmEmitter {
public:
    LlvmEmitter(const IrProgram& program, LlvmEmitOptions options)
        : program_(program), options_(options) {}

    std::string emit() {
        collect_symbols();
        emit_extern_decls();
        emit_runtime();
        emit_trait_object_vtables();
        std::vector<const IrFunction*> functions;
        functions.reserve(program_.functions.size());
        for (const auto& fn : program_.functions) functions.push_back(&fn);
        std::stable_sort(functions.begin(), functions.end(), [&](const IrFunction* lhs, const IrFunction* rhs) {
            const std::string& lhs_symbol = function_symbols_.at(lhs->name);
            const std::string& rhs_symbol = function_symbols_.at(rhs->name);
            if (lhs_symbol != rhs_symbol) return lhs_symbol < rhs_symbol;
            return lhs->name < rhs->name;
        });
        // Module cache replay can discover reachable helpers in a slightly
        // different order. Emit by final symbol so byte-for-byte cache checks
        // stay stable without changing semantic lowering.
        for (const IrFunction* fn : functions) emit_function(*fn);
        if (program_.require_main) emit_main_wrapper();

        std::ostringstream out;
        out << "; Ari LLVM IR backend\n";
        out << "target triple = \"" << resolve_target_info(options_.target_triple).triple << "\"\n\n";
        out << "@ari_argc = internal global i32 0\n";
        out << "@ari_argv = internal global ptr null\n\n";
        out << "@ari_context_cwd_buffer = internal global [4096 x i8] zeroinitializer, align 16\n";
        out << "@ari_context_executable_path_buffer = internal global [4096 x i8] zeroinitializer, align 16\n\n";
        out << "@ari_thread_id = internal thread_local global i64 0\n\n";
        out << "@ari_next_thread_id = internal global i64 1\n\n";
        out << "@ari_cwd_buffer = internal global [4096 x i8] zeroinitializer, align 16\n";
        out << "@ari_executable_path_buffer = internal global [4096 x i8] zeroinitializer, align 16\n\n";
        out << "@ari_realpath_buffer = internal global [4096 x i8] zeroinitializer, align 16\n\n";
        out << "@ari_readlink_buffer = internal global [4096 x i8] zeroinitializer, align 16\n\n";
        out << "@ari_line_buffer = internal global [4096 x i8] zeroinitializer, align 16\n\n";
        for (const auto& item : strings_) {
            out << item.name << " = private unnamed_addr constant [" << item.size << " x i8] c\"" << item.bytes << "\", align 1\n";
        }
        if (!strings_.empty()) out << "\n";
        out << trait_object_globals_.str();
        out << declarations_.str();
        out << runtime_.str();
        out << functions_.str();
        return out.str();
    }

private:
    struct StringGlobal {
        std::string name;
        std::string bytes;
        std::size_t size = 0;
    };

    struct Value {
        std::string type;
        std::string name;
        IrType ir_type;
    };

    struct LoopContext {
        std::string break_label;
        std::string plain_continue_label;
        std::string value_continue_label;
        std::string source_label;
        std::vector<std::string> update_names;
        bool is_loop = true;
        bool has_break = false;
        bool supports_break_value = false;
        std::string break_value_slot;
        IrType break_value_type;
    };

    static LoopContext make_loop_context(
        std::string break_label,
        std::string plain_continue_label,
        std::string value_continue_label,
        std::string source_label,
        std::vector<std::string> update_names = {},
        bool is_loop = true
    ) {
        LoopContext context;
        context.break_label = std::move(break_label);
        context.plain_continue_label = std::move(plain_continue_label);
        context.value_continue_label = std::move(value_continue_label);
        context.source_label = std::move(source_label);
        context.update_names = std::move(update_names);
        context.is_loop = is_loop;
        return context;
    }

    const IrProgram& program_;
    LlvmEmitOptions options_;
    std::ostringstream declarations_;
    std::ostringstream runtime_;
    std::ostringstream functions_;
    std::ostringstream trait_object_globals_;
    std::ostringstream* out_ = nullptr;
    std::vector<StringGlobal> strings_;
    std::map<std::string, std::string> function_symbols_;
    std::map<std::string, std::string> extern_symbols_;
    std::map<std::string, IrType> extern_results_;
    std::map<std::string, std::vector<IrParam>> extern_params_;
    std::map<std::string, IrType> function_results_;
    std::map<std::string, std::vector<IrParam>> function_params_;
    std::map<std::string, std::string> locals_;
    std::vector<LoopContext> loops_;
    int temp_counter_ = 0;
    int label_counter_ = 0;
    bool block_terminated_ = false;
    std::string current_label_;
    IrType current_return_;

    void line(const std::string& text = "") {
        *out_ << text << "\n";
    }

    std::string temp() {
        return "%t" + std::to_string(temp_counter_++);
    }

    std::string label(const std::string& prefix) {
        return prefix + "." + std::to_string(label_counter_++);
    }

    void emit_label(const std::string& name) {
        *out_ << name << ":\n";
        current_label_ = name;
        block_terminated_ = false;
    }

    std::string llvm_aggregate_type(const IrType& type) const {
        const std::vector<IrType>& fields = ari_aggregate_field_types(type);
        if (fields.empty()) return "{}";
        std::string text = "{ ";
        for (std::size_t i = 0; i < fields.size(); ++i) {
            if (i > 0) text += ", ";
            text += llvm_type(fields[i]);
        }
        text += " }";
        return text;
    }

    std::string llvm_type(const IrType& type) const {
        if (type.primitive == IrPrimitiveKind::TraitObject) {
            if (type.qualifier == TypeQualifier::Own) return "{ ptr, ptr, ptr }";
            return "{ ptr, ptr }";
        }
        if (type.qualifier == TypeQualifier::Ref ||
            type.qualifier == TypeQualifier::MutRef ||
            type.qualifier == TypeQualifier::Ptr) {
            return "ptr";
        }

        switch (type.primitive) {
            case IrPrimitiveKind::Void: return "void";
            case IrPrimitiveKind::Bool: return "i1";
            case IrPrimitiveKind::I8:
            case IrPrimitiveKind::U8: return "i8";
            case IrPrimitiveKind::I16:
            case IrPrimitiveKind::U16: return "i16";
            case IrPrimitiveKind::I32:
            case IrPrimitiveKind::U32: return "i32";
            case IrPrimitiveKind::I64:
            case IrPrimitiveKind::U64: return "i64";
            case IrPrimitiveKind::Enum:
                if (!has_aggregate_enum_layout(type)) return "i64";
                return llvm_aggregate_type(type);
            case IrPrimitiveKind::F32: return "float";
            case IrPrimitiveKind::F64: return "double";
            case IrPrimitiveKind::F128: return "fp128";
            case IrPrimitiveKind::String: return "ptr";
            case IrPrimitiveKind::Function: return "ptr";
            case IrPrimitiveKind::Zone: return "ptr";
            case IrPrimitiveKind::TraitObject:
                if (type.qualifier == TypeQualifier::Own) return "{ ptr, ptr, ptr }";
                return "{ ptr, ptr }";
            case IrPrimitiveKind::Struct:
            case IrPrimitiveKind::Tuple:
                return llvm_aggregate_type(type);
            case IrPrimitiveKind::Array:
                if (type.args.size() != 1) {
                    throw CompileError(where(type.loc) + ": array type is missing an element type");
                }
                return "[" + std::to_string(type.array_size) + " x " + llvm_type(type.args[0]) + "]";
            case IrPrimitiveKind::Vector:
                if (type.args.size() != 1) {
                    throw CompileError(where(type.loc) + ": LLVM backend cannot lower unsized Vec storage yet");
                }
                return "{ i64, [" + std::to_string(type.array_size) + " x " + llvm_type(type.args[0]) + "] }";
            default:
                throw CompileError(where(type.loc) + ": LLVM backend cannot lower type " + type_name(type));
        }
    }

    std::string default_value(const IrType& type) const {
        if (is_void_value_type(type)) return "";
        if (type.primitive == IrPrimitiveKind::Tuple ||
            type.primitive == IrPrimitiveKind::Array ||
            type.primitive == IrPrimitiveKind::Struct ||
            type.primitive == IrPrimitiveKind::TraitObject ||
            has_aggregate_enum_layout(type) ||
            type.primitive == IrPrimitiveKind::Vector) return "zeroinitializer";
        if (llvm_type(type) == "ptr") return "null";
        if (type.primitive == IrPrimitiveKind::F128) return fp128_literal_from_double(0.0);
        if (type.primitive == IrPrimitiveKind::F32 ||
            type.primitive == IrPrimitiveKind::F64) return "0.0";
        return "0";
    }

    std::string llvm_string_bytes(const std::string& text) const {
        static const char* hex = "0123456789ABCDEF";
        std::string out;
        for (unsigned char c : text) {
            out.push_back('\\');
            out.push_back(hex[(c >> 4) & 0xf]);
            out.push_back(hex[c & 0xf]);
        }
        out += "\\00";
        return out;
    }

    std::string string_ptr(const std::string& text) {
        StringGlobal global;
        global.name = "@.ari.str." + std::to_string(strings_.size());
        global.bytes = llvm_string_bytes(text);
        global.size = text.size() + 1;
        strings_.push_back(global);
        return "getelementptr inbounds ([" + std::to_string(global.size) + " x i8], ptr " +
               global.name + ", i64 0, i64 0)";
    }

    static bool has_runtime_c_declaration(const std::string& symbol) {
        return symbol == "printf" ||
               symbol == "putchar" ||
               symbol == "getchar" ||
               symbol == "fgets" ||
               symbol == "getenv" ||
               symbol == "setenv" ||
               symbol == "unsetenv" ||
               symbol == "getcwd" ||
               symbol == "chdir" ||
               symbol == "readlink" ||
               symbol == "getpid" ||
               symbol == "getuid" ||
               symbol == "getgid" ||
               symbol == "getpagesize" ||
               symbol == "fork" ||
               symbol == "waitpid" ||
               symbol == "pthread_create" ||
               symbol == "pthread_join" ||
               symbol == "sched_yield" ||
               symbol == "sysconf" ||
               symbol == "clock_gettime" ||
               symbol == "nanosleep" ||
               symbol == "getrandom" ||
               symbol == "access" ||
               symbol == "unlink" ||
               symbol == "rename" ||
               symbol == "link" ||
               symbol == "symlink" ||
               symbol == "mkdir" ||
               symbol == "rmdir" ||
               symbol == "chmod" ||
               symbol == "opendir" ||
               symbol == "readdir" ||
               symbol == "closedir" ||
               symbol == "open" ||
               symbol == "read" ||
               symbol == "write" ||
               symbol == "close" ||
               symbol == "lseek" ||
               symbol == "dup" ||
               symbol == "fcntl" ||
               symbol == "pipe" ||
               symbol == "malloc" ||
               symbol == "free" ||
               symbol == "exit" ||
               symbol == "abort";
    }

    static IrType builtin_result_type(const std::string& symbol, SourceLocation loc) {
        if (symbol == "ari_builtin_zone_create") {
            return IrType{TypeQualifier::Own, IrPrimitiveKind::Zone, "Zone", {}, {}, {}, {}, loc};
        }
        if (symbol == "ari_builtin_zone_alloc" || symbol == "ari_builtin_string_alloc_buffer") {
            return IrType{TypeQualifier::Ptr, IrPrimitiveKind::U8, "u8", {}, {}, {}, {}, loc};
        }
        if (symbol == "ari_builtin_zone_allocation_zone") {
            return IrType{TypeQualifier::Ptr, IrPrimitiveKind::Void, "void", {}, {}, {}, {}, loc};
        }
        if (symbol == "ari_builtin_random_entropy") {
            return IrType{TypeQualifier::Value, IrPrimitiveKind::U64, "u64", {}, {}, {}, {}, loc};
        }
        if (symbol == "ari_builtin_context_arg" ||
            symbol == "ari_builtin_context_cwd" ||
            symbol == "ari_builtin_context_executable_path" ||
            symbol == "ari_builtin_target_triple" ||
            symbol == "ari_builtin_target_arch_name" ||
            symbol == "ari_builtin_target_os_name" ||
            symbol == "ari_builtin_target_env_name" ||
            symbol == "ari_builtin_env_get" ||
            symbol == "ari_builtin_env_current_dir" ||
            symbol == "ari_builtin_env_executable_path" ||
            symbol == "ari_builtin_fs_canonicalize" ||
            symbol == "ari_builtin_fs_read_link" ||
            symbol == "ari_builtin_fs_read_dir_next" ||
            symbol == "ari_builtin_read_line") {
            return IrType{TypeQualifier::Value, IrPrimitiveKind::String, "string", {}, {}, {}, {}, loc};
        }
        if (symbol == "ari_builtin_env_has" ||
            symbol == "ari_builtin_env_set" ||
            symbol == "ari_builtin_env_remove" ||
            symbol == "ari_builtin_env_set_current_dir" ||
            symbol == "ari_builtin_fs_exists" ||
            symbol == "ari_builtin_fs_can_read" ||
            symbol == "ari_builtin_fs_can_write" ||
            symbol == "ari_builtin_fs_can_execute" ||
            symbol == "ari_builtin_fs_remove" ||
            symbol == "ari_builtin_fs_rename" ||
            symbol == "ari_builtin_fs_create_dir" ||
            symbol == "ari_builtin_fs_create_dir_all" ||
            symbol == "ari_builtin_fs_remove_dir" ||
            symbol == "ari_builtin_fs_set_mode" ||
            symbol == "ari_builtin_fs_close_dir" ||
            symbol == "ari_builtin_fs_close" ||
            symbol == "ari_builtin_fs_write_byte" ||
            symbol == "ari_builtin_fs_seek" ||
            symbol == "ari_builtin_os_close" ||
            symbol == "ari_builtin_os_set_close_on_exec" ||
            symbol == "ari_builtin_os_set_nonblocking" ||
            symbol == "ari_builtin_os_write_byte" ||
            symbol == "ari_builtin_sync_atomic_i64_compare_exchange") {
            return IrType{TypeQualifier::Value, IrPrimitiveKind::Bool, "bool", {}, {}, {}, {}, loc};
        }
        if (symbol == "ari_builtin_panic" ||
            symbol == "ari_builtin_process_exit" ||
            symbol == "ari_builtin_process_abort" ||
            symbol == "ari_builtin_thread_yield" ||
            symbol == "ari_builtin_mem_copy_bytes" ||
            symbol == "ari_builtin_mem_move_bytes" ||
            symbol == "ari_builtin_mem_set_bytes" ||
            symbol == "ari_builtin_sync_atomic_i64_store" ||
            symbol == "ari_builtin_time_sleep_nanos" ||
            symbol == "ari_builtin_random_fill" ||
            symbol == "ari_builtin_zone_reset" ||
            symbol == "ari_builtin_zone_destroy") {
            return IrType{TypeQualifier::Value, IrPrimitiveKind::Void, "void", {}, {}, {}, {}, loc};
        }
        return IrType{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, loc};
    }

    void collect_symbols() {
        for (const auto& fn : program_.extern_functions) {
            extern_symbols_[fn.name] = fn.link_name.empty() ? basename(fn.name) : fn.link_name;
            extern_results_[fn.name] = fn.return_type;
            extern_params_[fn.name] = fn.params;
        }
        for (const auto& fn : program_.functions) {
            function_symbols_[fn.name] = fn.link_name.empty() ? mangle_function_name(fn.name) : fn.link_name;
            function_results_[fn.name] = fn.return_type;
            function_params_[fn.name] = fn.params;
        }
    }

    void emit_extern_decls() {
        declarations_ << "declare i32 @printf(ptr, ...)\n";
        declarations_ << "declare i32 @putchar(i32)\n";
        declarations_ << "declare i32 @getchar()\n";
        declarations_ << "declare ptr @fgets(ptr, i32, ptr)\n";
        declarations_ << "declare ptr @getenv(ptr)\n";
        declarations_ << "declare i32 @setenv(ptr, ptr, i32)\n";
        declarations_ << "declare i32 @unsetenv(ptr)\n";
        declarations_ << "declare ptr @getcwd(ptr, i64)\n";
        declarations_ << "declare i32 @chdir(ptr)\n";
        declarations_ << "declare i64 @readlink(ptr, ptr, i64)\n";
        declarations_ << "declare ptr @realpath(ptr, ptr)\n";
        declarations_ << "declare i32 @getpid()\n";
        declarations_ << "declare i32 @getuid()\n";
        declarations_ << "declare i32 @getgid()\n";
        declarations_ << "declare i32 @getpagesize()\n";
        declarations_ << "declare i32 @fork()\n";
        declarations_ << "declare i32 @waitpid(i32, ptr, i32)\n";
        declarations_ << "declare i32 @pthread_create(ptr, ptr, ptr, ptr)\n";
        declarations_ << "declare i32 @pthread_join(i64, ptr)\n";
        declarations_ << "declare i32 @sched_yield()\n";
        declarations_ << "declare i64 @sysconf(i32)\n";
        declarations_ << "declare i32 @clock_gettime(i32, ptr)\n";
        declarations_ << "declare i32 @nanosleep(ptr, ptr)\n";
        declarations_ << "declare i64 @getrandom(ptr, i64, i32)\n";
        declarations_ << "declare i32 @access(ptr, i32)\n";
        declarations_ << "declare i32 @stat(ptr, ptr)\n";
        declarations_ << "declare i32 @unlink(ptr)\n";
        declarations_ << "declare i32 @rename(ptr, ptr)\n";
        declarations_ << "declare i32 @link(ptr, ptr)\n";
        declarations_ << "declare i32 @symlink(ptr, ptr)\n";
        declarations_ << "declare i32 @mkdir(ptr, i32)\n";
        declarations_ << "declare i32 @rmdir(ptr)\n";
        declarations_ << "declare i32 @chmod(ptr, i32)\n";
        declarations_ << "declare ptr @opendir(ptr)\n";
        declarations_ << "declare ptr @readdir(ptr)\n";
        declarations_ << "declare i32 @closedir(ptr)\n";
        declarations_ << "declare i32 @open(ptr, i32, i32)\n";
        declarations_ << "declare i64 @read(i32, ptr, i64)\n";
        declarations_ << "declare i64 @write(i32, ptr, i64)\n";
        declarations_ << "declare i32 @close(i32)\n";
        declarations_ << "declare i64 @lseek(i32, i64, i32)\n";
        declarations_ << "declare i32 @dup(i32)\n";
        declarations_ << "declare i32 @fcntl(i32, i32, ...)\n";
        declarations_ << "declare i32 @pipe(ptr)\n";
        declarations_ << "declare void @llvm.memcpy.p0.p0.i64(ptr, ptr, i64, i1)\n";
        declarations_ << "declare void @llvm.memmove.p0.p0.i64(ptr, ptr, i64, i1)\n";
        declarations_ << "declare void @llvm.memset.p0.i64(ptr, i8, i64, i1)\n";
        declarations_ << "declare ptr @malloc(i64)\n";
        declarations_ << "declare void @free(ptr)\n";
        declarations_ << "declare void @exit(i32)\n";
        declarations_ << "declare void @abort()\n";
        declarations_ << "@stdin = external global ptr\n";
        for (const auto& fn : program_.extern_functions) {
            const std::string& symbol = extern_symbols_.at(fn.name);
            if (fn.abi == IrExternAbi::AriBuiltin) continue;
            if (has_runtime_c_declaration(symbol)) continue;
            declarations_ << "declare " << llvm_type(fn.return_type) << " "
                          << quote_global(symbol) << "(";
            for (std::size_t i = 0; i < fn.params.size(); ++i) {
                if (i > 0) declarations_ << ", ";
                declarations_ << llvm_type(fn.params[i].type);
            }
            if (fn.is_variadic) {
                if (!fn.params.empty()) declarations_ << ", ";
                declarations_ << "...";
            }
            declarations_ << ")\n";
        }
        declarations_ << "\n";
    }

    void emit_runtime() {
        out_ = &runtime_;
        const std::string runtime_visibility = options_.shared_library ? "hidden " : "";
        std::string fmt_i64 = string_ptr("%lld");
        std::string fmt_u64 = string_ptr("%llu");
        std::string fmt_text = string_ptr("%s");
        std::string text_true = string_ptr("true");
        std::string text_false = string_ptr("false");
        std::string empty = string_ptr("");
        std::string fs_mode_read = string_ptr("r");
        std::string fs_mode_write = string_ptr("w");
        std::string fs_mode_append = string_ptr("a");
        std::string proc_self_exe = string_ptr("/proc/self/exe");
        std::string dev_urandom = string_ptr("/dev/urandom");
        TargetInfo target = resolve_target_info(options_.target_triple);
        std::string target_triple = string_ptr(target.triple);
        std::string target_arch = string_ptr(target.arch);
        std::string target_os = string_ptr(target_os_name(target));
        std::string target_env = string_ptr(target_env_name(target));
        std::string line_buffer = "getelementptr inbounds ([4096 x i8], ptr @ari_line_buffer, i64 0, i64 0)";
        std::string cwd_buffer = "getelementptr inbounds ([4096 x i8], ptr @ari_cwd_buffer, i64 0, i64 0)";
        std::string realpath_buffer =
            "getelementptr inbounds ([4096 x i8], ptr @ari_realpath_buffer, i64 0, i64 0)";
        std::string readlink_buffer =
            "getelementptr inbounds ([4096 x i8], ptr @ari_readlink_buffer, i64 0, i64 0)";
        std::string context_cwd_buffer =
            "getelementptr inbounds ([4096 x i8], ptr @ari_context_cwd_buffer, i64 0, i64 0)";
        std::string executable_path_buffer =
            "getelementptr inbounds ([4096 x i8], ptr @ari_executable_path_buffer, i64 0, i64 0)";
        std::string context_executable_path_buffer =
            "getelementptr inbounds ([4096 x i8], ptr @ari_context_executable_path_buffer, i64 0, i64 0)";
        const std::string zone_struct_bytes = std::to_string(kZoneRuntimeZoneStructBytes);
        const std::string zone_max_capacity = std::to_string(kZoneRuntimeMaxCreateCapacity);
        const std::string zone_arena_scale = std::to_string(kZoneRuntimeArenaReserveScale);
        const std::string zone_arena_slack = std::to_string(kZoneRuntimeArenaReserveSlack);
        const std::string zone_min_payload_align = std::to_string(kZoneRuntimeMinimumPayloadAlign);
        const std::string zone_header_bytes = std::to_string(kZoneAllocationHeaderBytes);
        const std::string zone_header_zone_offset = std::to_string(kZoneAllocationHeaderZoneOffset);

        line("define " + runtime_visibility + "void @ari_context_init(i32 %argc, ptr %argv) {");
        line("entry:");
        line("  store i32 %argc, ptr @ari_argc");
        line("  store ptr %argv, ptr @ari_argv");
        line("  store i64 0, ptr @ari_thread_id");
        line("  %context.cwd = call ptr @getcwd(ptr " + context_cwd_buffer + ", i64 4096)");
        line("  %context.cwd.missing = icmp eq ptr %context.cwd, null");
        line("  br i1 %context.cwd.missing, label %cwd.empty, label %cwd.done");
        line("cwd.empty:");
        line("  store i8 0, ptr " + context_cwd_buffer);
        line("  br label %cwd.done");
        line("cwd.done:");
        line("  %context.exe.len = call i64 @readlink(ptr " + proc_self_exe + ", ptr " + context_executable_path_buffer + ", i64 4095)");
        line("  %context.exe.negative = icmp slt i64 %context.exe.len, 0");
        line("  %context.exe.too_long = icmp sge i64 %context.exe.len, 4095");
        line("  %context.exe.bad = or i1 %context.exe.negative, %context.exe.too_long");
        line("  br i1 %context.exe.bad, label %exe.empty, label %exe.found");
        line("exe.found:");
        line("  %context.exe.end = getelementptr inbounds i8, ptr " + context_executable_path_buffer + ", i64 %context.exe.len");
        line("  store i8 0, ptr %context.exe.end");
        line("  br label %context.done");
        line("exe.empty:");
        line("  store i8 0, ptr " + context_executable_path_buffer);
        line("  br label %context.done");
        line("context.done:");
        line("  ret void");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_context_shutdown() {");
        line("entry:");
        line("  ret void");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_context_argc() {");
        line("entry:");
        line("  %argc = load i32, ptr @ari_argc");
        line("  %wide = sext i32 %argc to i64");
        line("  ret i64 %wide");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_context_thread_id() {");
        line("entry:");
        line("  %id = load i64, ptr @ari_thread_id");
        line("  ret i64 %id");
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_context_arg(i64 %index) {");
        line("entry:");
        line("  %argc32 = load i32, ptr @ari_argc");
        line("  %argc = sext i32 %argc32 to i64");
        line("  %low = icmp slt i64 %index, 0");
        line("  %high = icmp sge i64 %index, %argc");
        line("  %bad = or i1 %low, %high");
        line("  br i1 %bad, label %empty, label %load");
        line("load:");
        line("  %argv = load ptr, ptr @ari_argv");
        line("  %slot = getelementptr inbounds ptr, ptr %argv, i64 %index");
        line("  %arg = load ptr, ptr %slot");
        line("  ret ptr %arg");
        line("empty:");
        line("  ret ptr " + empty);
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_context_cwd() {");
        line("entry:");
        line("  ret ptr " + context_cwd_buffer);
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_context_executable_path() {");
        line("entry:");
        line("  ret ptr " + context_executable_path_buffer);
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_target_triple() {");
        line("entry:");
        line("  ret ptr " + target_triple);
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_target_arch() {");
        line("entry:");
        line("  ret i64 " + std::to_string(target_arch_tag(target)));
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_target_arch_name() {");
        line("entry:");
        line("  ret ptr " + target_arch);
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_target_os() {");
        line("entry:");
        line("  ret i64 " + std::to_string(target_os_tag(target)));
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_target_os_name() {");
        line("entry:");
        line("  ret ptr " + target_os);
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_target_env() {");
        line("entry:");
        line("  ret i64 " + std::to_string(target_env_tag(target)));
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_target_env_name() {");
        line("entry:");
        line("  ret ptr " + target_env);
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_target_object_format() {");
        line("entry:");
        line("  ret i64 " + std::to_string(target_object_format_tag(target)));
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_target_debug_format() {");
        line("entry:");
        line("  ret i64 " + std::to_string(target_debug_format_tag(target)));
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_target_errno_abi() {");
        line("entry:");
        line("  ret i64 " + std::to_string(target_errno_abi_tag(target)));
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_target_pointer_bits() {");
        line("entry:");
        line("  ret i64 " + std::to_string(target.pointer_bits));
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_target_long_bits() {");
        line("entry:");
        line("  ret i64 " + std::to_string(target.long_bits));
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_env_get(ptr %name) {");
        line("entry:");
        line("  %value = call ptr @getenv(ptr %name)");
        line("  %missing = icmp eq ptr %value, null");
        line("  br i1 %missing, label %empty, label %found");
        line("found:");
        line("  ret ptr %value");
        line("empty:");
        line("  ret ptr " + empty);
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_env_has(ptr %name) {");
        line("entry:");
        line("  %value = call ptr @getenv(ptr %name)");
        line("  %missing = icmp eq ptr %value, null");
        line("  %present = xor i1 %missing, true");
        line("  ret i1 %present");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_env_set(ptr %name, ptr %value) {");
        line("entry:");
        line("  %code = call i32 @setenv(ptr %name, ptr %value, i32 1)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_env_remove(ptr %name) {");
        line("entry:");
        line("  %code = call i32 @unsetenv(ptr %name)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_env_current_dir() {");
        line("entry:");
        line("  %path = call ptr @getcwd(ptr " + cwd_buffer + ", i64 4096)");
        line("  %missing = icmp eq ptr %path, null");
        line("  br i1 %missing, label %empty, label %found");
        line("found:");
        line("  ret ptr " + cwd_buffer);
        line("empty:");
        line("  ret ptr " + empty);
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_env_set_current_dir(ptr %path) {");
        line("entry:");
        line("  %code = call i32 @chdir(ptr %path)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_env_executable_path() {");
        line("entry:");
        line("  %len = call i64 @readlink(ptr " + proc_self_exe + ", ptr " + executable_path_buffer + ", i64 4095)");
        line("  %negative = icmp slt i64 %len, 0");
        line("  %too_long = icmp sge i64 %len, 4095");
        line("  %bad = or i1 %negative, %too_long");
        line("  br i1 %bad, label %empty, label %found");
        line("found:");
        line("  %end = getelementptr inbounds i8, ptr " + executable_path_buffer + ", i64 %len");
        line("  store i8 0, ptr %end");
        line("  ret ptr " + executable_path_buffer);
        line("empty:");
        line("  store i8 0, ptr " + executable_path_buffer);
        line("  ret ptr " + empty);
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_exists(ptr %path) {");
        line("entry:");
        line("  %code = call i32 @access(ptr %path, i32 0)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        // POSIX access constants: R_OK=4, W_OK=2, X_OK=1.
        line("define " + runtime_visibility + "i1 @ari_builtin_fs_can_read(ptr %path) {");
        line("entry:");
        line("  %code = call i32 @access(ptr %path, i32 4)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_can_write(ptr %path) {");
        line("entry:");
        line("  %code = call i32 @access(ptr %path, i32 2)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_can_execute(ptr %path) {");
        line("entry:");
        line("  %code = call i32 @access(ptr %path, i32 1)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_fs_metadata_size(ptr %path) {");
        line("entry:");
        line("  %stat.storage = alloca [144 x i8], align 8");
        line("  %stat.ptr = getelementptr inbounds [144 x i8], ptr %stat.storage, i64 0, i64 0");
        line("  %code = call i32 @stat(ptr %path, ptr %stat.ptr)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  br i1 %ok, label %load, label %fail");
        line("load:");
        // Linux/glibc x86_64 stat layout: st_size is an i64 at byte offset 48.
        line("  %size.ptr = getelementptr inbounds i8, ptr %stat.ptr, i64 48");
        line("  %size = load i64, ptr %size.ptr, align 8");
        line("  ret i64 %size");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_fs_metadata_kind(ptr %path) {");
        line("entry:");
        line("  %stat.storage = alloca [144 x i8], align 8");
        line("  %stat.ptr = getelementptr inbounds [144 x i8], ptr %stat.storage, i64 0, i64 0");
        line("  %code = call i32 @stat(ptr %path, ptr %stat.ptr)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  br i1 %ok, label %load, label %fail");
        line("load:");
        // Linux/glibc x86_64 stat layout: st_mode is an i32 at byte offset 24.
        line("  %mode.ptr = getelementptr inbounds i8, ptr %stat.ptr, i64 24");
        line("  %mode = load i32, ptr %mode.ptr, align 4");
        line("  %masked = and i32 %mode, 61440");
        line("  %is.regular = icmp eq i32 %masked, 32768");
        line("  %is.dir = icmp eq i32 %masked, 16384");
        line("  %is.symlink = icmp eq i32 %masked, 40960");
        line("  %kind.dir = select i1 %is.dir, i64 2, i64 0");
        line("  %kind.link = select i1 %is.symlink, i64 3, i64 %kind.dir");
        line("  %kind = select i1 %is.regular, i64 1, i64 %kind.link");
        line("  ret i64 %kind");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_fs_metadata_mode(ptr %path) {");
        line("entry:");
        line("  %stat.storage = alloca [144 x i8], align 8");
        line("  %stat.ptr = getelementptr inbounds [144 x i8], ptr %stat.storage, i64 0, i64 0");
        line("  %code = call i32 @stat(ptr %path, ptr %stat.ptr)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  br i1 %ok, label %load, label %fail");
        line("load:");
        // Linux/glibc x86_64 stat layout: st_mode is an i32 at byte offset 24.
        line("  %mode.ptr = getelementptr inbounds i8, ptr %stat.ptr, i64 24");
        line("  %mode32 = load i32, ptr %mode.ptr, align 4");
        line("  %mode = zext i32 %mode32 to i64");
        line("  ret i64 %mode");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define private i1 @ari_runtime_fs_ensure_dir_component(ptr %path) {");
        line("entry:");
        line("  %kind = call i64 @ari_builtin_fs_metadata_kind(ptr %path)");
        line("  %is.dir = icmp eq i64 %kind, 2");
        line("  br i1 %is.dir, label %success, label %not.dir");
        line("not.dir:");
        line("  %exists = icmp sge i64 %kind, 0");
        line("  br i1 %exists, label %fail, label %mkdir");
        line("mkdir:");
        // 0755 in decimal; the process umask still applies on POSIX hosts.
        line("  %code = call i32 @mkdir(ptr %path, i32 493)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  br i1 %ok, label %success, label %retry");
        line("retry:");
        line("  %retry.kind = call i64 @ari_builtin_fs_metadata_kind(ptr %path)");
        line("  %retry.dir = icmp eq i64 %retry.kind, 2");
        line("  br i1 %retry.dir, label %success, label %fail");
        line("success:");
        line("  ret i1 true");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_fs_canonicalize(ptr %path) {");
        line("entry:");
        line("  %resolved = call ptr @realpath(ptr %path, ptr " + realpath_buffer + ")");
        line("  %missing = icmp eq ptr %resolved, null");
        line("  br i1 %missing, label %empty, label %found");
        line("found:");
        line("  ret ptr " + realpath_buffer);
        line("empty:");
        line("  ret ptr " + empty);
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_fs_read_link(ptr %path) {");
        line("entry:");
        line("  %len = call i64 @readlink(ptr %path, ptr " + readlink_buffer + ", i64 4095)");
        line("  %negative = icmp slt i64 %len, 0");
        line("  %too_long = icmp sge i64 %len, 4095");
        line("  %bad = or i1 %negative, %too_long");
        line("  br i1 %bad, label %empty, label %found");
        line("found:");
        line("  %end = getelementptr inbounds i8, ptr " + readlink_buffer + ", i64 %len");
        line("  store i8 0, ptr %end");
        line("  ret ptr " + readlink_buffer);
        line("empty:");
        line("  store i8 0, ptr " + readlink_buffer);
        line("  ret ptr " + empty);
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_remove(ptr %path) {");
        line("entry:");
        line("  %code = call i32 @unlink(ptr %path)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_rename(ptr %source, ptr %target) {");
        line("entry:");
        line("  %code = call i32 @rename(ptr %source, ptr %target)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_hard_link(ptr %existing, ptr %link_path) {");
        line("entry:");
        line("  %code = call i32 @link(ptr %existing, ptr %link_path)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_symbolic_link(ptr %target, ptr %link_path) {");
        line("entry:");
        line("  %code = call i32 @symlink(ptr %target, ptr %link_path)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_create_dir(ptr %path) {");
        line("entry:");
        // 0755 in decimal; the process umask still applies on POSIX hosts.
        line("  %code = call i32 @mkdir(ptr %path, i32 493)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_create_dir_all(ptr %path) {");
        line("entry:");
        line("  %first = load i8, ptr %path");
        line("  %empty.path = icmp eq i8 %first, 0");
        line("  br i1 %empty.path, label %fail, label %existing.check");
        line("existing.check:");
        line("  %existing.kind = call i64 @ari_builtin_fs_metadata_kind(ptr %path)");
        line("  %existing.dir = icmp eq i64 %existing.kind, 2");
        line("  br i1 %existing.dir, label %success, label %existing.not.dir");
        line("existing.not.dir:");
        line("  %existing.any = icmp sge i64 %existing.kind, 0");
        line("  br i1 %existing.any, label %fail, label %copy.init");
        line("copy.init:");
        line("  %buffer.storage = alloca [4096 x i8], align 1");
        line("  %buffer = getelementptr inbounds [4096 x i8], ptr %buffer.storage, i64 0, i64 0");
        line("  br label %copy.loop");
        line("copy.loop:");
        line("  %copy.i = phi i64 [0, %copy.init], [%copy.next, %copy.more]");
        line("  %copy.too.long = icmp sge i64 %copy.i, 4095");
        line("  br i1 %copy.too.long, label %fail, label %copy.byte");
        line("copy.byte:");
        line("  %source.ptr = getelementptr inbounds i8, ptr %path, i64 %copy.i");
        line("  %byte = load i8, ptr %source.ptr");
        line("  %dest.ptr = getelementptr inbounds i8, ptr %buffer, i64 %copy.i");
        line("  store i8 %byte, ptr %dest.ptr");
        line("  %copy.done = icmp eq i8 %byte, 0");
        line("  br i1 %copy.done, label %scan.init, label %copy.more");
        line("copy.more:");
        line("  %copy.next = add i64 %copy.i, 1");
        line("  br label %copy.loop");
        line("scan.init:");
        line("  br label %scan.loop");
        line("scan.loop:");
        line("  %scan.i = phi i64 [0, %scan.init], [%scan.next, %scan.next.block]");
        line("  %scan.ptr = getelementptr inbounds i8, ptr %buffer, i64 %scan.i");
        line("  %scan.byte = load i8, ptr %scan.ptr");
        line("  %scan.end = icmp eq i8 %scan.byte, 0");
        line("  br i1 %scan.end, label %final, label %scan.check");
        line("scan.check:");
        line("  %is.slash = icmp eq i8 %scan.byte, 47");
        line("  %after.first = icmp sgt i64 %scan.i, 0");
        line("  %should.ensure = and i1 %is.slash, %after.first");
        line("  br i1 %should.ensure, label %prefix.ensure, label %scan.next.block");
        line("prefix.ensure:");
        line("  store i8 0, ptr %scan.ptr");
        line("  %prefix.ok = call i1 @ari_runtime_fs_ensure_dir_component(ptr %buffer)");
        line("  store i8 47, ptr %scan.ptr");
        line("  br i1 %prefix.ok, label %scan.next.block, label %fail");
        line("scan.next.block:");
        line("  %scan.next = add i64 %scan.i, 1");
        line("  br label %scan.loop");
        line("final:");
        line("  %final.ok = call i1 @ari_runtime_fs_ensure_dir_component(ptr %buffer)");
        line("  br i1 %final.ok, label %success, label %fail");
        line("success:");
        line("  ret i1 true");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_remove_dir(ptr %path) {");
        line("entry:");
        line("  %code = call i32 @rmdir(ptr %path)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("}");
        line();

        line("define " + runtime_visibility + "{ ptr } @ari_builtin_fs_open_dir(ptr %path) {");
        line("entry:");
        line("  %handle = call ptr @opendir(ptr %path)");
        line("  %dir = insertvalue { ptr } undef, ptr %handle, 0");
        line("  ret { ptr } %dir");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_close_dir({ ptr } %dir) {");
        line("entry:");
        line("  %handle = extractvalue { ptr } %dir, 0");
        line("  %invalid = icmp eq ptr %handle, null");
        line("  br i1 %invalid, label %fail, label %do_close");
        line("do_close:");
        line("  %code = call i32 @closedir(ptr %handle)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_fs_read_dir_next({ ptr } %dir) {");
        line("entry:");
        line("  %handle = extractvalue { ptr } %dir, 0");
        line("  %invalid = icmp eq ptr %handle, null");
        line("  br i1 %invalid, label %empty, label %next");
        line("next:");
        line("  %entry.ptr = call ptr @readdir(ptr %handle)");
        line("  %done = icmp eq ptr %entry.ptr, null");
        line("  br i1 %done, label %empty, label %name");
        line("name:");
        // Linux/glibc x86_64 dirent layout: d_name starts at byte offset 19.
        line("  %name.ptr = getelementptr inbounds i8, ptr %entry.ptr, i64 19");
        line("  %b0 = load i8, ptr %name.ptr, align 1");
        line("  %b1.ptr = getelementptr inbounds i8, ptr %name.ptr, i64 1");
        line("  %b1 = load i8, ptr %b1.ptr, align 1");
        line("  %b2.ptr = getelementptr inbounds i8, ptr %name.ptr, i64 2");
        line("  %b2 = load i8, ptr %b2.ptr, align 1");
        line("  %b0.dot = icmp eq i8 %b0, 46");
        line("  %b1.nul = icmp eq i8 %b1, 0");
        line("  %single.dot = and i1 %b0.dot, %b1.nul");
        line("  %b1.dot = icmp eq i8 %b1, 46");
        line("  %b2.nul = icmp eq i8 %b2, 0");
        line("  %dotdot.prefix = and i1 %b0.dot, %b1.dot");
        line("  %double.dot = and i1 %dotdot.prefix, %b2.nul");
        line("  %skip = or i1 %single.dot, %double.dot");
        line("  br i1 %skip, label %next, label %found");
        line("found:");
        line("  ret ptr %name.ptr");
        line("empty:");
        line("  ret ptr " + empty);
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_set_mode(ptr %path, i64 %mode) {");
        line("entry:");
        line("  %too_low = icmp slt i64 %mode, 0");
        line("  %too_high = icmp sgt i64 %mode, 511");
        line("  %bad = or i1 %too_low, %too_high");
        line("  br i1 %bad, label %fail, label %set");
        line("set:");
        line("  %mode32 = trunc i64 %mode to i32");
        line("  %code = call i32 @chmod(ptr %path, i32 %mode32)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "{ i64 } @ari_builtin_fs_open(ptr %path, ptr %mode) {");
        line("entry:");
        line("  %m0 = load i8, ptr %mode, align 1");
        line("  %m0.nul = icmp eq i8 %m0, 0");
        line("  br i1 %m0.nul, label %invalid, label %parse_first");
        line("parse_first:");
        line("  %m1.ptr = getelementptr inbounds i8, ptr %mode, i64 1");
        line("  %m1 = load i8, ptr %m1.ptr, align 1");
        line("  %m0.r = icmp eq i8 %m0, 114");
        line("  %m0.w = icmp eq i8 %m0, 119");
        line("  %m0.a = icmp eq i8 %m0, 97");
        line("  %m1.nul = icmp eq i8 %m1, 0");
        line("  %single.r = and i1 %m0.r, %m1.nul");
        line("  %single.w = and i1 %m0.w, %m1.nul");
        line("  %single.a = and i1 %m0.a, %m1.nul");
        line("  %single.valid.0 = or i1 %single.r, %single.w");
        line("  %single.valid = or i1 %single.valid.0, %single.a");
        line("  br i1 %m1.nul, label %choose, label %compound");
        line("compound:");
        line("  %m2.ptr = getelementptr inbounds i8, ptr %mode, i64 2");
        line("  %m2 = load i8, ptr %m2.ptr, align 1");
        line("  %m1.w = icmp eq i8 %m1, 119");
        line("  %m1.plus = icmp eq i8 %m1, 43");
        line("  %m2.nul = icmp eq i8 %m2, 0");
        line("  %rw.prefix = and i1 %m0.r, %m1.w");
        line("  %compound.rw = and i1 %rw.prefix, %m2.nul");
        line("  %rplus.prefix = and i1 %m0.r, %m1.plus");
        line("  %compound.rplus = and i1 %rplus.prefix, %m2.nul");
        line("  %wplus.prefix = and i1 %m0.w, %m1.plus");
        line("  %compound.wplus = and i1 %wplus.prefix, %m2.nul");
        line("  %aplus.prefix = and i1 %m0.a, %m1.plus");
        line("  %compound.aplus = and i1 %aplus.prefix, %m2.nul");
        line("  %compound.valid.0 = or i1 %compound.rw, %compound.rplus");
        line("  %compound.valid.1 = or i1 %compound.valid.0, %compound.wplus");
        line("  %compound.valid = or i1 %compound.valid.1, %compound.aplus");
        line("  br label %choose");
        line("choose:");
        line("  %mode.r = phi i1 [ %single.r, %parse_first ], [ false, %compound ]");
        line("  %mode.w = phi i1 [ %single.w, %parse_first ], [ false, %compound ]");
        line("  %mode.a = phi i1 [ %single.a, %parse_first ], [ false, %compound ]");
        line("  %mode.rw = phi i1 [ false, %parse_first ], [ %compound.rw, %compound ]");
        line("  %mode.rplus = phi i1 [ false, %parse_first ], [ %compound.rplus, %compound ]");
        line("  %mode.wplus = phi i1 [ false, %parse_first ], [ %compound.wplus, %compound ]");
        line("  %mode.aplus = phi i1 [ false, %parse_first ], [ %compound.aplus, %compound ]");
        line("  %valid = phi i1 [ %single.valid, %parse_first ], [ %compound.valid, %compound ]");
        line("  br i1 %valid, label %open, label %invalid");
        line("open:");
        line("  %flags.r = select i1 %mode.r, i32 0, i32 -1");
        line("  %flags.w = select i1 %mode.w, i32 577, i32 %flags.r");
        line("  %flags.a = select i1 %mode.a, i32 1089, i32 %flags.w");
        line("  %flags.rw = select i1 %mode.rw, i32 2, i32 %flags.a");
        line("  %rplus.any = or i1 %mode.rplus, %mode.rw");
        line("  %flags.rplus = select i1 %rplus.any, i32 2, i32 %flags.rw");
        line("  %flags.wplus = select i1 %mode.wplus, i32 578, i32 %flags.rplus");
        line("  %flags = select i1 %mode.aplus, i32 1090, i32 %flags.wplus");
        line("  %fd32 = call i32 @open(ptr %path, i32 %flags, i32 420)");
        line("  %fd = sext i32 %fd32 to i64");
        line("  %file = insertvalue { i64 } undef, i64 %fd, 0");
        line("  ret { i64 } %file");
        line("invalid:");
        line("  %invalid.file = insertvalue { i64 } undef, i64 -1, 0");
        line("  ret { i64 } %invalid.file");
        line("}");
        line();

        line("define " + runtime_visibility + "{ i64 } @ari_builtin_fs_open_read(ptr %path) {");
        line("entry:");
        line("  %file = call { i64 } @ari_builtin_fs_open(ptr %path, ptr " + fs_mode_read + ")");
        line("  ret { i64 } %file");
        line("}");
        line();

        line("define " + runtime_visibility + "{ i64 } @ari_builtin_fs_open_write(ptr %path) {");
        line("entry:");
        line("  %file = call { i64 } @ari_builtin_fs_open(ptr %path, ptr " + fs_mode_write + ")");
        line("  ret { i64 } %file");
        line("}");
        line();

        line("define " + runtime_visibility + "{ i64 } @ari_builtin_fs_open_append(ptr %path) {");
        line("entry:");
        line("  %file = call { i64 } @ari_builtin_fs_open(ptr %path, ptr " + fs_mode_append + ")");
        line("  ret { i64 } %file");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_close({ i64 } %file) {");
        line("entry:");
        line("  %fd = extractvalue { i64 } %file, 0");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %do_close");
        line("do_close:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %code = call i32 @close(i32 %fd32)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_os_close(i64 %fd) {");
        line("entry:");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %do_close");
        line("do_close:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %code = call i32 @close(i32 %fd32)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_os_dup(i64 %fd) {");
        line("entry:");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %do_dup");
        line("do_dup:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %copy = call i32 @dup(i32 %fd32)");
        line("  %wide = sext i32 %copy to i64");
        line("  ret i64 %wide");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_os_close_on_exec(i64 %fd) {");
        line("entry:");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %query");
        line("query:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %flags = call i32 (i32, i32, ...) @fcntl(i32 %fd32, i32 1)");
        line("  %bad = icmp slt i32 %flags, 0");
        line("  br i1 %bad, label %fail, label %ok");
        line("ok:");
        line("  %masked = and i32 %flags, 1");
        line("  %enabled = icmp ne i32 %masked, 0");
        line("  %wide = zext i1 %enabled to i64");
        line("  ret i64 %wide");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_os_set_close_on_exec(i64 %fd, i1 %enabled) {");
        line("entry:");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %query");
        line("query:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %flags = call i32 (i32, i32, ...) @fcntl(i32 %fd32, i32 1)");
        line("  %bad = icmp slt i32 %flags, 0");
        line("  br i1 %bad, label %fail, label %set");
        line("set:");
        line("  %with = or i32 %flags, 1");
        line("  %without = and i32 %flags, -2");
        line("  %next = select i1 %enabled, i32 %with, i32 %without");
        line("  %code = call i32 (i32, i32, ...) @fcntl(i32 %fd32, i32 2, i32 %next)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_os_nonblocking(i64 %fd) {");
        line("entry:");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %query");
        line("query:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %flags = call i32 (i32, i32, ...) @fcntl(i32 %fd32, i32 3)");
        line("  %bad = icmp slt i32 %flags, 0");
        line("  br i1 %bad, label %fail, label %ok");
        line("ok:");
        line("  %masked = and i32 %flags, 2048");
        line("  %enabled = icmp ne i32 %masked, 0");
        line("  %wide = zext i1 %enabled to i64");
        line("  ret i64 %wide");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_os_set_nonblocking(i64 %fd, i1 %enabled) {");
        line("entry:");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %query");
        line("query:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %flags = call i32 (i32, i32, ...) @fcntl(i32 %fd32, i32 3)");
        line("  %bad = icmp slt i32 %flags, 0");
        line("  br i1 %bad, label %fail, label %set");
        line("set:");
        line("  %with = or i32 %flags, 2048");
        line("  %without = and i32 %flags, -2049");
        line("  %next = select i1 %enabled, i32 %with, i32 %without");
        line("  %code = call i32 (i32, i32, ...) @fcntl(i32 %fd32, i32 4, i32 %next)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  ret i1 %ok");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_os_pipe() {");
        line("entry:");
        line("  %fds = alloca [2 x i32], align 4");
        line("  %read_ptr = getelementptr inbounds [2 x i32], ptr %fds, i64 0, i64 0");
        line("  %code = call i32 @pipe(ptr %read_ptr)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  br i1 %ok, label %pack, label %fail");
        line("pack:");
        line("  %write_ptr = getelementptr inbounds [2 x i32], ptr %fds, i64 0, i64 1");
        line("  %read_fd = load i32, ptr %read_ptr, align 4");
        line("  %write_fd = load i32, ptr %write_ptr, align 4");
        line("  %read64 = zext i32 %read_fd to i64");
        line("  %write64 = zext i32 %write_fd to i64");
        line("  %shifted = shl i64 %write64, 32");
        line("  %packed = or i64 %shifted, %read64");
        line("  ret i64 %packed");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_os_read_byte(i64 %fd) {");
        line("entry:");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %do_read");
        line("do_read:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %byte.ptr = alloca i8, align 1");
        line("  %count = call i64 @read(i32 %fd32, ptr %byte.ptr, i64 1)");
        line("  %one = icmp eq i64 %count, 1");
        line("  br i1 %one, label %load, label %fail");
        line("load:");
        line("  %byte = load i8, ptr %byte.ptr, align 1");
        line("  %wide = zext i8 %byte to i64");
        line("  ret i64 %wide");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_os_write_byte(i64 %fd, i8 %value) {");
        line("entry:");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %do_write");
        line("do_write:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %byte.ptr = alloca i8, align 1");
        line("  store i8 %value, ptr %byte.ptr, align 1");
        line("  %count = call i64 @write(i32 %fd32, ptr %byte.ptr, i64 1)");
        line("  %ok = icmp eq i64 %count, 1");
        line("  ret i1 %ok");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_fs_read_byte({ i64 } %file) {");
        line("entry:");
        line("  %fd = extractvalue { i64 } %file, 0");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %do_read");
        line("do_read:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %byte.ptr = alloca i8, align 1");
        line("  %count = call i64 @read(i32 %fd32, ptr %byte.ptr, i64 1)");
        line("  %one = icmp eq i64 %count, 1");
        line("  br i1 %one, label %load, label %fail");
        line("load:");
        line("  %byte = load i8, ptr %byte.ptr, align 1");
        line("  %wide = zext i8 %byte to i64");
        line("  ret i64 %wide");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_write_byte({ i64 } %file, i8 %value) {");
        line("entry:");
        line("  %fd = extractvalue { i64 } %file, 0");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %do_write");
        line("do_write:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %byte.ptr = alloca i8, align 1");
        line("  store i8 %value, ptr %byte.ptr, align 1");
        line("  %count = call i64 @write(i32 %fd32, ptr %byte.ptr, i64 1)");
        line("  %ok = icmp eq i64 %count, 1");
        line("  ret i1 %ok");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_fs_position({ i64 } %file) {");
        line("entry:");
        line("  %fd = extractvalue { i64 } %file, 0");
        line("  %invalid = icmp slt i64 %fd, 0");
        line("  br i1 %invalid, label %fail, label %query");
        line("query:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %position = call i64 @lseek(i32 %fd32, i64 0, i32 1)");
        line("  ret i64 %position");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_fs_seek({ i64 } %file, i64 %position) {");
        line("entry:");
        line("  %fd = extractvalue { i64 } %file, 0");
        line("  %invalid.fd = icmp slt i64 %fd, 0");
        line("  %invalid.position = icmp slt i64 %position, 0");
        line("  %invalid = or i1 %invalid.fd, %invalid.position");
        line("  br i1 %invalid, label %fail, label %do_seek");
        line("do_seek:");
        line("  %fd32 = trunc i64 %fd to i32");
        line("  %result = call i64 @lseek(i32 %fd32, i64 %position, i32 0)");
        line("  %ok = icmp sge i64 %result, 0");
        line("  ret i1 %ok");
        line("fail:");
        line("  ret i1 false");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_mem_copy_bytes(ptr %target, ptr %source, i64 %len) {");
        line("entry:");
        line("  %bad.len = icmp slt i64 %len, 0");
        line("  br i1 %bad.len, label %fail, label %copy");
        line("copy:");
        line("  call void @llvm.memcpy.p0.p0.i64(ptr %target, ptr %source, i64 %len, i1 false)");
        line("  ret void");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_mem_move_bytes(ptr %target, ptr %source, i64 %len) {");
        line("entry:");
        line("  %bad.len = icmp slt i64 %len, 0");
        line("  br i1 %bad.len, label %fail, label %move");
        line("move:");
        line("  call void @llvm.memmove.p0.p0.i64(ptr %target, ptr %source, i64 %len, i1 false)");
        line("  ret void");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_mem_set_bytes(ptr %target, i8 %value, i64 %len) {");
        line("entry:");
        line("  %bad.len = icmp slt i64 %len, 0");
        line("  br i1 %bad.len, label %fail, label %set");
        line("set:");
        line("  call void @llvm.memset.p0.i64(ptr %target, i8 %value, i64 %len, i1 false)");
        line("  ret void");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_mem_page_size() {");
        line("entry:");
        line("  %size = call i32 @getpagesize()");
        line("  %wide = zext i32 %size to i64");
        line("  ret i64 %wide");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_process_id() {");
        line("entry:");
        line("  %pid = call i32 @getpid()");
        line("  %wide = sext i32 %pid to i64");
        line("  ret i64 %wide");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_process_uid() {");
        line("entry:");
        line("  %uid = call i32 @getuid()");
        line("  %wide = zext i32 %uid to i64");
        line("  ret i64 %wide");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_process_gid() {");
        line("entry:");
        line("  %gid = call i32 @getgid()");
        line("  %wide = zext i32 %gid to i64");
        line("  ret i64 %wide");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_process_exit(i64 %code) {");
        line("entry:");
        line("  %narrow = trunc i64 %code to i32");
        line("  call void @exit(i32 %narrow)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_process_abort() {");
        line("entry:");
        line("  call void @abort()");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_process_fork() {");
        line("entry:");
        line("  %pid32 = call i32 @fork()");
        line("  %pid = sext i32 %pid32 to i64");
        line("  ret i64 %pid");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_process_wait(i64 %pid) {");
        line("entry:");
        line("  %status.ptr = alloca i32, align 4");
        line("  %pid32 = trunc i64 %pid to i32");
        line("  %waited = call i32 @waitpid(i32 %pid32, ptr %status.ptr, i32 0)");
        line("  %failed = icmp sle i32 %waited, 0");
        line("  br i1 %failed, label %fail, label %decode");
        line("decode:");
        line("  %status = load i32, ptr %status.ptr, align 4");
        line("  %signal.bits = and i32 %status, 127");
        line("  %exited = icmp eq i32 %signal.bits, 0");
        line("  br i1 %exited, label %exit_status, label %fail");
        line("exit_status:");
        line("  %shifted = ashr i32 %status, 8");
        line("  %code32 = and i32 %shifted, 255");
        line("  %code = sext i32 %code32 to i64");
        line("  ret i64 %code");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define private ptr @ari_thread_trampoline(ptr %packet) {");
        line("entry:");
        line("  %thread.entry = load ptr, ptr %packet, align 8");
        line("  %id.ptr = getelementptr i8, ptr %packet, i64 8");
        line("  %id = load i64, ptr %id.ptr, align 8");
        line("  store i64 %id, ptr @ari_thread_id");
        line("  call void @free(ptr %packet)");
        line("  %result = call i64 %thread.entry()");
        line("  %as.ptr = inttoptr i64 %result to ptr");
        line("  ret ptr %as.ptr");
        line("}");
        line();

        line("define " + runtime_visibility + "{ i64, i64 } @ari_builtin_thread_spawn(ptr %start) {");
        line("entry:");
        line("  %native.ptr = alloca i64, align 8");
        line("  %packet = call ptr @malloc(i64 16)");
        line("  %alloc.failed = icmp eq ptr %packet, null");
        line("  br i1 %alloc.failed, label %fail, label %prepare");
        line("prepare:");
        line("  store ptr %start, ptr %packet, align 8");
        line("  %id.ptr = getelementptr i8, ptr %packet, i64 8");
        line("  %id = atomicrmw add ptr @ari_next_thread_id, i64 1 monotonic");
        line("  store i64 %id, ptr %id.ptr, align 8");
        line("  %code = call i32 @pthread_create(ptr %native.ptr, ptr null, ptr @ari_thread_trampoline, ptr %packet)");
        line("  %create.failed = icmp ne i32 %code, 0");
        line("  br i1 %create.failed, label %create_fail, label %ok");
        line("ok:");
        line("  %native = load i64, ptr %native.ptr, align 8");
        line("  %with.native = insertvalue { i64, i64 } undef, i64 %native, 0");
        line("  %with.id = insertvalue { i64, i64 } %with.native, i64 %id, 1");
        line("  ret { i64, i64 } %with.id");
        line("create_fail:");
        line("  call void @free(ptr %packet)");
        line("  br label %fail");
        line("fail:");
        line("  %fail.native = insertvalue { i64, i64 } undef, i64 -1, 0");
        line("  %fail.thread = insertvalue { i64, i64 } %fail.native, i64 -1, 1");
        line("  ret { i64, i64 } %fail.thread");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_thread_join({ i64, i64 } %thread) {");
        line("entry:");
        line("  %native = extractvalue { i64, i64 } %thread, 0");
        line("  %invalid = icmp slt i64 %native, 0");
        line("  br i1 %invalid, label %fail, label %join");
        line("join:");
        line("  %result.ptr = alloca ptr, align 8");
        line("  %code = call i32 @pthread_join(i64 %native, ptr %result.ptr)");
        line("  %join.failed = icmp ne i32 %code, 0");
        line("  br i1 %join.failed, label %fail, label %ok");
        line("ok:");
        line("  %result.raw = load ptr, ptr %result.ptr, align 8");
        line("  %result = ptrtoint ptr %result.raw to i64");
        line("  ret i64 %result");
        line("fail:");
        line("  ret i64 -1");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_thread_yield() {");
        line("entry:");
        line("  %ignored = call i32 @sched_yield()");
        line("  ret void");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_thread_available_parallelism() {");
        line("entry:");
        line("  %count = call i64 @sysconf(i32 " + std::to_string(kScNprocessorsOnln) + ")");
        line("  %invalid = icmp slt i64 %count, 1");
        line("  br i1 %invalid, label %fallback, label %ok");
        line("ok:");
        line("  ret i64 %count");
        line("fallback:");
        line("  ret i64 1");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_sync_atomic_i64_load(ptr %cell) {");
        line("entry:");
        line("  %value.ptr = getelementptr inbounds { i64 }, ptr %cell, i32 0, i32 0");
        line("  %value = load atomic i64, ptr %value.ptr seq_cst, align 8");
        line("  ret i64 %value");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_sync_atomic_i64_store(ptr %cell, i64 %next) {");
        line("entry:");
        line("  %value.ptr = getelementptr inbounds { i64 }, ptr %cell, i32 0, i32 0");
        line("  store atomic i64 %next, ptr %value.ptr seq_cst, align 8");
        line("  ret void");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_sync_atomic_i64_swap(ptr %cell, i64 %next) {");
        line("entry:");
        line("  %value.ptr = getelementptr inbounds { i64 }, ptr %cell, i32 0, i32 0");
        line("  %previous = atomicrmw xchg ptr %value.ptr, i64 %next seq_cst");
        line("  ret i64 %previous");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_sync_atomic_i64_fetch_add(ptr %cell, i64 %amount) {");
        line("entry:");
        line("  %value.ptr = getelementptr inbounds { i64 }, ptr %cell, i32 0, i32 0");
        line("  %previous = atomicrmw add ptr %value.ptr, i64 %amount seq_cst");
        line("  ret i64 %previous");
        line("}");
        line();

        line("define " + runtime_visibility + "i1 @ari_builtin_sync_atomic_i64_compare_exchange(ptr %cell, i64 %expected, i64 %next) {");
        line("entry:");
        line("  %value.ptr = getelementptr inbounds { i64 }, ptr %cell, i32 0, i32 0");
        line("  %result = cmpxchg ptr %value.ptr, i64 %expected, i64 %next seq_cst seq_cst");
        line("  %exchanged = extractvalue { i64, i1 } %result, 1");
        line("  ret i1 %exchanged");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_time_monotonic_nanos() {");
        line("entry:");
        line("  %ts = alloca { i64, i64 }, align 8");
        line("  %code = call i32 @clock_gettime(i32 1, ptr %ts)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  br i1 %ok, label %load, label %fail");
        line("load:");
        line("  %sec.ptr = getelementptr inbounds { i64, i64 }, ptr %ts, i32 0, i32 0");
        line("  %nsec.ptr = getelementptr inbounds { i64, i64 }, ptr %ts, i32 0, i32 1");
        line("  %sec = load i64, ptr %sec.ptr");
        line("  %nsec = load i64, ptr %nsec.ptr");
        line("  %sec.ns = mul i64 %sec, 1000000000");
        line("  %total = add i64 %sec.ns, %nsec");
        line("  ret i64 %total");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_time_unix_nanos() {");
        line("entry:");
        line("  %ts = alloca { i64, i64 }, align 8");
        line("  %code = call i32 @clock_gettime(i32 0, ptr %ts)");
        line("  %ok = icmp eq i32 %code, 0");
        line("  br i1 %ok, label %load, label %fail");
        line("load:");
        line("  %sec.ptr = getelementptr inbounds { i64, i64 }, ptr %ts, i32 0, i32 0");
        line("  %nsec.ptr = getelementptr inbounds { i64, i64 }, ptr %ts, i32 0, i32 1");
        line("  %sec = load i64, ptr %sec.ptr");
        line("  %nsec = load i64, ptr %nsec.ptr");
        line("  %sec.ns = mul i64 %sec, 1000000000");
        line("  %total = add i64 %sec.ns, %nsec");
        line("  ret i64 %total");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_time_sleep_nanos(i64 %nanos) {");
        line("entry:");
        line("  %negative = icmp slt i64 %nanos, 0");
        line("  br i1 %negative, label %fail, label %sleep");
        line("sleep:");
        line("  %ts = alloca { i64, i64 }, align 8");
        line("  %sec = sdiv i64 %nanos, 1000000000");
        line("  %nsec = srem i64 %nanos, 1000000000");
        line("  %sec.ptr = getelementptr inbounds { i64, i64 }, ptr %ts, i32 0, i32 0");
        line("  %nsec.ptr = getelementptr inbounds { i64, i64 }, ptr %ts, i32 0, i32 1");
        line("  store i64 %sec, ptr %sec.ptr");
        line("  store i64 %nsec, ptr %nsec.ptr");
        line("  call i32 @nanosleep(ptr %ts, ptr null)");
        line("  ret void");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_random_fill(ptr %data, i64 %len) {");
        line("entry:");
        line("  %bad.len = icmp slt i64 %len, 0");
        line("  br i1 %bad.len, label %fail, label %check_empty");
        line("check_empty:");
        line("  %empty = icmp eq i64 %len, 0");
        line("  br i1 %empty, label %done, label %getrandom_loop");
        line("getrandom_loop:");
        line("  %offset = phi i64 [ 0, %check_empty ], [ %next.offset, %advance_getrandom ]");
        line("  %remaining = sub i64 %len, %offset");
        line("  %out.ptr = getelementptr i8, ptr %data, i64 %offset");
        line("  %got = call i64 @getrandom(ptr %out.ptr, i64 %remaining, i32 0)");
        line("  %made.progress = icmp sgt i64 %got, 0");
        line("  br i1 %made.progress, label %advance_getrandom, label %fallback_open");
        line("advance_getrandom:");
        line("  %next.offset = add i64 %offset, %got");
        line("  %done.getrandom = icmp uge i64 %next.offset, %len");
        line("  br i1 %done.getrandom, label %done, label %getrandom_loop");
        line("fallback_open:");
        line("  %fd = call i32 @open(ptr " + dev_urandom + ", i32 0, i32 0)");
        line("  %opened = icmp sge i32 %fd, 0");
        line("  br i1 %opened, label %fallback_loop, label %fail");
        line("fallback_loop:");
        line("  %fallback.offset = phi i64 [ %offset, %fallback_open ], [ %fallback.next, %fallback_advance ]");
        line("  %fallback.remaining = sub i64 %len, %fallback.offset");
        line("  %fallback.ptr = getelementptr i8, ptr %data, i64 %fallback.offset");
        line("  %read = call i64 @read(i32 %fd, ptr %fallback.ptr, i64 %fallback.remaining)");
        line("  %read.progress = icmp sgt i64 %read, 0");
        line("  br i1 %read.progress, label %fallback_advance, label %fallback_fail");
        line("fallback_advance:");
        line("  %fallback.next = add i64 %fallback.offset, %read");
        line("  %fallback.done = icmp uge i64 %fallback.next, %len");
        line("  br i1 %fallback.done, label %fallback_close, label %fallback_loop");
        line("fallback_close:");
        line("  %ignored.close = call i32 @close(i32 %fd)");
        line("  ret void");
        line("fallback_fail:");
        line("  %ignored.close.fail = call i32 @close(i32 %fd)");
        line("  br label %fail");
        line("done:");
        line("  ret void");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_random_entropy() {");
        line("entry:");
        line("  %slot = alloca i64, align 8");
        line("  call void @ari_builtin_random_fill(ptr %slot, i64 8)");
        line("  %value = load i64, ptr %slot, align 8");
        line("  ret i64 %value");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_write_i64(i64 %value) {");
        line("entry:");
        line("  call i32 (ptr, ...) @printf(ptr " + fmt_i64 + ", i64 %value)");
        line("  ret i64 0");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_write_u64(i64 %value) {");
        line("entry:");
        line("  call i32 (ptr, ...) @printf(ptr " + fmt_u64 + ", i64 %value)");
        line("  ret i64 0");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_write_bool(i1 %value) {");
        line("entry:");
        line("  %text = select i1 %value, ptr " + text_true + ", ptr " + text_false);
        line("  call i32 (ptr, ...) @printf(ptr " + fmt_text + ", ptr %text)");
        line("  ret i64 0");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_write_byte(i8 %value) {");
        line("entry:");
        line("  %wide = zext i8 %value to i32");
        line("  call i32 @putchar(i32 %wide)");
        line("  ret i64 0");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_write_error_byte(i8 %value) {");
        line("entry:");
        line("  %slot = alloca i8, align 1");
        line("  store i8 %value, ptr %slot, align 1");
        line("  %written = call i64 @write(i32 2, ptr %slot, i64 1)");
        line("  ret i64 %written");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_newline() {");
        line("entry:");
        line("  call i32 @putchar(i32 10)");
        line("  ret i64 0");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_read_byte() {");
        line("entry:");
        line("  %ch = call i32 @getchar()");
        line("  %wide = sext i32 %ch to i64");
        line("  ret i64 %wide");
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_read_line() {");
        line("entry:");
        line("  %stdin.file = load ptr, ptr @stdin");
        line("  %line = call ptr @fgets(ptr " + line_buffer + ", i32 4096, ptr %stdin.file)");
        line("  %is_empty = icmp eq ptr %line, null");
        line("  br i1 %is_empty, label %empty, label %strip");
        line("empty:");
        line("  ret ptr " + empty);
        line("strip:");
        line("  br label %scan");
        line("scan:");
        line("  %i = phi i64 [0, %strip], [%next, %scan.cont]");
        line("  %slot = getelementptr inbounds [4096 x i8], ptr @ari_line_buffer, i64 0, i64 %i");
        line("  %ch = load i8, ptr %slot");
        line("  %nul = icmp eq i8 %ch, 0");
        line("  br i1 %nul, label %done, label %scan.eol");
        line("scan.eol:");
        line("  %lf = icmp eq i8 %ch, 10");
        line("  %cr = icmp eq i8 %ch, 13");
        line("  %eol = or i1 %lf, %cr");
        line("  br i1 %eol, label %trim, label %scan.cont");
        line("trim:");
        line("  store i8 0, ptr %slot");
        line("  br label %done");
        line("scan.cont:");
        line("  %next = add i64 %i, 1");
        line("  %keep_scanning = icmp ult i64 %next, 4096");
        line("  br i1 %keep_scanning, label %scan, label %done");
        line("done:");
        line("  ret ptr " + line_buffer);
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_zone_create(i64 %capacity) {");
        line("entry:");
        line("  %nonpositive.capacity = icmp sle i64 %capacity, 0");
        line("  %too.large.capacity = icmp ugt i64 %capacity, " + zone_max_capacity);
        line("  %bad.capacity = or i1 %nonpositive.capacity, %too.large.capacity");
        line("  br i1 %bad.capacity, label %fail, label %alloc.data");
        line("alloc.data:");
        line("  %raw.scaled = mul i64 %capacity, " + zone_arena_scale);
        line("  %raw.capacity = add i64 %raw.scaled, " + zone_arena_slack);
        line("  %data = call ptr @malloc(i64 %raw.capacity)");
        line("  %data.null = icmp eq ptr %data, null");
        line("  br i1 %data.null, label %fail, label %alloc.zone");
        line("alloc.zone:");
        line("  %zone = call ptr @malloc(i64 " + zone_struct_bytes + ")");
        line("  %zone.null = icmp eq ptr %zone, null");
        line("  br i1 %zone.null, label %free.data, label %init");
        line("free.data:");
        line("  call void @free(ptr %data)");
        line("  br label %fail");
        line("init:");
        line("  %capacity.slot = getelementptr i64, ptr %zone, i64 0");
        line("  store i64 %capacity, ptr %capacity.slot");
        line("  %offset.slot = getelementptr i64, ptr %zone, i64 1");
        line("  store i64 0, ptr %offset.slot");
        line("  %data.slot = getelementptr i8, ptr %zone, i64 16");
        line("  store ptr %data, ptr %data.slot");
        line("  %used.slot = getelementptr i64, ptr %zone, i64 3");
        line("  store i64 0, ptr %used.slot");
        line("  %raw.capacity.slot = getelementptr i64, ptr %zone, i64 4");
        line("  store i64 %raw.capacity, ptr %raw.capacity.slot");
        line("  ret ptr %zone");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_zone_alloc(ptr %zone.slot, i64 %bytes, i64 %align) {");
        line("entry:");
        line("  %zone = load ptr, ptr %zone.slot");
        line("  %zone.null = icmp eq ptr %zone, null");
        line("  %bytes.bad = icmp sle i64 %bytes, 0");
        line("  %align.bad = icmp sle i64 %align, 0");
        line("  %bad.0 = or i1 %zone.null, %bytes.bad");
        line("  %bad.1 = or i1 %bad.0, %align.bad");
        line("  br i1 %bad.1, label %fail, label %check.align");
        line("check.align:");
        line("  %mask = sub i64 %align, 1");
        line("  %power.bits = and i64 %align, %mask");
        line("  %power.bad = icmp ne i64 %power.bits, 0");
        line("  br i1 %power.bad, label %fail, label %load");
        line("load:");
        line("  %capacity.slot = getelementptr i64, ptr %zone, i64 0");
        line("  %capacity = load i64, ptr %capacity.slot");
        line("  %raw.offset.slot = getelementptr i64, ptr %zone, i64 1");
        line("  %raw.offset = load i64, ptr %raw.offset.slot");
        line("  %logical.used.slot = getelementptr i64, ptr %zone, i64 3");
        line("  %logical.used = load i64, ptr %logical.used.slot");
        line("  %raw.capacity.slot = getelementptr i64, ptr %zone, i64 4");
        line("  %raw.capacity = load i64, ptr %raw.capacity.slot");
        line("  %logical.new = add i64 %logical.used, %bytes");
        line("  %logical.overflow = icmp ult i64 %logical.new, %logical.used");
        line("  %logical.too.big = icmp ugt i64 %logical.new, %capacity");
        line("  %bad.logical = or i1 %logical.overflow, %logical.too.big");
        line("  br i1 %bad.logical, label %fail, label %layout");
        line("layout:");
        line("  %needs.min.align = icmp ult i64 %align, " + zone_min_payload_align);
        line("  %effective.align = select i1 %needs.min.align, i64 " + zone_min_payload_align + ", i64 %align");
        line("  %effective.mask = sub i64 %effective.align, 1");
        line("  %payload.unaligned = add i64 %raw.offset, " + zone_header_bytes);
        line("  %payload.unaligned.overflow = icmp ult i64 %payload.unaligned, %raw.offset");
        line("  %payload.biased = add i64 %payload.unaligned, %effective.mask");
        line("  %payload.biased.overflow = icmp ult i64 %payload.biased, %payload.unaligned");
        line("  %neg.effective.align = sub i64 0, %effective.align");
        line("  %payload.offset = and i64 %payload.biased, %neg.effective.align");
        line("  %header.offset = sub i64 %payload.offset, " + zone_header_bytes);
        line("  %raw.new = add i64 %payload.offset, %bytes");
        line("  %raw.overflow = icmp ult i64 %raw.new, %payload.offset");
        line("  %raw.too.big = icmp ugt i64 %raw.new, %raw.capacity");
        line("  %bad.raw.0 = or i1 %payload.unaligned.overflow, %payload.biased.overflow");
        line("  %bad.raw.1 = or i1 %raw.overflow, %raw.too.big");
        line("  %bad.raw = or i1 %bad.raw.0, %bad.raw.1");
        line("  br i1 %bad.raw, label %fail, label %commit");
        line("commit:");
        line("  store i64 %raw.new, ptr %raw.offset.slot");
        line("  store i64 %logical.new, ptr %logical.used.slot");
        line("  %data.slot = getelementptr i8, ptr %zone, i64 16");
        line("  %data = load ptr, ptr %data.slot");
        line("  %header = getelementptr i8, ptr %data, i64 %header.offset");
        line("  store ptr %zone, ptr %header");
        line("  %out = getelementptr i8, ptr %data, i64 %payload.offset");
        line("  ret ptr %out");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_zone_allocation_zone(ptr %data) {");
        line("entry:");
        line("  %null = icmp eq ptr %data, null");
        line("  br i1 %null, label %fail, label %load");
        line("load:");
        line("  %zone.slot = getelementptr i8, ptr %data, i64 " + zone_header_zone_offset);
        line("  %zone = load ptr, ptr %zone.slot");
        line("  %zone.null = icmp eq ptr %zone, null");
        line("  br i1 %zone.null, label %fail, label %ok");
        line("ok:");
        line("  ret ptr %zone");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "ptr @ari_builtin_string_alloc_buffer(ptr %zone.slot, i64 %capacity) {");
        line("entry:");
        line("  %valid = icmp sge i64 %capacity, 0");
        line("  br i1 %valid, label %check.empty, label %fail");
        line("check.empty:");
        line("  %empty = icmp eq i64 %capacity, 0");
        line("  br i1 %empty, label %empty.ret, label %alloc");
        line("empty.ret:");
        line("  ret ptr null");
        line("alloc:");
        line("  %ptr = call ptr @ari_builtin_zone_alloc(ptr %zone.slot, i64 %capacity, i64 1)");
        line("  ret ptr %ptr");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "{ ptr, i64, i64 } @ari_builtin_string_with_capacity(ptr %zone.slot, i64 %capacity) {");
        line("entry:");
        line("  %data = call ptr @ari_builtin_string_alloc_buffer(ptr %zone.slot, i64 %capacity)");
        line("  %with.data = insertvalue { ptr, i64, i64 } undef, ptr %data, 0");
        line("  %with.len = insertvalue { ptr, i64, i64 } %with.data, i64 0, 1");
        line("  %with.capacity = insertvalue { ptr, i64, i64 } %with.len, i64 %capacity, 2");
        line("  ret { ptr, i64, i64 } %with.capacity");
        line("}");
        line();

        line("define " + runtime_visibility + "{ { ptr, i64, i64 } } @ari_builtin_string_new(ptr %zone.slot, i64 %capacity) {");
        line("entry:");
        line("  %raw = call { ptr, i64, i64 } @ari_builtin_string_with_capacity(ptr %zone.slot, i64 %capacity)");
        line("  %out = insertvalue { { ptr, i64, i64 } } undef, { ptr, i64, i64 } %raw, 0");
        line("  ret { { ptr, i64, i64 } } %out");
        line("}");
        line();

        line("define " + runtime_visibility + "{ { ptr, i64, i64 } } @ari_builtin_string_from_string(ptr %zone.slot, ptr %text) {");
        line("entry:");
        line("  br label %scan");
        line("scan:");
        line("  %index = phi i64 [ 0, %entry ], [ %next, %scan.next ]");
        line("  %source.ptr = getelementptr i8, ptr %text, i64 %index");
        line("  %byte = load i8, ptr %source.ptr");
        line("  %done = icmp eq i8 %byte, 0");
        line("  br i1 %done, label %alloc, label %scan.next");
        line("scan.next:");
        line("  %next = add i64 %index, 1");
        line("  br label %scan");
        line("alloc:");
        line("  %result = call { { ptr, i64, i64 } } @ari_builtin_string_new(ptr %zone.slot, i64 %index)");
        line("  %raw = extractvalue { { ptr, i64, i64 } } %result, 0");
        line("  %dest = extractvalue { ptr, i64, i64 } %raw, 0");
        line("  br label %copy.cond");
        line("copy.cond:");
        line("  %copy.index = phi i64 [ 0, %alloc ], [ %copy.next, %copy.body ]");
        line("  %copy.done = icmp eq i64 %copy.index, %index");
        line("  br i1 %copy.done, label %ret, label %copy.body");
        line("copy.body:");
        line("  %copy.source.ptr = getelementptr i8, ptr %text, i64 %copy.index");
        line("  %copy.byte = load i8, ptr %copy.source.ptr");
        line("  %copy.dest.ptr = getelementptr i8, ptr %dest, i64 %copy.index");
        line("  store i8 %copy.byte, ptr %copy.dest.ptr");
        line("  %copy.next = add i64 %copy.index, 1");
        line("  br label %copy.cond");
        line("ret:");
        line("  %owned.raw = insertvalue { ptr, i64, i64 } %raw, i64 %index, 1");
        line("  %owned.result = insertvalue { { ptr, i64, i64 } } %result, { ptr, i64, i64 } %owned.raw, 0");
        line("  ret { { ptr, i64, i64 } } %owned.result");
        line("}");
        line();

        line("define " + runtime_visibility + "{ { ptr, i64, i64 } } @ari_builtin_string_copy_to(ptr %value.slot, ptr %target.slot) {");
        line("entry:");
        line("  %value = load { { ptr, i64, i64 } }, ptr %value.slot");
        line("  %source.raw = extractvalue { { ptr, i64, i64 } } %value, 0");
        line("  %source.data = extractvalue { ptr, i64, i64 } %source.raw, 0");
        line("  %length = extractvalue { ptr, i64, i64 } %source.raw, 1");
        line("  %result = call { { ptr, i64, i64 } } @ari_builtin_string_new(ptr %target.slot, i64 %length)");
        line("  %dest.raw = extractvalue { { ptr, i64, i64 } } %result, 0");
        line("  %dest.data = extractvalue { ptr, i64, i64 } %dest.raw, 0");
        line("  br label %copy.cond");
        line("copy.cond:");
        line("  %copy.index = phi i64 [ 0, %entry ], [ %copy.next, %copy.body ]");
        line("  %copy.done = icmp eq i64 %copy.index, %length");
        line("  br i1 %copy.done, label %ret, label %copy.body");
        line("copy.body:");
        line("  %source.ptr = getelementptr i8, ptr %source.data, i64 %copy.index");
        line("  %byte = load i8, ptr %source.ptr");
        line("  %dest.ptr = getelementptr i8, ptr %dest.data, i64 %copy.index");
        line("  store i8 %byte, ptr %dest.ptr");
        line("  %copy.next = add i64 %copy.index, 1");
        line("  br label %copy.cond");
        line("ret:");
        line("  %copied.raw = insertvalue { ptr, i64, i64 } %dest.raw, i64 %length, 1");
        line("  %copied.result = insertvalue { { ptr, i64, i64 } } %result, { ptr, i64, i64 } %copied.raw, 0");
        line("  ret { { ptr, i64, i64 } } %copied.result");
        line("}");
        line();

        line("define " + runtime_visibility + "{ { ptr, i64, i64 } } @ari_builtin_read_line_owned(ptr %zone.slot) {");
        line("entry:");
        line("  %line = call ptr @ari_builtin_read_line()");
        line("  %owned = call { { ptr, i64, i64 } } @ari_builtin_string_from_string(ptr %zone.slot, ptr %line)");
        line("  ret { { ptr, i64, i64 } } %owned");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_zone_reset(ptr %zone.slot) {");
        line("entry:");
        line("  %zone = load ptr, ptr %zone.slot");
        line("  %zone.null = icmp eq ptr %zone, null");
        line("  br i1 %zone.null, label %fail, label %reset");
        line("reset:");
        line("  %offset.slot = getelementptr i64, ptr %zone, i64 1");
        line("  store i64 0, ptr %offset.slot");
        line("  %used.slot = getelementptr i64, ptr %zone, i64 3");
        line("  store i64 0, ptr %used.slot");
        line("  ret void");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_zone_destroy(ptr %zone) {");
        line("entry:");
        line("  %zone.null = icmp eq ptr %zone, null");
        line("  br i1 %zone.null, label %fail, label %destroy");
        line("destroy:");
        line("  %data.slot = getelementptr i8, ptr %zone, i64 16");
        line("  %data = load ptr, ptr %data.slot");
        line("  call void @free(ptr %data)");
        line("  call void @free(ptr %zone)");
        line("  ret void");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_assert(i1 %condition) {");
        line("entry:");
        line("  br i1 %condition, label %ok, label %fail");
        line("ok:");
        line("  ret i64 0");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_assert_eq_i64(i64 %left, i64 %right) {");
        line("entry:");
        line("  %ok = icmp eq i64 %left, %right");
        line("  br i1 %ok, label %pass, label %fail");
        line("pass:");
        line("  ret i64 0");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_assert_ne_i64(i64 %left, i64 %right) {");
        line("entry:");
        line("  %ok = icmp ne i64 %left, %right");
        line("  br i1 %ok, label %pass, label %fail");
        line("pass:");
        line("  ret i64 0");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_assert_eq_bool(i1 %left, i1 %right) {");
        line("entry:");
        line("  %ok = icmp eq i1 %left, %right");
        line("  br i1 %ok, label %pass, label %fail");
        line("pass:");
        line("  ret i64 0");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "i64 @ari_builtin_assert_ne_bool(i1 %left, i1 %right) {");
        line("entry:");
        line("  %ok = icmp ne i1 %left, %right");
        line("  br i1 %ok, label %pass, label %fail");
        line("pass:");
        line("  ret i64 0");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define " + runtime_visibility + "void @ari_builtin_panic() {");
        line("entry:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();
    }

    void emit_trait_object_vtables() {
        out_ = &functions_;
        for (const auto& table : program_.trait_object_vtables) {
            trait_object_globals_ << quote_global(table.name) << " = private constant ["
                                  << table.methods.size() << " x ptr] ";
            if (table.methods.empty()) {
                trait_object_globals_ << "zeroinitializer\n";
            } else {
                trait_object_globals_ << "[";
                for (std::size_t i = 0; i < table.methods.size(); ++i) {
                    if (i > 0) trait_object_globals_ << ", ";
                    trait_object_globals_ << "ptr " << quote_global(table.methods[i].thunk_name);
                }
                trait_object_globals_ << "]\n";
            }

            for (const auto& method : table.methods) {
                line("define private " + llvm_type(method.result_type) + " " +
                     quote_global(method.thunk_name) + "(" + param_type_decl(method.erased_params) + ") {");
                line("entry:");
                std::string receiver = temp();
                line("  " + receiver + " = load " + llvm_type(method.concrete_receiver_type) + ", ptr %arg0");

                std::string call = "call " + llvm_type(method.result_type) + " " +
                                   quote_global(function_symbols_.at(method.impl_name)) + "(";
                call += llvm_type(method.concrete_receiver_type) + " " + receiver;
                for (std::size_t i = 1; i < method.impl_params.size(); ++i) {
                    call += ", " + llvm_type(method.impl_params[i]) + " %arg" + std::to_string(i);
                }
                call += ")";

                if (is_void_value_type(method.result_type)) {
                    line("  " + call);
                    line("  ret void");
                } else {
                    std::string out = temp();
                    line("  " + out + " = " + call);
                    line("  ret " + llvm_type(method.result_type) + " " + out);
                }
                line("}");
                line();
            }

            if (!table.drop_thunk_name.empty()) {
                line("define private void " + quote_global(table.drop_thunk_name) + "(ptr %arg0) {");
                line("entry:");
                if (!table.drop_impl_name.empty()) {
                    std::string receiver = temp();
                    line("  " + receiver + " = load " + llvm_type(table.drop_receiver_type) + ", ptr %arg0");
                    line("  call void " + quote_global(function_symbols_.at(table.drop_impl_name)) +
                         "(" + llvm_type(table.drop_receiver_type) + " " + receiver + ")");
                }
                line("  ret void");
                line("}");
                line();
            }
        }
        if (!program_.trait_object_vtables.empty()) trait_object_globals_ << "\n";
    }

    void collect_locals(const std::vector<IrStmtPtr>& statements, std::vector<std::pair<std::string, IrType>>& locals) {
        for (const auto& stmt : statements) collect_locals(*stmt, locals);
    }

    void collect_expr_locals(const IrExprPtr& expr, std::vector<std::pair<std::string, IrType>>& locals) {
        if (!expr) return;
        collect_expr_locals(*expr, locals);
    }

    void collect_expr_locals(const IrExpr& expr, std::vector<std::pair<std::string, IrType>>& locals) {
        collect_expr_locals(ir_expr_operand(expr), locals);
        collect_expr_locals(ir_expr_payload(expr), locals);
        collect_expr_locals(ir_expr_left(expr), locals);
        collect_expr_locals(ir_expr_right(expr), locals);
        collect_expr_locals(ir_expr_if_condition(expr), locals);
        collect_locals(ir_expr_if_then_body(expr), locals);
        collect_expr_locals(ir_expr_if_then_value(expr), locals);
        collect_locals(ir_expr_if_else_body(expr), locals);
        collect_expr_locals(ir_expr_if_else_value(expr), locals);
        collect_locals(ir_expr_block_body(expr), locals);
        collect_expr_locals(ir_expr_block_value(expr), locals);
        collect_locals(ir_expr_try_residual_cleanup(expr), locals);
        collect_expr_locals(ir_expr_match_value(expr), locals);
        for (const auto& arg : expr.args) collect_expr_locals(arg, locals);
        for (const auto& arm : ir_expr_match_arms(expr)) {
            if (arm.has_value_binding) locals.push_back({arm.value_name, arm.value_type});
            collect_payload_binding_locals(arm, locals);
            collect_locals(arm.body, locals);
            collect_expr_locals(arm.value, locals);
        }
    }

    template <typename Arm>
    void collect_payload_binding_locals(const Arm& arm, std::vector<std::pair<std::string, IrType>>& locals) {
        if (!arm.payload_bindings.empty()) {
            for (const auto& binding : arm.payload_bindings) locals.push_back({binding.name, binding.type});
            return;
        }
        if (arm.has_payload_binding) locals.push_back({arm.payload_name, arm.payload_type});
    }

    void collect_locals(const IrStmt& stmt, std::vector<std::pair<std::string, IrType>>& locals) {
        switch (stmt.kind) {
            case IrStmtKind::Block:
                collect_locals(ir_stmt_statements(stmt), locals);
                break;
            case IrStmtKind::VarDecl:
                locals.push_back({stmt.binding.name, stmt.binding.type});
                collect_expr_locals(stmt.binding.init, locals);
                break;
            case IrStmtKind::If:
                collect_expr_locals(stmt.condition, locals);
                collect_locals(ir_stmt_then_body(stmt), locals);
                collect_locals(ir_stmt_else_body(stmt), locals);
                break;
            case IrStmtKind::While:
            case IrStmtKind::InitWhile:
            case IrStmtKind::WhileLet:
                for (const auto& binding : stmt.init_bindings) locals.push_back({binding.name, binding.type});
                for (const auto& binding : stmt.init_bindings) collect_expr_locals(binding.init, locals);
                collect_expr_locals(stmt.condition, locals);
                if (stmt.kind == IrStmtKind::WhileLet) {
                    for (const auto& arm : ir_stmt_match_arms(stmt)) {
                        if (arm.has_value_binding) locals.push_back({arm.value_name, arm.value_type});
                        collect_payload_binding_locals(arm, locals);
                        collect_locals(arm.body, locals);
                    }
                }
                collect_locals(ir_stmt_loop_body(stmt), locals);
                for (const auto& update : stmt.updates) collect_expr_locals(update, locals);
                break;
            case IrStmtKind::ForRange:
                locals.push_back({ir_stmt_for_index_name(stmt), ir_stmt_for_start(stmt)->type});
                locals.push_back({ir_stmt_for_end_name(stmt), ir_stmt_for_end(stmt)->type});
                collect_expr_locals(ir_stmt_for_start(stmt), locals);
                collect_expr_locals(ir_stmt_for_end(stmt), locals);
                if (!ir_stmt_for_binding_name(stmt).empty()) {
                    locals.push_back({ir_stmt_for_binding_name(stmt), ir_stmt_for_binding_type(stmt)});
                }
                collect_locals(ir_stmt_loop_body(stmt), locals);
                break;
            case IrStmtKind::ForVector:
                if (!ir_stmt_for_binding_name(stmt).empty()) {
                    locals.push_back({ir_stmt_for_binding_name(stmt), ir_stmt_for_binding_type(stmt)});
                }
                for (const auto& value : ir_stmt_for_values(stmt)) collect_expr_locals(value, locals);
                collect_locals(ir_stmt_loop_body(stmt), locals);
                break;
            case IrStmtKind::Match:
                collect_expr_locals(stmt.match_value, locals);
                for (const auto& arm : ir_stmt_match_arms(stmt)) {
                    if (arm.has_value_binding) locals.push_back({arm.value_name, arm.value_type});
                    collect_payload_binding_locals(arm, locals);
                    collect_locals(arm.body, locals);
                }
                break;
            case IrStmtKind::Assign:
                collect_expr_locals(ir_stmt_assign_rhs(stmt), locals);
                break;
            case IrStmtKind::ExprStmt:
            case IrStmtKind::Return:
                collect_expr_locals(stmt.expr, locals);
                break;
            case IrStmtKind::Continue:
                for (const auto& update : stmt.updates) collect_expr_locals(update, locals);
                break;
            case IrStmtKind::Break:
                collect_expr_locals(ir_stmt_break_value(stmt), locals);
                break;
            default:
                break;
        }
    }

    std::string local_slot(SourceLocation loc, const std::string& name) const {
        auto found = locals_.find(name);
        if (found == locals_.end()) throw CompileError(where(loc) + ": unknown LLVM local '" + name + "'");
        return found->second;
    }

    void emit_function(const IrFunction& fn) {
        out_ = &functions_;
        locals_.clear();
        loops_.clear();
        temp_counter_ = 0;
        label_counter_ = 0;
        block_terminated_ = false;
        current_label_.clear();
        current_return_ = fn.return_type;

        std::string visibility;
        if (options_.shared_library && !fn.shared_export) visibility = "hidden ";
        line("define " + visibility + llvm_type(fn.return_type) + " " + quote_global(function_symbols_.at(fn.name)) + "(" + param_decl(fn.params) + ") {");
        emit_label("entry");

        std::vector<std::pair<std::string, IrType>> local_types;
        for (const auto& param : fn.params) local_types.push_back({param.name, param.type});
        collect_locals(fn.body, local_types);
        for (const auto& item : local_types) {
            if (locals_.count(item.first)) continue;
            std::string slot = "%" + sanitize_local(item.first) + ".addr." + std::to_string(locals_.size());
            locals_[item.first] = slot;
            line("  " + slot + " = alloca " + llvm_type(item.second));
        }
        for (std::size_t i = 0; i < fn.params.size(); ++i) {
            line("  store " + llvm_type(fn.params[i].type) + " %arg" + std::to_string(i) + ", ptr " + local_slot(fn.loc, fn.params[i].name));
        }

        emit_statements(fn.body);
        if (!block_terminated_) {
            if (is_void_value_type(fn.return_type)) line("  ret void");
            else line("  ret " + llvm_type(fn.return_type) + " " + default_value(fn.return_type));
        }
        line("}");
        line();
    }

    void emit_ari_main_bridge() {
        out_ = &functions_;
        line("define i64 " + quote_global("ari::main") + "() {");
        line("entry:");
        line("  %code64 = call i64 " + quote_global(function_symbols_.at("main")) + "()");
        line("  ret i64 %code64");
        line("}");
        line();
    }

    void emit_main_wrapper() {
        emit_ari_main_bridge();
        line("define i32 @main(i32 %argc, ptr %argv) {");
        line("entry:");
        line("  %code64 = call i64 @ari_entry(i32 %argc, ptr %argv)");
        line("  %code32 = trunc i64 %code64 to i32");
        line("  ret i32 %code32");
        line("}");
        line();

        line("define i64 @ari_entry(i32 %argc, ptr %argv) {");
        line("entry:");
        line("  call void @ari_context_init(i32 %argc, ptr %argv)");
        line("  %code64 = call i64 " + quote_global("ari::main") + "()");
        line("  call void @ari_context_shutdown()");
        line("  ret i64 %code64");
        line("}");
        line();
    }

    std::string param_decl(const std::vector<IrParam>& params) const {
        std::string text;
        for (std::size_t i = 0; i < params.size(); ++i) {
            if (i > 0) text += ", ";
            text += llvm_type(params[i].type) + " %arg" + std::to_string(i);
        }
        return text;
    }

    std::string param_type_decl(const std::vector<IrType>& params) const {
        std::string text;
        for (std::size_t i = 0; i < params.size(); ++i) {
            if (i > 0) text += ", ";
            text += llvm_type(params[i]) + " %arg" + std::to_string(i);
        }
        return text;
    }

    void emit_statements(const std::vector<IrStmtPtr>& statements) {
        for (const auto& stmt : statements) {
            if (block_terminated_) return;
            emit_statement(*stmt);
        }
    }

    void emit_statement(const IrStmt& stmt) {
        switch (stmt.kind) {
            case IrStmtKind::Block:
                emit_block(stmt);
                break;
            case IrStmtKind::VarDecl: {
                Value value = emit_expr(*stmt.binding.init);
                value = cast_value(value, stmt.binding.type);
                line("  store " + value.type + " " + value.name + ", ptr " + local_slot(stmt.loc, stmt.binding.name));
                break;
            }
            case IrStmtKind::Assign: {
                const IrExprPtr& rhs = ir_stmt_assign_rhs(stmt);
                const IrExprPtr& assign_target = ir_stmt_assign_target(stmt);
                Value value = emit_expr(*rhs);
                if (assign_target) {
                    std::string slot = emit_lvalue_ptr(*assign_target);
                    line("  store " + value.type + " " + value.name + ", ptr " + slot);
                } else {
                    line("  store " + value.type + " " + value.name + ", ptr " + local_slot(stmt.loc, ir_stmt_assign_name(stmt)));
                }
                break;
            }
            case IrStmtKind::ExprStmt:
                emit_expr(*stmt.expr);
                break;
            case IrStmtKind::Return:
                if (stmt.expr) {
                    Value value = cast_value(emit_expr(*stmt.expr), current_return_);
                    line("  ret " + value.type + " " + value.name);
                } else {
                    line("  ret void");
                }
                block_terminated_ = true;
                break;
            case IrStmtKind::If:
                emit_if(stmt);
                break;
            case IrStmtKind::While:
                emit_while(stmt);
                break;
            case IrStmtKind::WhileLet:
                emit_while_let(stmt);
                break;
            case IrStmtKind::ForRange:
                emit_for_range(stmt);
                break;
            case IrStmtKind::ForVector:
                emit_for_vector(stmt);
                break;
            case IrStmtKind::InitWhile:
                emit_init_while(stmt);
                break;
            case IrStmtKind::Continue:
                emit_continue(stmt);
                break;
            case IrStmtKind::Break:
                {
                    LoopContext& target = loop_for_break(stmt);
                    const IrExprPtr& break_value = ir_stmt_break_value(stmt);
                    if (break_value) {
                        if (!target.supports_break_value) {
                            throw CompileError(where(stmt.loc) + ": break value used with a non-value break target during LLVM lowering");
                        }
                        Value value = cast_value(emit_expr(*break_value), target.break_value_type);
                        line("  store " + value.type + " " + value.name + ", ptr " + target.break_value_slot);
                    }
                    target.has_break = true;
                    line("  br label %" + target.break_label);
                }
                block_terminated_ = true;
                break;
            case IrStmtKind::Match:
                emit_match(stmt);
                break;
            case IrStmtKind::Drop:
                if (stmt.expr) emit_expr(*stmt.expr);
                break;
        }
    }

    void emit_if(const IrStmt& stmt) {
        Value cond = cast_value(emit_expr(*stmt.condition), stmt.condition->type);
        std::string then_label = label("if.then");
        std::string else_label = label("if.else");
        std::string end_label = label("if.end");
        line("  br i1 " + cond.name + ", label %" + then_label + ", label %" + else_label);

        emit_label(then_label);
        emit_statements(ir_stmt_then_body(stmt));
        bool then_term = block_terminated_;
        if (!then_term) line("  br label %" + end_label);

        emit_label(else_label);
        emit_statements(ir_stmt_else_body(stmt));
        bool else_term = block_terminated_;
        if (!else_term) line("  br label %" + end_label);

        if (then_term && else_term) {
            block_terminated_ = true;
            return;
        }
        emit_label(end_label);
    }

    void emit_block(const IrStmt& stmt) {
        const std::string& source_label = ir_stmt_label(stmt);
        if (source_label.empty()) {
            emit_statements(ir_stmt_statements(stmt));
            return;
        }

        std::string end_label = label("block.end");
        loops_.push_back(make_loop_context(end_label, "", "", source_label, {}, false));
        emit_statements(ir_stmt_statements(stmt));

        bool body_terminated = block_terminated_;
        bool has_break = loops_.back().has_break;
        loops_.pop_back();

        if (body_terminated && !has_break) return;
        if (!body_terminated) line("  br label %" + end_label);
        emit_label(end_label);
    }

    void emit_while(const IrStmt& stmt) {
        std::string cond_label = label("while.cond");
        std::string body_label = label("while.body");
        std::string end_label = label("while.end");
        line("  br label %" + cond_label);
        emit_label(cond_label);
        Value cond = emit_expr(*stmt.condition);
        line("  br i1 " + cond.name + ", label %" + body_label + ", label %" + end_label);
        loops_.push_back(make_loop_context(end_label, cond_label, cond_label, ir_stmt_label(stmt)));
        emit_label(body_label);
        emit_statements(ir_stmt_loop_body(stmt));
        if (!block_terminated_) line("  br label %" + cond_label);
        loops_.pop_back();
        emit_label(end_label);
    }

    void emit_while_let(const IrStmt& stmt) {
        if (ir_stmt_match_arms(stmt).empty()) throw CompileError(where(stmt.loc) + ": while-let missing lowered pattern");
        std::string cond_label = label("whilelet.cond");
        std::string body_label = label("whilelet.body");
        std::string end_label = label("whilelet.end");
        std::vector<std::string> arm_labels;
        arm_labels.reserve(ir_stmt_match_arms(stmt).size());
        for (std::size_t i = 0; i < ir_stmt_match_arms(stmt).size(); ++i) {
            arm_labels.push_back(label("whilelet.arm"));
        }

        line("  br label %" + cond_label);
        emit_label(cond_label);
        for (const auto& binding : stmt.init_bindings) {
            Value init = emit_expr(*binding.init);
            line("  store " + init.type + " " + init.name + ", ptr " + local_slot(binding.loc, binding.name));
        }
        Value value = emit_expr(*stmt.match_value);
        std::string subject = value.name;
        std::string subject_type = value.type;
        if (value.ir_type.primitive == IrPrimitiveKind::Enum) {
            Value tag = emit_enum_tag_value(stmt.loc, value);
            subject = tag.name;
            subject_type = tag.type;
        }

        for (std::size_t i = 0; i < ir_stmt_match_arms(stmt).size(); ++i) {
            const IrMatchArm& arm = ir_stmt_match_arms(stmt)[i];
            std::string next = (i + 1 == ir_stmt_match_arms(stmt).size()) ? end_label : label("whilelet.test");
            std::string cmp = emit_match_condition(arm, value, subject, subject_type);
            line("  br i1 " + cmp + ", label %" + arm_labels[i] + ", label %" + next);
            if (next != end_label) emit_label(next);
        }

        for (std::size_t i = 0; i < ir_stmt_match_arms(stmt).size(); ++i) {
            const IrMatchArm& arm = ir_stmt_match_arms(stmt)[i];
            emit_label(arm_labels[i]);
            if (arm.has_value_binding) {
                Value bound = cast_value(value, arm.value_type);
                line("  store " + bound.type + " " + bound.name + ", ptr " + local_slot(arm.loc, arm.value_name));
            }
            emit_payload_bindings(arm, value);
            emit_statements(arm.body);
            if (!block_terminated_) {
                if (stmt.while_let_continue_on_mismatch && i + 1 == ir_stmt_match_arms(stmt).size()) {
                    line("  br label %" + cond_label);
                } else {
                    line("  br label %" + body_label);
                }
            }
        }

        loops_.push_back(make_loop_context(end_label, cond_label, cond_label, ir_stmt_label(stmt)));
        emit_label(body_label);
        emit_statements(ir_stmt_loop_body(stmt));
        if (!block_terminated_) line("  br label %" + cond_label);
        loops_.pop_back();
        emit_label(end_label);
    }

    void emit_for_range(const IrStmt& stmt) {
        const IrStmtForPayload& for_loop = ir_stmt_for_payload(stmt);
        Value start = emit_expr(*for_loop.start);
        Value end = emit_expr(*for_loop.end);
        line("  store " + start.type + " " + start.name + ", ptr " + local_slot(stmt.loc, for_loop.index_name));
        line("  store " + end.type + " " + end.name + ", ptr " + local_slot(stmt.loc, for_loop.end_name));

        std::string cond_label = label("for.cond");
        std::string body_label = label("for.body");
        std::string step_label = label("for.step");
        std::string end_label = label("for.end");
        line("  br label %" + cond_label);
        emit_label(cond_label);
        Value index = load_local(stmt.loc, for_loop.index_name, for_loop.start->type);
        Value limit = load_local(stmt.loc, for_loop.end_name, for_loop.end->type);
        std::string cmp = temp();
        bool unsigned_range = is_unsigned_integer_type(for_loop.start->type);
        std::string op = for_loop.inclusive
            ? (unsigned_range ? "ule " : "sle ")
            : (unsigned_range ? "ult " : "slt ");
        line("  " + cmp + " = icmp " + op +
             index.type + " " + index.name + ", " + limit.name);
        line("  br i1 " + cmp + ", label %" + body_label + ", label %" + end_label);

        loops_.push_back(make_loop_context(end_label, step_label, cond_label, ir_stmt_label(stmt)));
        emit_label(body_label);
        if (!for_loop.binding_name.empty()) {
            Value current = load_local(stmt.loc, for_loop.index_name, for_loop.binding_type);
            line("  store " + current.type + " " + current.name + ", ptr " + local_slot(stmt.loc, for_loop.binding_name));
        }
        emit_statements(ir_stmt_loop_body(stmt));
        if (!block_terminated_) line("  br label %" + step_label);

        emit_label(step_label);
        Value old = load_local(stmt.loc, for_loop.index_name, for_loop.start->type);
        if (for_loop.inclusive) {
            Value step_limit = load_local(stmt.loc, for_loop.end_name, for_loop.end->type);
            std::string done = temp();
            std::string inc_label = label("for.inc");
            line("  " + done + " = icmp eq " + old.type + " " + old.name + ", " + step_limit.name);
            line("  br i1 " + done + ", label %" + end_label + ", label %" + inc_label);
            emit_label(inc_label);
        }
        std::string next = temp();
        line("  " + next + " = add " + old.type + " " + old.name + ", 1");
        line("  store " + old.type + " " + next + ", ptr " + local_slot(stmt.loc, for_loop.index_name));
        line("  br label %" + cond_label);
        loops_.pop_back();
        emit_label(end_label);
    }

    void emit_for_vector(const IrStmt& stmt) {
        std::string end_label = label("forvec.end");
        loops_.push_back(make_loop_context(end_label, "", "", ir_stmt_label(stmt)));
        const IrStmtForPayload& for_loop = ir_stmt_for_payload(stmt);
        for (std::size_t i = 0; i < for_loop.values.size(); ++i) {
            std::string next_label = i + 1 == for_loop.values.size() ? end_label : label("forvec.next");
            loops_.back().plain_continue_label = next_label;
            loops_.back().value_continue_label = next_label;
            Value value = emit_expr(*for_loop.values[i]);
            if (!for_loop.binding_name.empty()) {
                line("  store " + value.type + " " + value.name + ", ptr " + local_slot(stmt.loc, for_loop.binding_name));
            }
            emit_statements(ir_stmt_loop_body(stmt));
            if (!block_terminated_) line("  br label %" + next_label);
            if (next_label != end_label) emit_label(next_label);
        }
        loops_.pop_back();
        emit_label(end_label);
    }

    void emit_init_while(const IrStmt& stmt) {
        std::vector<std::string> names;
        for (const auto& binding : stmt.init_bindings) {
            names.push_back(binding.name);
            Value value = emit_expr(*binding.init);
            line("  store " + value.type + " " + value.name + ", ptr " + local_slot(binding.loc, binding.name));
        }

        std::string cond_label = label("initwhile.cond");
        std::string body_label = label("initwhile.body");
        std::string update_label = label("initwhile.update");
        std::string end_label = label("initwhile.end");
        line("  br label %" + cond_label);
        emit_label(cond_label);
        Value cond = emit_expr(*stmt.condition);
        line("  br i1 " + cond.name + ", label %" + body_label + ", label %" + end_label);

        loops_.push_back(make_loop_context(end_label, update_label, cond_label, ir_stmt_label(stmt), names));
        emit_label(body_label);
        emit_statements(ir_stmt_loop_body(stmt));
        if (!block_terminated_) line("  br label %" + update_label);

        emit_label(update_label);
        emit_parallel_update(stmt.loc, names, stmt.updates);
        line("  br label %" + cond_label);
        loops_.pop_back();
        emit_label(end_label);
    }

    void emit_parallel_update(SourceLocation loc, const std::vector<std::string>& names, const std::vector<IrExprPtr>& values) {
        if (names.size() != values.size()) throw CompileError(where(loc) + ": update count mismatch during LLVM lowering");
        std::vector<Value> temps;
        for (const auto& value : values) temps.push_back(emit_expr(*value));
        for (std::size_t i = 0; i < names.size(); ++i) {
            line("  store " + temps[i].type + " " + temps[i].name + ", ptr " + local_slot(loc, names[i]));
        }
    }

    void emit_continue(const IrStmt& stmt) {
        const LoopContext& loop = loop_for_continue(stmt.loc);
        if (!stmt.updates.empty()) {
            emit_parallel_update(stmt.loc, loop.update_names, stmt.updates);
            line("  br label %" + loop.value_continue_label);
        } else {
            line("  br label %" + loop.plain_continue_label);
        }
        block_terminated_ = true;
    }

    const LoopContext& loop_for_continue(SourceLocation loc) const {
        for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
            if (loop->is_loop) return *loop;
        }
        throw CompileError(where(loc) + ": continue outside loop during LLVM lowering");
    }

    LoopContext& loop_for_break(const IrStmt& stmt) {
        const std::string& break_label = ir_stmt_break_label(stmt);
        if (break_label.empty()) {
            for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
                if (loop->is_loop) return *loop;
            }
            throw CompileError(where(stmt.loc) + ": break outside loop during LLVM lowering");
        }
        for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
            if (loop->source_label == break_label) return *loop;
        }
        throw CompileError(where(stmt.loc) + ": unknown loop label '" + break_label + "' during LLVM lowering");
    }

    void emit_match(const IrStmt& stmt) {
        Value value = emit_expr(*stmt.match_value);
        std::string subject = value.name;
        std::string subject_type = value.type;
        if (value.ir_type.primitive == IrPrimitiveKind::Enum) {
            Value tag = emit_enum_tag_value(stmt.loc, value);
            subject = tag.name;
            subject_type = tag.type;
        }
        std::string end_label = label("match.end");
        std::vector<std::string> arm_labels;
        for (std::size_t i = 0; i < ir_stmt_match_arms(stmt).size(); ++i) arm_labels.push_back(label("match.arm"));

        std::string first_test = label("match.test");
        line("  br label %" + first_test);
        emit_label(first_test);
        for (std::size_t i = 0; i < ir_stmt_match_arms(stmt).size(); ++i) {
            const IrMatchArm& arm = ir_stmt_match_arms(stmt)[i];
            if (arm.wildcard) {
                line("  br label %" + arm_labels[i]);
            } else {
                std::string next = (i + 1 == ir_stmt_match_arms(stmt).size()) ? arm_labels[i] : label("match.test");
                std::string cmp = emit_match_condition(arm, value, subject, subject_type);
                line("  br i1 " + cmp + ", label %" + arm_labels[i] + ", label %" + next);
                if (next != arm_labels[i]) emit_label(next);
            }
        }

        for (std::size_t i = 0; i < ir_stmt_match_arms(stmt).size(); ++i) {
            const IrMatchArm& arm = ir_stmt_match_arms(stmt)[i];
            emit_label(arm_labels[i]);
            if (arm.has_value_binding) {
                Value bound = cast_value(value, arm.value_type);
                line("  store " + bound.type + " " + bound.name + ", ptr " + local_slot(arm.loc, arm.value_name));
            }
            emit_payload_bindings(arm, value);
            emit_statements(arm.body);
            if (!block_terminated_) line("  br label %" + end_label);
        }
        emit_label(end_label);
    }

    Value emit_expr(const IrExpr& expr) {
        switch (expr.kind) {
            case IrExprKind::Integer: {
                std::string value = expr.int_negative ? "-" + std::to_string(expr.int_value) : std::to_string(expr.int_value);
                return Value{llvm_type(expr.type), value, expr.type};
            }
            case IrExprKind::Bool:
                return Value{"i1", expr.bool_value ? "1" : "0", expr.type};
            case IrExprKind::Float:
                if (expr.type.primitive == IrPrimitiveKind::F128) {
                    return Value{llvm_type(expr.type), fp128_literal_from_double(expr.float_value), expr.type};
                }
                return Value{llvm_type(expr.type), float_literal(expr.float_value), expr.type};
            case IrExprKind::String:
                return Value{"ptr", string_ptr(ir_expr_string_value(expr)), expr.type};
            case IrExprKind::Null:
                return Value{"ptr", "null", expr.type};
            case IrExprKind::FunctionRef:
                return emit_function_ref(expr);
            case IrExprKind::Local:
                return load_local(expr.loc, ir_expr_name(expr), expr.type);
            case IrExprKind::Borrow:
                if (ir_expr_operand(expr)) return Value{"ptr", emit_lvalue_ptr(*ir_expr_operand(expr)), expr.type};
                return Value{"ptr", local_slot(expr.loc, ir_expr_name(expr)), expr.type};
            case IrExprKind::Unary: {
                Value operand = emit_expr(*ir_expr_operand(expr));
                std::string out = temp();
                switch (expr.unary_op) {
                    case IrUnaryOp::Not:
                        line("  " + out + " = xor i1 " + operand.name + ", true");
                        return Value{"i1", out, expr.type};
                    case IrUnaryOp::BitNot:
                        line("  " + out + " = xor " + operand.type + " " + operand.name + ", -1");
                        return Value{operand.type, out, expr.type};
                }
                throw CompileError(where(expr.loc) + ": unsupported LLVM unary expression");
            }
            case IrExprKind::Cast:
                if (expr.type.primitive == IrPrimitiveKind::TraitObject) return emit_trait_object_cast(expr);
                return cast_value(emit_expr(*ir_expr_operand(expr)), expr.type);
            case IrExprKind::PointerOffset:
                return emit_pointer_offset(expr);
            case IrExprKind::PointerAdd:
                return emit_pointer_add(expr);
            case IrExprKind::PointerLoad:
                return emit_pointer_load(expr);
            case IrExprKind::PointerStore:
                return emit_pointer_store(expr);
            case IrExprKind::Try:
                return emit_try(expr);
            case IrExprKind::NullCoalesce:
                return emit_null_coalesce(expr);
            case IrExprKind::EnumTag:
                return emit_enum_tag_expr(expr);
            case IrExprKind::EnumConstruct:
                return emit_enum_construct(expr);
            case IrExprKind::Tuple:
                return emit_tuple(expr);
            case IrExprKind::TupleIndex:
                return emit_tuple_index(expr);
            case IrExprKind::Index:
                if (ir_expr_operand(expr) && ir_expr_operand(expr)->type.primitive == IrPrimitiveKind::Array) {
                    return emit_array_index(expr);
                }
                if (ir_expr_operand(expr) && ir_expr_operand(expr)->type.primitive == IrPrimitiveKind::Vector) {
                    return emit_vector_index(expr);
                }
                if (ir_expr_operand(expr) && is_prelude_slice_type(ir_expr_operand(expr)->type)) {
                    return emit_slice_index(expr);
                }
                throw CompileError(where(expr.loc) + ": LLVM backend can only lower array, vector, and Slice indexing yet");
            case IrExprKind::SliceRange:
                return emit_slice_range(expr);
            case IrExprKind::Vector:
                if (expr.type.primitive == IrPrimitiveKind::Array) return emit_tuple(expr);
                if (expr.type.primitive == IrPrimitiveKind::Vector) return emit_vector(expr);
                throw CompileError(where(expr.loc) + ": LLVM backend cannot lower vector value type " + type_name(expr.type));
            case IrExprKind::VectorPush:
                return emit_vector_push(expr);
            case IrExprKind::VectorPop:
                return emit_vector_pop(expr);
            case IrExprKind::VectorReserve:
                return emit_vector_reserve(expr);
            case IrExprKind::VectorClear:
                return emit_vector_clear(expr);
            case IrExprKind::VectorTruncate:
                return emit_vector_truncate(expr);
            case IrExprKind::VectorSet:
                return emit_vector_set(expr);
            case IrExprKind::VectorSwap:
                return emit_vector_swap(expr);
            case IrExprKind::VectorRemove:
                return emit_vector_remove(expr);
            case IrExprKind::VectorInsert:
                return emit_vector_insert(expr);
            case IrExprKind::VectorContains:
                return emit_vector_search(expr, false);
            case IrExprKind::VectorIndexOf:
                return emit_vector_search(expr, true);
            case IrExprKind::VectorCount:
                return emit_vector_count(expr);
            case IrExprKind::Noop:
                return Value{"void", "", expr.type};
            case IrExprKind::FormatPrint:
                return emit_format_print(expr);
            case IrExprKind::Match:
                return emit_match_expr(expr);
            case IrExprKind::If:
                return emit_if_expr(expr);
            case IrExprKind::Block:
                return emit_block_expr(expr);
            case IrExprKind::IndirectCall:
                return emit_indirect_call(expr);
            case IrExprKind::TraitObjectCall:
                return emit_trait_object_call(expr);
            case IrExprKind::TraitObjectDrop:
                return emit_trait_object_drop(expr);
            case IrExprKind::Call:
                return emit_call(expr);
            case IrExprKind::Binary:
                return emit_binary(expr);
        }
        throw CompileError(where(expr.loc) + ": unsupported LLVM expression");
    }

    Value load_local(SourceLocation loc, const std::string& name, const IrType& type) {
        std::string out = temp();
        std::string lltype = llvm_type(type);
        line("  " + out + " = load " + lltype + ", ptr " + local_slot(loc, name));
        return Value{lltype, out, type};
    }

    static bool is_reference_like_lvalue_type(const IrType& type) {
        return type.qualifier == TypeQualifier::Ref ||
               type.qualifier == TypeQualifier::MutRef ||
               type.qualifier == TypeQualifier::Ptr;
    }

    static IrType lvalue_pointee_type(IrType type) {
        type.qualifier = TypeQualifier::Value;
        return type;
    }

    const IrType& gep_base_type(const IrExpr& expr, IrType& scratch) const {
        if (is_reference_like_lvalue_type(expr.type)) {
            scratch = lvalue_pointee_type(expr.type);
            return scratch;
        }
        return expr.type;
    }

    std::string emit_lvalue_ptr(const IrExpr& expr) {
        if (expr.kind == IrExprKind::Local) {
            std::string slot = local_slot(expr.loc, ir_expr_name(expr));
            if (!is_reference_like_lvalue_type(expr.type)) return slot;
            std::string out = temp();
            line("  " + out + " = load ptr, ptr " + slot);
            return out;
        }
        if (expr.kind == IrExprKind::PointerLoad && ir_expr_operand(expr)) {
            return emit_expr(*ir_expr_operand(expr)).name;
        }
        if (expr.kind == IrExprKind::TupleIndex && ir_expr_operand(expr)) {
            std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
            std::string ptr = temp();
            IrType base_type;
            line("  " + ptr + " = getelementptr inbounds " + llvm_type(gep_base_type(*ir_expr_operand(expr), base_type)) +
                 ", ptr " + base + ", i32 0, i32 " + std::to_string(expr.tuple_index));
            return ptr;
        }
        if (expr.kind == IrExprKind::Index && ir_expr_operand(expr) && ir_expr_right(expr)) {
            if (is_prelude_slice_type(ir_expr_operand(expr)->type)) {
                return emit_slice_element_ptr(expr);
            }
            if (ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Array &&
                ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector) {
                throw CompileError(where(expr.loc) + ": LLVM backend can only index array, vector, or Slice lvalues");
            }
            std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
            IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
            Value index = cast_value(emit_expr(*ir_expr_right(expr)), index_type);
            std::string ptr = temp();
            IrType base_type;
            const IrType& storage_type = gep_base_type(*ir_expr_operand(expr), base_type);
            if (ir_expr_operand(expr)->type.primitive == IrPrimitiveKind::Array) {
                emit_array_bounds_check(index, ir_expr_operand(expr)->type.array_size);
                line("  " + ptr + " = getelementptr inbounds " + llvm_type(storage_type) +
                     ", ptr " + base + ", i64 0, i64 " + index.name);
            } else {
                emit_vector_bounds_check(index, storage_type, base);
                line("  " + ptr + " = getelementptr inbounds " + llvm_type(storage_type) +
                     ", ptr " + base + ", i32 0, i32 1, i64 " + index.name);
            }
            return ptr;
        }
        throw CompileError(where(expr.loc) + ": LLVM backend can only assign to local aggregate fields yet");
    }

    static bool is_pointer_backed_lvalue(const IrExpr& expr) {
        if (expr.kind == IrExprKind::PointerLoad) return true;
        if (expr.kind == IrExprKind::Local) {
            return expr.type.qualifier == TypeQualifier::Ref ||
                   expr.type.qualifier == TypeQualifier::MutRef ||
                   expr.type.qualifier == TypeQualifier::Ptr;
        }
        if ((expr.kind == IrExprKind::TupleIndex ||
             expr.kind == IrExprKind::Index) &&
            ir_expr_operand(expr)) {
            return is_pointer_backed_lvalue(*ir_expr_operand(expr));
        }
        return false;
    }

    Value emit_tuple(const IrExpr& expr) {
        std::string tuple_type = llvm_type(expr.type);
        if (expr.args.empty()) return Value{tuple_type, "zeroinitializer", expr.type};
        std::string current = "undef";
        const std::vector<IrType>& fields = (expr.type.primitive == IrPrimitiveKind::Struct ||
                                             expr.type.primitive == IrPrimitiveKind::Array)
            ? expr.type.field_types
            : expr.type.args;
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            Value item = cast_value(emit_expr(*expr.args[i]), fields[i]);
            std::string out = temp();
            line("  " + out + " = insertvalue " + tuple_type + " " + current +
                 ", " + item.type + " " + item.name + ", " + std::to_string(i));
            current = out;
        }
        return Value{tuple_type, current, expr.type};
    }

    Value emit_tuple_index(const IrExpr& expr) {
        if (is_pointer_backed_lvalue(expr)) {
            std::string ptr = emit_lvalue_ptr(expr);
            std::string out = temp();
            line("  " + out + " = load " + llvm_type(expr.type) + ", ptr " + ptr);
            return Value{llvm_type(expr.type), out, expr.type};
        }
        Value tuple = emit_expr(*ir_expr_operand(expr));
        const IrType& result = expr.type;
        std::string out = temp();
        line("  " + out + " = extractvalue " + tuple.type + " " + tuple.name +
             ", " + std::to_string(expr.tuple_index));
        return Value{llvm_type(result), out, result};
    }

    Value emit_array_index(const IrExpr& expr) {
        std::string ptr = emit_lvalue_ptr(expr);
        std::string out = temp();
        line("  " + out + " = load " + llvm_type(expr.type) + ", ptr " + ptr);
        return Value{llvm_type(expr.type), out, expr.type};
    }

    Value emit_vector(const IrExpr& expr) {
        if (expr.type.args.size() != 1 || expr.type.array_size < expr.args.size()) {
            throw CompileError(where(expr.loc) + ": malformed vector literal during LLVM lowering");
        }
        const IrType& element_type = expr.type.args[0];
        std::string vector_type = llvm_type(expr.type);
        std::string array_type = "[" + std::to_string(expr.type.array_size) + " x " + llvm_type(element_type) + "]";
        std::string array_value = "zeroinitializer";
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            Value item = cast_value(emit_expr(*expr.args[i]), element_type);
            std::string next = temp();
            line("  " + next + " = insertvalue " + array_type + " " + array_value +
                 ", " + item.type + " " + item.name + ", " + std::to_string(i));
            array_value = next;
        }
        std::string with_len = temp();
        line("  " + with_len + " = insertvalue " + vector_type + " undef, i64 " +
             std::to_string(expr.args.size()) + ", 0");
        std::string out = temp();
        line("  " + out + " = insertvalue " + vector_type + " " + with_len +
             ", " + array_type + " " + array_value + ", 1");
        return Value{vector_type, out, expr.type};
    }

    Value emit_vector_index(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || ir_expr_operand(expr)->kind != IrExprKind::Local) {
            throw CompileError(where(expr.loc) + ": LLVM backend can only dynamically index local vectors yet");
        }
        if (ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": LLVM backend cannot index unsized Vec storage");
        }
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*ir_expr_right(expr)), index_type);
        std::string base = local_slot(ir_expr_operand(expr)->loc, ir_expr_name(*ir_expr_operand(expr)));
        emit_vector_bounds_check(index, ir_expr_operand(expr)->type, base);
        std::string ptr = temp();
        line("  " + ptr + " = getelementptr inbounds " + llvm_type(ir_expr_operand(expr)->type) +
             ", ptr " + base +
             ", i64 0, i32 1, i64 " + index.name);
        std::string out = temp();
        line("  " + out + " = load " + llvm_type(expr.type) + ", ptr " + ptr);
        return Value{llvm_type(expr.type), out, expr.type};
    }

    Value emit_slice_index(const IrExpr& expr) {
        std::string ptr = emit_slice_element_ptr(expr);
        std::string out = temp();
        line("  " + out + " = load " + llvm_type(expr.type) + ", ptr " + ptr);
        return Value{llvm_type(expr.type), out, expr.type};
    }

    Value emit_slice_range(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || !ir_expr_left(expr) || !ir_expr_right(expr) || !is_prelude_slice_type(ir_expr_operand(expr)->type)) {
            throw CompileError(where(expr.loc) + ": malformed Slice range during LLVM lowering");
        }
        if (expr.type.args.empty()) {
            throw CompileError(where(expr.loc) + ": malformed Slice range result type");
        }

        Value slice = emit_expr(*ir_expr_operand(expr));
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value start = cast_value(emit_expr(*ir_expr_left(expr)), index_type);
        Value end = cast_value(emit_expr(*ir_expr_right(expr)), index_type);

        std::string data = temp();
        std::string len = temp();
        line("  " + data + " = extractvalue " + slice.type + " " + slice.name + ", 0");
        line("  " + len + " = extractvalue " + slice.type + " " + slice.name + ", 1");
        emit_slice_range_bounds_check(start, end, len, expr.bool_value);

        std::string range_len = temp();
        if (expr.bool_value) {
            std::string delta = temp();
            line("  " + delta + " = sub i64 " + end.name + ", " + start.name);
            line("  " + range_len + " = add i64 " + delta + ", 1");
        } else {
            line("  " + range_len + " = sub i64 " + end.name + ", " + start.name);
        }

        std::string range_data = temp();
        IrType element = expr.type.args[0];
        line("  " + range_data + " = getelementptr " + llvm_type(element) +
             ", ptr " + data + ", i64 " + start.name);

        std::string tuple_type = llvm_type(expr.type);
        std::string with_data = temp();
        line("  " + with_data + " = insertvalue " + tuple_type + " undef, ptr " + range_data + ", 0");
        std::string out = temp();
        line("  " + out + " = insertvalue " + tuple_type + " " + with_data +
             ", i64 " + range_len + ", 1");
        return Value{tuple_type, out, expr.type};
    }

    std::string emit_slice_element_ptr(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || !ir_expr_right(expr) || !is_prelude_slice_type(ir_expr_operand(expr)->type)) {
            throw CompileError(where(expr.loc) + ": malformed Slice index during LLVM lowering");
        }
        Value slice = emit_expr(*ir_expr_operand(expr));
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*ir_expr_right(expr)), index_type);
        std::string data = temp();
        std::string len = temp();
        line("  " + data + " = extractvalue " + slice.type + " " + slice.name + ", 0");
        line("  " + len + " = extractvalue " + slice.type + " " + slice.name + ", 1");
        emit_slice_bounds_check(index, len);
        std::string ptr = temp();
        line("  " + ptr + " = getelementptr inbounds " + llvm_type(expr.type) +
             ", ptr " + data + ", i64 " + index.name);
        return ptr;
    }

    Value emit_vector_push(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || !ir_expr_right(expr) || ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.push lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
        std::string len_ptr = temp();
        std::string len = temp();
        std::string full = temp();
        std::string fail_label = label("vector.push.full");
        std::string ok_label = label("vector.push.ok");

        line("  " + len_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 0");
        line("  " + len + " = load i64, ptr " + len_ptr);
        line("  " + full + " = icmp sge i64 " + len + ", " + std::to_string(vector_type.array_size));
        line("  br i1 " + full + ", label %" + fail_label + ", label %" + ok_label);
        emit_label(fail_label);
        line("  call void @ari_builtin_panic()");
        line("  unreachable");
        emit_label(ok_label);

        Value value = cast_value(emit_expr(*ir_expr_right(expr)), element_type);
        std::string item_ptr = temp();
        std::string next_len = temp();
        line("  " + item_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + len);
        line("  store " + value.type + " " + value.name + ", ptr " + item_ptr);
        line("  " + next_len + " = add i64 " + len + ", 1");
        line("  store i64 " + next_len + ", ptr " + len_ptr);
        return Value{"void", "", expr.type};
    }

    Value emit_vector_pop(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.pop lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
        std::string len_ptr = temp();
        std::string len = temp();
        std::string empty = temp();
        std::string fail_label = label("vector.pop.empty");
        std::string ok_label = label("vector.pop.ok");

        line("  " + len_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 0");
        line("  " + len + " = load i64, ptr " + len_ptr);
        line("  " + empty + " = icmp sle i64 " + len + ", 0");
        line("  br i1 " + empty + ", label %" + fail_label + ", label %" + ok_label);
        emit_label(fail_label);
        line("  call void @ari_builtin_panic()");
        line("  unreachable");
        emit_label(ok_label);

        std::string next_len = temp();
        std::string item_ptr = temp();
        std::string out = temp();
        line("  " + next_len + " = add i64 " + len + ", -1");
        line("  store i64 " + next_len + ", ptr " + len_ptr);
        line("  " + item_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + next_len);
        line("  " + out + " = load " + llvm_type(element_type) + ", ptr " + item_ptr);
        return Value{llvm_type(element_type), out, element_type};
    }

    Value emit_vector_reserve(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || !ir_expr_right(expr) || ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.reserve lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value requested = cast_value(emit_expr(*ir_expr_right(expr)), index_type);

        std::string negative = temp();
        std::string too_large = temp();
        std::string bad = temp();
        std::string fail_label = label("vector.reserve.fail");
        std::string ok_label = label("vector.reserve.ok");
        line("  " + negative + " = icmp slt i64 " + requested.name + ", 0");
        line("  " + too_large + " = icmp sgt i64 " + requested.name + ", " +
             std::to_string(vector_type.array_size));
        line("  " + bad + " = or i1 " + negative + ", " + too_large);
        line("  br i1 " + bad + ", label %" + fail_label + ", label %" + ok_label);
        emit_label(fail_label);
        line("  call void @ari_builtin_panic()");
        line("  unreachable");
        emit_label(ok_label);
        return Value{"void", "", expr.type};
    }

    Value emit_vector_clear(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.clear lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
        std::string len_ptr = temp();
        line("  " + len_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 0");
        line("  store i64 0, ptr " + len_ptr);
        return Value{"void", "", expr.type};
    }

    Value emit_vector_truncate(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || !ir_expr_right(expr) || ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.truncate lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value new_len = cast_value(emit_expr(*ir_expr_right(expr)), index_type);

        std::string negative = temp();
        std::string trap_label = label("vector.truncate.negative");
        std::string ok_label = label("vector.truncate.ok");
        line("  " + negative + " = icmp slt i64 " + new_len.name + ", 0");
        line("  br i1 " + negative + ", label %" + trap_label + ", label %" + ok_label);
        emit_label(trap_label);
        line("  call void @ari_builtin_panic()");
        line("  unreachable");
        emit_label(ok_label);

        std::string len_ptr = temp();
        std::string len = temp();
        std::string should_shrink = temp();
        std::string kept_len = temp();
        line("  " + len_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 0");
        line("  " + len + " = load i64, ptr " + len_ptr);
        line("  " + should_shrink + " = icmp slt i64 " + new_len.name + ", " + len);
        line("  " + kept_len + " = select i1 " + should_shrink +
             ", i64 " + new_len.name + ", i64 " + len);
        line("  store i64 " + kept_len + ", ptr " + len_ptr);
        return Value{"void", "", expr.type};
    }

    Value emit_vector_set(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || !ir_expr_right(expr) || !ir_expr_payload(expr) ||
            ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.set lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*ir_expr_right(expr)), index_type);
        emit_vector_bounds_check(index, vector_type, base);

        Value value = cast_value(emit_expr(*ir_expr_payload(expr)), element_type);
        std::string item_ptr = temp();
        line("  " + item_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + index.name);
        line("  store " + value.type + " " + value.name + ", ptr " + item_ptr);
        return Value{"void", "", expr.type};
    }

    Value emit_vector_swap(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || !ir_expr_right(expr) || !ir_expr_payload(expr) ||
            ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.swap lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value first_index = cast_value(emit_expr(*ir_expr_right(expr)), index_type);
        emit_vector_bounds_check(first_index, vector_type, base);
        Value second_index = cast_value(emit_expr(*ir_expr_payload(expr)), index_type);
        emit_vector_bounds_check(second_index, vector_type, base);

        std::string first_ptr = temp();
        std::string second_ptr = temp();
        std::string first_value = temp();
        std::string second_value = temp();
        std::string element_llvm = llvm_type(element_type);
        line("  " + first_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + first_index.name);
        line("  " + second_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + second_index.name);
        line("  " + first_value + " = load " + element_llvm + ", ptr " + first_ptr);
        line("  " + second_value + " = load " + element_llvm + ", ptr " + second_ptr);
        line("  store " + element_llvm + " " + second_value + ", ptr " + first_ptr);
        line("  store " + element_llvm + " " + first_value + ", ptr " + second_ptr);
        return Value{"void", "", expr.type};
    }

    Value emit_vector_remove(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || !ir_expr_right(expr) ||
            ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.remove lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*ir_expr_right(expr)), index_type);
        emit_vector_bounds_check(index, vector_type, base);

        std::string element_llvm = llvm_type(element_type);
        std::string len_ptr = temp();
        std::string len = temp();
        std::string next_len = temp();
        std::string removed_ptr = temp();
        std::string removed = temp();
        line("  " + len_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 0");
        line("  " + len + " = load i64, ptr " + len_ptr);
        line("  " + next_len + " = add i64 " + len + ", -1");
        line("  " + removed_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + index.name);
        line("  " + removed + " = load " + element_llvm + ", ptr " + removed_ptr);

        std::string entry_label = current_label_;
        std::string cond_label = label("vector.remove.cond");
        std::string body_label = label("vector.remove.body");
        std::string end_label = label("vector.remove.end");
        std::string cursor = temp();
        std::string should_shift = temp();
        std::string src_index = temp();
        std::string dst_ptr = temp();
        std::string src_ptr = temp();
        std::string moved = temp();
        std::string next_cursor = temp();

        line("  br label %" + cond_label);
        emit_label(cond_label);
        line("  " + cursor + " = phi i64 [ " + index.name + ", %" + entry_label +
             " ], [ " + next_cursor + ", %" + body_label + " ]");
        line("  " + should_shift + " = icmp slt i64 " + cursor + ", " + next_len);
        line("  br i1 " + should_shift + ", label %" + body_label + ", label %" + end_label);

        emit_label(body_label);
        line("  " + src_index + " = add i64 " + cursor + ", 1");
        line("  " + dst_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + cursor);
        line("  " + src_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + src_index);
        line("  " + moved + " = load " + element_llvm + ", ptr " + src_ptr);
        line("  store " + element_llvm + " " + moved + ", ptr " + dst_ptr);
        line("  " + next_cursor + " = add i64 " + cursor + ", 1");
        line("  br label %" + cond_label);

        emit_label(end_label);
        line("  store i64 " + next_len + ", ptr " + len_ptr);
        return Value{element_llvm, removed, element_type};
    }

    Value emit_vector_insert(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || !ir_expr_right(expr) || !ir_expr_payload(expr) ||
            ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.insert lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*ir_expr_right(expr)), index_type);

        std::string len_ptr = temp();
        std::string len = temp();
        std::string low = temp();
        std::string high = temp();
        std::string bad_index = temp();
        std::string bounds_fail_label = label("vector.insert.bounds.fail");
        std::string bounds_ok_label = label("vector.insert.bounds.ok");
        line("  " + len_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 0");
        line("  " + len + " = load i64, ptr " + len_ptr);
        line("  " + low + " = icmp slt i64 " + index.name + ", 0");
        line("  " + high + " = icmp sgt i64 " + index.name + ", " + len);
        line("  " + bad_index + " = or i1 " + low + ", " + high);
        line("  br i1 " + bad_index + ", label %" + bounds_fail_label + ", label %" + bounds_ok_label);
        emit_label(bounds_fail_label);
        line("  call void @ari_builtin_panic()");
        line("  unreachable");
        emit_label(bounds_ok_label);

        std::string full = temp();
        std::string full_fail_label = label("vector.insert.full");
        std::string full_ok_label = label("vector.insert.ok");
        line("  " + full + " = icmp sge i64 " + len + ", " + std::to_string(vector_type.array_size));
        line("  br i1 " + full + ", label %" + full_fail_label + ", label %" + full_ok_label);
        emit_label(full_fail_label);
        line("  call void @ari_builtin_panic()");
        line("  unreachable");
        emit_label(full_ok_label);

        Value value = cast_value(emit_expr(*ir_expr_payload(expr)), element_type);
        std::string element_llvm = llvm_type(element_type);
        std::string entry_label = current_label_;
        std::string cond_label = label("vector.insert.cond");
        std::string body_label = label("vector.insert.body");
        std::string end_label = label("vector.insert.end");
        std::string cursor = temp();
        std::string should_shift = temp();
        std::string src_index = temp();
        std::string dst_ptr = temp();
        std::string src_ptr = temp();
        std::string moved = temp();
        std::string next_cursor = temp();

        line("  br label %" + cond_label);
        emit_label(cond_label);
        line("  " + cursor + " = phi i64 [ " + len + ", %" + entry_label +
             " ], [ " + next_cursor + ", %" + body_label + " ]");
        line("  " + should_shift + " = icmp sgt i64 " + cursor + ", " + index.name);
        line("  br i1 " + should_shift + ", label %" + body_label + ", label %" + end_label);

        emit_label(body_label);
        line("  " + src_index + " = add i64 " + cursor + ", -1");
        line("  " + dst_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + cursor);
        line("  " + src_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + src_index);
        line("  " + moved + " = load " + element_llvm + ", ptr " + src_ptr);
        line("  store " + element_llvm + " " + moved + ", ptr " + dst_ptr);
        line("  " + next_cursor + " = add i64 " + cursor + ", -1");
        line("  br label %" + cond_label);

        emit_label(end_label);
        std::string item_ptr = temp();
        std::string next_len = temp();
        line("  " + item_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + index.name);
        line("  store " + value.type + " " + value.name + ", ptr " + item_ptr);
        line("  " + next_len + " = add i64 " + len + ", 1");
        line("  store i64 " + next_len + ", ptr " + len_ptr);
        return Value{"void", "", expr.type};
    }

    std::string emit_vector_value_eq(const IrType& element_type,
                                     const Value& left,
                                     const Value& right) {
        std::string matched = temp();
        bool is_float = is_llvm_float_type(element_type);
        line("  " + matched + " = " + std::string(is_float ? "fcmp oeq " : "icmp eq ") +
             left.type + " " + left.name + ", " + right.name);
        return matched;
    }

    Value emit_vector_search(const IrExpr& expr, bool return_index) {
        if (!ir_expr_operand(expr) || !ir_expr_payload(expr) ||
            ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec search lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
        Value needle = cast_value(emit_expr(*ir_expr_payload(expr)), element_type);
        std::string element_llvm = llvm_type(element_type);
        std::string len_ptr = temp();
        std::string len = temp();
        line("  " + len_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 0");
        line("  " + len + " = load i64, ptr " + len_ptr);

        std::string entry_label = current_label_;
        std::string cond_label = label(return_index ? "vector.index_of.cond" : "vector.contains.cond");
        std::string body_label = label(return_index ? "vector.index_of.body" : "vector.contains.body");
        std::string next_label = label(return_index ? "vector.index_of.next" : "vector.contains.next");
        std::string found_label = label(return_index ? "vector.index_of.found" : "vector.contains.found");
        std::string missing_label = label(return_index ? "vector.index_of.missing" : "vector.contains.missing");
        std::string end_label = label(return_index ? "vector.index_of.end" : "vector.contains.end");
        std::string cursor = temp();
        std::string keep_scanning = temp();
        std::string item_ptr = temp();
        std::string item = temp();
        std::string next_cursor = temp();

        line("  br label %" + cond_label);
        emit_label(cond_label);
        line("  " + cursor + " = phi i64 [ 0, %" + entry_label +
             " ], [ " + next_cursor + ", %" + next_label + " ]");
        line("  " + keep_scanning + " = icmp slt i64 " + cursor + ", " + len);
        line("  br i1 " + keep_scanning + ", label %" + body_label + ", label %" + missing_label);

        emit_label(body_label);
        line("  " + item_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + cursor);
        line("  " + item + " = load " + element_llvm + ", ptr " + item_ptr);
        std::string matched = emit_vector_value_eq(element_type, Value{element_llvm, item, element_type}, needle);
        line("  br i1 " + matched + ", label %" + found_label + ", label %" + next_label);

        emit_label(next_label);
        line("  " + next_cursor + " = add i64 " + cursor + ", 1");
        line("  br label %" + cond_label);

        emit_label(found_label);
        line("  br label %" + end_label);

        emit_label(missing_label);
        line("  br label %" + end_label);

        emit_label(end_label);
        std::string out = temp();
        if (return_index) {
            line("  " + out + " = phi i64 [ " + cursor + ", %" + found_label +
                 " ], [ -1, %" + missing_label + " ]");
            return Value{"i64", out, expr.type};
        }
        line("  " + out + " = phi i1 [ true, %" + found_label +
             " ], [ false, %" + missing_label + " ]");
        return Value{"i1", out, expr.type};
    }

    Value emit_vector_count(const IrExpr& expr) {
        if (!ir_expr_operand(expr) || !ir_expr_payload(expr) ||
            ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::Vector ||
            ir_expr_operand(expr)->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.count lowering");
        }
        const IrType& vector_type = ir_expr_operand(expr)->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*ir_expr_operand(expr));
        Value needle = cast_value(emit_expr(*ir_expr_payload(expr)), element_type);
        std::string element_llvm = llvm_type(element_type);
        std::string len_ptr = temp();
        std::string len = temp();
        line("  " + len_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 0");
        line("  " + len + " = load i64, ptr " + len_ptr);

        std::string entry_label = current_label_;
        std::string cond_label = label("vector.count.cond");
        std::string body_label = label("vector.count.body");
        std::string end_label = label("vector.count.end");
        std::string cursor = temp();
        std::string count = temp();
        std::string keep_scanning = temp();
        std::string item_ptr = temp();
        std::string item = temp();
        std::string increment = temp();
        std::string next_count = temp();
        std::string next_cursor = temp();

        line("  br label %" + cond_label);
        emit_label(cond_label);
        line("  " + cursor + " = phi i64 [ 0, %" + entry_label +
             " ], [ " + next_cursor + ", %" + body_label + " ]");
        line("  " + count + " = phi i64 [ 0, %" + entry_label +
             " ], [ " + next_count + ", %" + body_label + " ]");
        line("  " + keep_scanning + " = icmp slt i64 " + cursor + ", " + len);
        line("  br i1 " + keep_scanning + ", label %" + body_label + ", label %" + end_label);

        emit_label(body_label);
        line("  " + item_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + cursor);
        line("  " + item + " = load " + element_llvm + ", ptr " + item_ptr);
        std::string matched = emit_vector_value_eq(element_type, Value{element_llvm, item, element_type}, needle);
        line("  " + increment + " = zext i1 " + matched + " to i64");
        line("  " + next_count + " = add i64 " + count + ", " + increment);
        line("  " + next_cursor + " = add i64 " + cursor + ", 1");
        line("  br label %" + cond_label);

        emit_label(end_label);
        return Value{"i64", count, expr.type};
    }

    void emit_vector_bounds_check(const Value& index, const IrType& vector_type, const std::string& vector_ptr) {
        std::string fail_label = label("vector.bounds.fail");
        std::string ok_label = label("vector.bounds.ok");
        std::string len_ptr = temp();
        std::string len = temp();
        std::string low = temp();
        std::string high = temp();
        std::string bad = temp();
        line("  " + len_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + vector_ptr + ", i32 0, i32 0");
        line("  " + len + " = load i64, ptr " + len_ptr);
        line("  " + low + " = icmp slt i64 " + index.name + ", 0");
        line("  " + high + " = icmp sge i64 " + index.name + ", " + len);
        line("  " + bad + " = or i1 " + low + ", " + high);
        line("  br i1 " + bad + ", label %" + fail_label + ", label %" + ok_label);
        emit_label(fail_label);
        line("  call void @ari_builtin_panic()");
        line("  unreachable");
        emit_label(ok_label);
    }

    void emit_slice_bounds_check(const Value& index, const std::string& len) {
        std::string fail_label = label("slice.bounds.fail");
        std::string ok_label = label("slice.bounds.ok");
        std::string low = temp();
        std::string high = temp();
        std::string bad = temp();
        line("  " + low + " = icmp slt i64 " + index.name + ", 0");
        line("  " + high + " = icmp sge i64 " + index.name + ", " + len);
        line("  " + bad + " = or i1 " + low + ", " + high);
        line("  br i1 " + bad + ", label %" + fail_label + ", label %" + ok_label);
        emit_label(fail_label);
        line("  call void @ari_builtin_panic()");
        line("  unreachable");
        emit_label(ok_label);
    }

    void emit_slice_range_bounds_check(const Value& start,
                                       const Value& end,
                                       const std::string& len,
                                       bool inclusive) {
        std::string fail_label = label("slice.range.bounds.fail");
        std::string ok_label = label("slice.range.bounds.ok");
        std::string start_low = temp();
        std::string end_low = temp();
        std::string order_bad = temp();
        std::string high_bad = temp();
        std::string low_bad = temp();
        std::string range_bad = temp();
        std::string bad = temp();

        line("  " + start_low + " = icmp slt i64 " + start.name + ", 0");
        line("  " + end_low + " = icmp slt i64 " + end.name + ", 0");
        line("  " + order_bad + " = icmp sgt i64 " + start.name + ", " + end.name);
        if (inclusive) {
            line("  " + high_bad + " = icmp sge i64 " + end.name + ", " + len);
        } else {
            line("  " + high_bad + " = icmp sgt i64 " + end.name + ", " + len);
        }
        line("  " + low_bad + " = or i1 " + start_low + ", " + end_low);
        line("  " + range_bad + " = or i1 " + order_bad + ", " + high_bad);
        line("  " + bad + " = or i1 " + low_bad + ", " + range_bad);
        line("  br i1 " + bad + ", label %" + fail_label + ", label %" + ok_label);
        emit_label(fail_label);
        line("  call void @ari_builtin_panic()");
        line("  unreachable");
        emit_label(ok_label);
    }

    void emit_array_bounds_check(const Value& index, std::uint64_t array_size) {
        std::string fail_label = label("array.bounds.fail");
        std::string ok_label = label("array.bounds.ok");
        std::string low = temp();
        std::string high = temp();
        std::string bad = temp();
        line("  " + low + " = icmp slt i64 " + index.name + ", 0");
        line("  " + high + " = icmp sge i64 " + index.name + ", " + std::to_string(array_size));
        line("  " + bad + " = or i1 " + low + ", " + high);
        line("  br i1 " + bad + ", label %" + fail_label + ", label %" + ok_label);
        emit_label(fail_label);
        line("  call void @ari_builtin_panic()");
        line("  unreachable");
        emit_label(ok_label);
    }

    Value emit_enum_tag_value(SourceLocation loc, const Value& value) {
        if (!has_aggregate_enum_layout(value.ir_type)) {
            std::string tag = temp();
            line("  " + tag + " = trunc " + value.type + " " + value.name + " to i32");
            return Value{"i32", tag, enum_tag_storage_type(loc)};
        }
        std::string tag = temp();
        line("  " + tag + " = extractvalue " + value.type + " " + value.name + ", 0");
        return Value{"i32", tag, enum_tag_storage_type(loc)};
    }

    Value emit_enum_tag_expr(const IrExpr& expr) {
        const IrExpr& operand_expr = *ir_expr_operand(expr);
        std::string tag32;
        if (is_reference_like_lvalue_type(operand_expr.type)) {
            Value pointer = emit_expr(operand_expr);
            IrType storage_type = lvalue_pointee_type(operand_expr.type);
            if (has_aggregate_enum_layout(storage_type)) {
                std::string tag_ptr = temp();
                line("  " + tag_ptr + " = getelementptr inbounds " + llvm_type(storage_type) +
                     ", ptr " + pointer.name + ", i32 0, i32 0");
                tag32 = temp();
                line("  " + tag32 + " = load i32, ptr " + tag_ptr);
            } else {
                std::string loaded = temp();
                line("  " + loaded + " = load " + llvm_type(storage_type) + ", ptr " + pointer.name);
                tag32 = temp();
                line("  " + tag32 + " = trunc " + llvm_type(storage_type) + " " + loaded + " to i32");
            }
        } else {
            Value value = emit_expr(operand_expr);
            tag32 = emit_enum_tag_value(expr.loc, value).name;
        }

        std::string out = temp();
        line("  " + out + " = zext i32 " + tag32 + " to i64");
        return Value{"i64", out, expr.type};
    }

    Value emit_enum_payload_slot(SourceLocation loc, const Value& value, std::uint32_t index) {
        if (!has_aggregate_enum_layout(value.ir_type)) {
            std::string shifted = temp();
            line("  " + shifted + " = lshr " + value.type + " " + value.name + ", 32");
            return Value{value.type, shifted, enum_payload_storage_type(loc)};
        }
        std::size_t field_index = static_cast<std::size_t>(index) + 1;
        if (field_index >= value.ir_type.field_types.size()) {
            throw CompileError(where(loc) + ": enum payload index is out of range during LLVM lowering");
        }
        std::string payload = temp();
        std::string slot_type = llvm_type(value.ir_type.field_types[field_index]);
        line("  " + payload + " = extractvalue " + value.type + " " + value.name + ", " + std::to_string(field_index));
        return Value{slot_type, payload, value.ir_type.field_types[field_index]};
    }

    Value emit_enum_payload_scalar_lane(SourceLocation loc, const Value& payload) {
        const IrType* lane_type = enum_payload_slot_scalar_lane_type(payload.ir_type);
        std::optional<std::uint32_t> lane_index = enum_payload_slot_scalar_lane_index(payload.ir_type);
        if (!lane_type || !lane_index) {
            throw CompileError(where(loc) + ": enum payload slot has no scalar storage lane during LLVM lowering");
        }
        std::string lane = temp();
        line("  " + lane + " = extractvalue " + payload.type + " " + payload.name +
             ", " + std::to_string(*lane_index));
        return Value{llvm_type(*lane_type), lane, *lane_type};
    }

    Value materialize_enum_payload_for_slot(SourceLocation loc, Value payload, const IrType& slot_type) {
        if (enum_payload_slot_uses_byte_storage(slot_type, payload.ir_type)) {
            return reinterpret_payload_storage(loc, std::move(payload), slot_type);
        }
        if (!enum_payload_slot_uses_scalar_lane(slot_type, payload.ir_type)) {
            return cast_value(payload, slot_type);
        }

        const IrType* lane_type = enum_payload_slot_scalar_lane_type(slot_type);
        std::optional<std::uint32_t> lane_index = enum_payload_slot_scalar_lane_index(slot_type);
        if (!lane_type || !lane_index) {
            throw CompileError(where(loc) + ": enum payload slot has no scalar storage lane during LLVM lowering");
        }
        Value lane = cast_value(payload, *lane_type);
        std::string slot_value = temp();
        std::string slot_llvm_type = llvm_type(slot_type);
        line("  " + slot_value + " = insertvalue " + slot_llvm_type + " zeroinitializer, " +
             lane.type + " " + lane.name + ", " + std::to_string(*lane_index));
        return Value{slot_llvm_type, slot_value, slot_type};
    }

    Value cast_enum_payload_slot_to_type(SourceLocation loc, Value payload, const IrType& target) {
        if (enum_payload_slot_uses_byte_storage(payload.ir_type, target)) {
            return reinterpret_payload_storage(loc, std::move(payload), target);
        }
        if (enum_payload_slot_uses_scalar_lane(payload.ir_type, target)) {
            payload = emit_enum_payload_scalar_lane(loc, payload);
        }
        return cast_value(payload, target);
    }

    Value reinterpret_payload_storage(SourceLocation loc, Value value, const IrType& target) {
        std::string target_type = llvm_type(target);
        if (value.type == target_type) {
            value.ir_type = target;
            return value;
        }

        std::uint64_t source_size = 0;
        std::uint64_t target_size = 0;
        std::uint64_t source_align = 0;
        std::uint64_t target_align = 0;
        if (!ari_layout_size_bytes(value.ir_type, source_size) ||
            !ari_layout_size_bytes(target, target_size) ||
            !ari_layout_align_bytes(value.ir_type, source_align) ||
            !ari_layout_align_bytes(target, target_align)) {
            throw CompileError(where(loc) + ": LLVM backend cannot reinterpret enum payload storage from " +
                               type_name(value.ir_type) + " to " + type_name(target));
        }

        std::uint64_t scratch_size = std::max(source_size, target_size);
        std::uint64_t scratch_align = std::max(source_align, target_align);
        IrType scratch_type = enum_payload_byte_storage_type(loc, scratch_size);
        std::string scratch_llvm_type = llvm_type(scratch_type);
        std::string scratch = temp();
        line("  " + scratch + " = alloca " + scratch_llvm_type + ", align " + std::to_string(scratch_align));
        if (target_size > source_size) {
            line("  store " + scratch_llvm_type + " zeroinitializer, ptr " + scratch +
                 ", align " + std::to_string(scratch_align));
        }
        line("  store " + value.type + " " + value.name + ", ptr " + scratch +
             ", align " + std::to_string(source_align));
        std::string out = temp();
        line("  " + out + " = load " + target_type + ", ptr " + scratch +
             ", align " + std::to_string(target_align));
        return Value{target_type, out, target};
    }

    Value emit_payload_binding_field_path(SourceLocation loc,
                                          Value value,
                                          const std::vector<std::uint32_t>& field_path) {
        for (std::uint32_t field_index : field_path) {
            const std::vector<IrType>& fields = ari_aggregate_field_types(value.ir_type);
            if (field_index >= fields.size()) {
                throw CompileError(where(loc) + ": enum payload binding field path is out of range during LLVM lowering");
            }
            std::string field = temp();
            line("  " + field + " = extractvalue " + value.type + " " + value.name +
                 ", " + std::to_string(field_index));
            value = Value{llvm_type(fields[field_index]), field, fields[field_index]};
        }
        return value;
    }

    Value payload_literal_test_value(SourceLocation loc, Value payload) {
        IrType target = enum_payload_storage_type(loc);
        if (enum_payload_slot_uses_scalar_lane(payload.ir_type, target)) {
            return emit_enum_payload_scalar_lane(loc, payload);
        }
        return payload;
    }

    Value emit_compact_enum_payload_value(SourceLocation loc,
                                          const Value& value,
                                          std::uint32_t index,
                                          const IrType& enum_type) {
        Value payload = emit_enum_payload_slot(loc, value, index);
        if (enum_payload_slot_uses_scalar_lane(payload.ir_type, enum_type)) {
            payload = emit_enum_payload_scalar_lane(loc, payload);
        }
        return Value{llvm_type(enum_type), payload.name, enum_type};
    }

    static IrType enum_tag_storage_type(SourceLocation loc) {
        return IrType{TypeQualifier::Value, IrPrimitiveKind::I32, "i32", {}, {}, {}, {}, loc};
    }

    static IrType enum_payload_storage_type(SourceLocation loc) {
        return IrType{TypeQualifier::Value, IrPrimitiveKind::U64, "u64", {}, {}, {}, {}, loc};
    }

    template <typename Arm>
    void emit_payload_bindings(const Arm& arm, const Value& enum_value) {
        if (!arm.payload_bindings.empty()) {
            for (const auto& binding : arm.payload_bindings) {
                Value payload = binding.compact_enum_payload
                    ? emit_enum_payload_slot(
                          arm.loc,
                          emit_compact_enum_payload_value(arm.loc, enum_value, binding.index, binding.compact_enum_type),
                          binding.compact_enum_payload_index
                      )
                    : emit_enum_payload_slot(arm.loc, enum_value, binding.index);
                payload = emit_payload_binding_field_path(arm.loc, payload, binding.field_path);
                payload = cast_enum_payload_slot_to_type(arm.loc, payload, binding.type);
                line("  store " + payload.type + " " + payload.name + ", ptr " + local_slot(arm.loc, binding.name));
            }
            return;
        }
        if (!arm.has_payload_binding) return;
        Value payload = emit_enum_payload_slot(arm.loc, enum_value, arm.payload_index);
        payload = cast_enum_payload_slot_to_type(arm.loc, payload, arm.payload_type);
        line("  store " + payload.type + " " + payload.name + ", ptr " + local_slot(arm.loc, arm.payload_name));
    }

    Value emit_match_expr(const IrExpr& expr) {
        const auto& arms = ir_expr_match_arms(expr);
        if (arms.empty()) throw CompileError(where(expr.loc) + ": match expression has no arms during LLVM lowering");
        Value value = emit_expr(*ir_expr_match_value(expr));
        std::string subject = value.name;
        std::string subject_type = value.type;
        if (value.ir_type.primitive == IrPrimitiveKind::Enum) {
            Value tag = emit_enum_tag_value(expr.loc, value);
            subject = tag.name;
            subject_type = tag.type;
        }
        std::string end_label = label("match.expr.end");
        std::vector<std::string> arm_labels;
        for (std::size_t i = 0; i < arms.size(); ++i) arm_labels.push_back(label("match.expr.arm"));

        std::string first_test = label("match.expr.test");
        line("  br label %" + first_test);
        emit_label(first_test);
        for (std::size_t i = 0; i < arms.size(); ++i) {
            const IrMatchExprArm& arm = arms[i];
            if (arm.wildcard || i + 1 == arms.size()) {
                line("  br label %" + arm_labels[i]);
            } else {
                std::string next = label("match.expr.test");
                std::string cmp = emit_match_condition(arm, value, subject, subject_type);
                line("  br i1 " + cmp + ", label %" + arm_labels[i] + ", label %" + next);
                emit_label(next);
            }
        }

        std::vector<std::pair<Value, std::string>> incoming;
        for (std::size_t i = 0; i < arms.size(); ++i) {
            const IrMatchExprArm& arm = arms[i];
            emit_label(arm_labels[i]);
            if (arm.has_value_binding) {
                Value bound = cast_value(value, arm.value_type);
                line("  store " + bound.type + " " + bound.name + ", ptr " + local_slot(arm.loc, arm.value_name));
            }
            emit_payload_bindings(arm, value);
            emit_statements(arm.body);
            if (!block_terminated_) {
                Value arm_value = emit_expr(*arm.value);
                if (!block_terminated_) {
                    arm_value = cast_value(arm_value, expr.type);
                    std::string incoming_label = current_label_;
                    line("  br label %" + end_label);
                    incoming.push_back({arm_value, incoming_label});
                }
            }
        }

        if (incoming.empty()) {
            throw CompileError(where(expr.loc) + ": match expression has no reachable value arms during LLVM lowering");
        }
        emit_label(end_label);
        std::string out = temp();
        std::string type = llvm_type(expr.type);
        std::string phi = "  " + out + " = phi " + type + " ";
        for (std::size_t i = 0; i < incoming.size(); ++i) {
            if (i > 0) phi += ", ";
            phi += "[" + incoming[i].first.name + ", %" + incoming[i].second + "]";
        }
        line(phi);
        return Value{type, out, expr.type};
    }

    template <typename Arm>
    std::string emit_match_condition(const Arm& arm,
                                     const Value& value,
                                     const std::string& tag_subject,
                                     const std::string& tag_subject_type) {
        if (arm.has_range) {
            std::string lower = temp();
            std::string upper = temp();
            std::string both = temp();
            std::string lower_op = arm.range_is_unsigned ? "uge" : "sge";
            std::string upper_op = arm.range_inclusive
                ? (arm.range_is_unsigned ? "ule" : "sle")
                : (arm.range_is_unsigned ? "ult" : "slt");
            line("  " + lower + " = icmp " + lower_op + " " + value.type + " " +
                 value.name + ", " + match_range_start_constant(arm));
            line("  " + upper + " = icmp " + upper_op + " " + value.type + " " +
                 value.name + ", " + match_range_end_constant(arm));
            line("  " + both + " = and i1 " + lower + ", " + upper);
            return both;
        }

        std::string cmp = temp();
        std::string arm_subject = arm.has_literal ? value.name : tag_subject;
        std::string arm_subject_type = arm.has_literal ? value.type : tag_subject_type;
        line("  " + cmp + " = icmp eq " + arm_subject_type + " " + arm_subject + ", " +
             match_arm_constant(arm));
        std::string condition = cmp;
        for (const auto& payload_condition : arm.payload_literal_conditions) {
            Value payload = emit_enum_payload_slot(arm.loc, value, payload_condition.index);
            payload = payload_literal_test_value(arm.loc, payload);
            std::string payload_cmp = temp();
            std::string both = temp();
            line("  " + payload_cmp + " = icmp eq " + payload.type + " " + payload.name + ", " +
                 match_payload_literal_constant(payload_condition));
            line("  " + both + " = and i1 " + condition + ", " + payload_cmp);
            condition = both;
        }
        for (const auto& payload_condition : arm.payload_range_conditions) {
            Value payload = emit_enum_payload_slot(arm.loc, value, payload_condition.index);
            payload = cast_enum_payload_slot_to_type(arm.loc, payload, payload_condition.type);
            std::string lower = temp();
            std::string upper = temp();
            std::string range = temp();
            std::string both = temp();
            std::string lower_op = payload_condition.is_unsigned ? "uge" : "sge";
            std::string upper_op = payload_condition.inclusive
                ? (payload_condition.is_unsigned ? "ule" : "sle")
                : (payload_condition.is_unsigned ? "ult" : "slt");
            line("  " + lower + " = icmp " + lower_op + " " + payload.type + " " +
                 payload.name + ", " + match_payload_range_start_constant(payload_condition));
            line("  " + upper + " = icmp " + upper_op + " " + payload.type + " " +
                 payload.name + ", " + match_payload_range_end_constant(payload_condition));
            line("  " + range + " = and i1 " + lower + ", " + upper);
            line("  " + both + " = and i1 " + condition + ", " + range);
            condition = both;
        }
        for (const auto& payload_condition : arm.payload_vector_length_conditions) {
            Value payload = emit_enum_payload_slot(arm.loc, value, payload_condition.index);
            std::vector<std::uint32_t> length_path = payload_condition.field_path;
            length_path.push_back(0);
            Value length = emit_payload_binding_field_path(arm.loc, payload, length_path);
            std::string length_cmp = temp();
            std::string both = temp();
            const std::string op = payload_condition.at_least ? "sge" : "eq";
            line("  " + length_cmp + " = icmp " + op + " " + length.type + " " +
                 length.name + ", " + std::to_string(payload_condition.length));
            line("  " + both + " = and i1 " + condition + ", " + length_cmp);
            condition = both;
        }
        for (const auto& payload_condition : arm.payload_enum_conditions) {
            Value nested = emit_compact_enum_payload_value(arm.loc, value, payload_condition.index, payload_condition.enum_type);
            Value tag = emit_enum_tag_value(arm.loc, nested);
            std::string tag_cmp = temp();
            std::string tag_both = temp();
            line("  " + tag_cmp + " = icmp eq " + tag.type + " " + tag.name + ", " +
                 std::to_string(payload_condition.tag));
            line("  " + tag_both + " = and i1 " + condition + ", " + tag_cmp);
            condition = tag_both;

            if (payload_condition.has_payload_literal) {
                Value nested_payload = emit_enum_payload_slot(
                    arm.loc, nested, payload_condition.nested_payload_index);
                nested_payload = cast_enum_payload_slot_to_type(arm.loc, nested_payload, payload_condition.payload_type);
                std::string payload_cmp = temp();
                std::string both = temp();
                line("  " + payload_cmp + " = icmp eq " + nested_payload.type + " " + nested_payload.name + ", " +
                     nested_payload_literal_constant(payload_condition));
                line("  " + both + " = and i1 " + condition + ", " + payload_cmp);
                condition = both;
            }

            if (payload_condition.has_payload_range) {
                Value nested_payload = emit_enum_payload_slot(
                    arm.loc, nested, payload_condition.nested_payload_index);
                nested_payload = cast_enum_payload_slot_to_type(arm.loc, nested_payload, payload_condition.payload_type);
                std::string lower = temp();
                std::string upper = temp();
                std::string range = temp();
                std::string both = temp();
                std::string lower_op = payload_condition.range_is_unsigned ? "uge" : "sge";
                std::string upper_op = payload_condition.range_inclusive
                    ? (payload_condition.range_is_unsigned ? "ule" : "sle")
                    : (payload_condition.range_is_unsigned ? "ult" : "slt");
                line("  " + lower + " = icmp " + lower_op + " " + nested_payload.type + " " +
                     nested_payload.name + ", " + nested_payload_range_start_constant(payload_condition));
                line("  " + upper + " = icmp " + upper_op + " " + nested_payload.type + " " +
                     nested_payload.name + ", " + nested_payload_range_end_constant(payload_condition));
                line("  " + range + " = and i1 " + lower + ", " + upper);
                line("  " + both + " = and i1 " + condition + ", " + range);
                condition = both;
            }
        }
        return condition;
    }

    template <typename Arm>
    static std::string match_arm_constant(const Arm& arm) {
        if (!arm.has_literal) return std::to_string(arm.enum_tag);
        if (arm.literal_is_bool) return arm.literal_bool ? "1" : "0";
        return (arm.literal_negative ? "-" : "") + std::to_string(arm.literal_int);
    }

    static std::string match_payload_literal_constant(const IrPayloadLiteralCondition& condition) {
        if (condition.is_bool) return condition.bool_literal() ? "1" : "0";
        return std::to_string(condition.bits());
    }

    static std::string match_payload_range_start_constant(const IrPayloadRangeCondition& condition) {
        return (condition.start_negative ? "-" : "") + std::to_string(condition.start_int);
    }

    static std::string match_payload_range_end_constant(const IrPayloadRangeCondition& condition) {
        return (condition.end_negative ? "-" : "") + std::to_string(condition.end_int);
    }

    static std::string nested_payload_literal_constant(const IrPayloadEnumCondition& condition) {
        if (condition.payload_literal_is_bool) return condition.payload_literal.boolean ? "1" : "0";
        return (condition.payload_literal_negative ? "-" : "") + std::to_string(condition.payload_literal.integer);
    }

    static std::string nested_payload_range_start_constant(const IrPayloadEnumCondition& condition) {
        return (condition.range_start_negative ? "-" : "") + std::to_string(condition.range_start_int);
    }

    static std::string nested_payload_range_end_constant(const IrPayloadEnumCondition& condition) {
        return (condition.range_end_negative ? "-" : "") + std::to_string(condition.range_end_int);
    }

    template <typename Arm>
    static std::string match_range_start_constant(const Arm& arm) {
        return (arm.range_start_negative ? "-" : "") + std::to_string(arm.range_start_int);
    }

    template <typename Arm>
    static std::string match_range_end_constant(const Arm& arm) {
        return (arm.range_end_negative ? "-" : "") + std::to_string(arm.range_end_int);
    }

    Value emit_if_expr(const IrExpr& expr) {
        Value cond = emit_expr(*ir_expr_if_condition(expr));
        std::string then_label = label("if.expr.then");
        std::string else_label = label("if.expr.else");
        std::string end_label = label("if.expr.end");
        line("  br i1 " + cond.name + ", label %" + then_label + ", label %" + else_label);

        emit_label(then_label);
        emit_statements(ir_expr_if_then_body(expr));
        std::vector<std::pair<Value, std::string>> incoming;
        if (!block_terminated_) {
            Value then_value = emit_expr(*ir_expr_if_then_value(expr));
            if (!block_terminated_) {
                then_value = cast_value(then_value, expr.type);
                incoming.push_back({then_value, current_label_});
                line("  br label %" + end_label);
            }
        }

        emit_label(else_label);
        emit_statements(ir_expr_if_else_body(expr));
        if (!block_terminated_) {
            Value else_value = emit_expr(*ir_expr_if_else_value(expr));
            if (!block_terminated_) {
                else_value = cast_value(else_value, expr.type);
                incoming.push_back({else_value, current_label_});
                line("  br label %" + end_label);
            }
        }

        if (incoming.empty()) {
            throw CompileError(where(expr.loc) + ": if expression has no reachable value arms during LLVM lowering");
        }
        emit_label(end_label);
        std::string out = temp();
        std::string type = llvm_type(expr.type);
        std::string phi = "  " + out + " = phi " + type + " ";
        for (std::size_t i = 0; i < incoming.size(); ++i) {
            if (i > 0) phi += ", ";
            phi += "[" + incoming[i].first.name + ", %" + incoming[i].second + "]";
        }
        line(phi);
        return Value{type, out, expr.type};
    }

    Value emit_block_expr(const IrExpr& expr) {
        if (!ir_expr_block_label(expr).empty()) {
            std::string type = llvm_type(expr.type);
            std::string slot = temp();
            std::string end_label = label("block.expr.end");
            line("  " + slot + " = alloca " + type);

            LoopContext context;
            context.break_label = end_label;
            context.source_label = ir_expr_block_label(expr);
            context.is_loop = false;
            context.supports_break_value = true;
            context.break_value_slot = slot;
            context.break_value_type = expr.type;
            loops_.push_back(context);

            emit_statements(ir_expr_block_body(expr));
            bool body_terminated = block_terminated_;
            bool has_break = loops_.back().has_break;
            loops_.pop_back();

            if (!body_terminated) {
                Value value = cast_value(emit_expr(*ir_expr_block_value(expr)), expr.type);
                line("  store " + value.type + " " + value.name + ", ptr " + slot);
                line("  br label %" + end_label);
            } else if (!has_break) {
                throw CompileError(where(expr.loc) + ": labeled block expression has no reachable value during LLVM lowering");
            }

            emit_label(end_label);
            std::string out = temp();
            line("  " + out + " = load " + type + ", ptr " + slot);
            return Value{type, out, expr.type};
        }
        emit_statements(ir_expr_block_body(expr));
        return emit_expr(*ir_expr_block_value(expr));
    }

    Value emit_trait_object_cast(const IrExpr& expr) {
        Value source = emit_expr(*ir_expr_operand(expr));
        if (ir_expr_operand(expr)->type.primitive == IrPrimitiveKind::TraitObject) {
            std::string data = temp();
            line("  " + data + " = extractvalue " + source.type + " " + source.name + ", 0");
            std::string source_vtable = temp();
            line("  " + source_vtable + " = extractvalue " + source.type + " " + source.name + ", 1");

            std::string target_vtable = source_vtable;
            if (expr.tuple_index != 0) {
                target_vtable = temp();
                line("  " + target_vtable + " = getelementptr ptr, ptr " + source_vtable +
                     ", i64 " + std::to_string(expr.tuple_index));
            }

            std::string object_type = llvm_type(expr.type);
            std::string with_data = temp();
            line("  " + with_data + " = insertvalue " + object_type + " undef, ptr " + data + ", 0");
            std::string with_vtable = temp();
            line("  " + with_vtable + " = insertvalue " + object_type + " " + with_data +
                 ", ptr " + target_vtable + ", 1");
            if (expr.type.qualifier != TypeQualifier::Own) {
                return Value{object_type, with_vtable, expr.type};
            }
            if (ir_expr_operand(expr)->type.qualifier != TypeQualifier::Own) {
                throw CompileError(where(expr.loc) + ": own dyn upcast requires an own dyn source during LLVM lowering");
            }
            std::string drop = temp();
            line("  " + drop + " = extractvalue " + source.type + " " + source.name + ", 2");
            std::string with_drop = temp();
            line("  " + with_drop + " = insertvalue " + object_type + " " + with_vtable +
                 ", ptr " + drop + ", 2");
            return Value{object_type, with_drop, expr.type};
        }

        std::string data_pointer = source.name;
        if (expr.type.qualifier == TypeQualifier::Own &&
            ir_expr_operand(expr)->type.qualifier == TypeQualifier::Ptr) {
            data_pointer = source.name;
        } else if (ir_expr_operand(expr)->type.qualifier != TypeQualifier::Ref &&
            ir_expr_operand(expr)->type.qualifier != TypeQualifier::MutRef) {
            data_pointer = temp();
            line("  " + data_pointer + " = alloca " + source.type);
            line("  store " + source.type + " " + source.name + ", ptr " + data_pointer);
        }

        std::string object_type = llvm_type(expr.type);
        std::string with_data = temp();
        line("  " + with_data + " = insertvalue " + object_type + " undef, ptr " + data_pointer + ", 0");
        std::string with_vtable = temp();
        line("  " + with_vtable + " = insertvalue " + object_type + " " + with_data +
             ", ptr " + quote_global(ir_expr_name(expr)) + ", 1");
        if (expr.type.qualifier != TypeQualifier::Own) {
            return Value{object_type, with_vtable, expr.type};
        }
        if (ir_expr_label(expr).empty()) {
            throw CompileError(where(expr.loc) + ": own dyn conversion is missing a drop thunk during LLVM lowering");
        }
        std::string with_drop = temp();
        line("  " + with_drop + " = insertvalue " + object_type + " " + with_vtable +
             ", ptr " + quote_global(ir_expr_label(expr)) + ", 2");
        return Value{object_type, with_drop, expr.type};
    }

    Value emit_trait_object_call(const IrExpr& expr) {
        const std::vector<IrType>& call_param_types = ir_expr_call_param_types(expr);
        if (call_param_types.empty()) {
            throw CompileError(where(expr.loc) + ": malformed trait object call");
        }

        Value object = emit_expr(*ir_expr_operand(expr));
        std::string data = temp();
        line("  " + data + " = extractvalue " + object.type + " " + object.name + ", 0");
        std::string vtable = temp();
        line("  " + vtable + " = extractvalue " + object.type + " " + object.name + ", 1");
        std::string slot_ptr = temp();
        line("  " + slot_ptr + " = getelementptr ptr, ptr " + vtable + ", i64 " + std::to_string(expr.tuple_index));
        std::string callee = temp();
        line("  " + callee + " = load ptr, ptr " + slot_ptr);

        std::vector<Value> args;
        args.reserve(expr.args.size() + 1);
        args.push_back(Value{"ptr", data, call_param_types[0]});
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            Value arg = emit_expr(*expr.args[i]);
            if (i + 1 < call_param_types.size()) arg = cast_value(arg, call_param_types[i + 1]);
            args.push_back(arg);
        }

        std::string result_type = llvm_type(expr.type);
        std::string call = "call " + result_type + " " + callee + "(";
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (i > 0) call += ", ";
            call += args[i].type + " " + args[i].name;
        }
        call += ")";

        if (is_void_value_type(expr.type)) {
            line("  " + call);
            return Value{"void", "", expr.type};
        }
        std::string out = temp();
        line("  " + out + " = " + call);
        return Value{result_type, out, expr.type};
    }

    Value emit_trait_object_drop(const IrExpr& expr) {
        Value object = emit_expr(*ir_expr_operand(expr));
        if (ir_expr_operand(expr)->type.qualifier != TypeQualifier::Own ||
            ir_expr_operand(expr)->type.primitive != IrPrimitiveKind::TraitObject) {
            throw CompileError(where(expr.loc) + ": LLVM backend can only drop own dyn trait objects");
        }
        std::string data = temp();
        line("  " + data + " = extractvalue " + object.type + " " + object.name + ", 0");
        std::string drop = temp();
        line("  " + drop + " = extractvalue " + object.type + " " + object.name + ", 2");
        line("  call void " + drop + "(ptr " + data + ")");
        return Value{"void", "", expr.type};
    }

    Value cast_value(Value value, const IrType& target) {
        std::string target_type = llvm_type(target);
        if (value.type == target_type) {
            value.ir_type = target;
            return value;
        }
        if (target_type == "ptr" && value.type == "ptr") {
            value.ir_type = target;
            value.type = target_type;
            return value;
        }
        int from_bits = integer_bits(value.ir_type);
        int to_bits = integer_bits(target);
        if (value.type == "ptr" && to_bits > 0) {
            std::string out = temp();
            line("  " + out + " = ptrtoint ptr " + value.name + " to " + target_type);
            return Value{target_type, out, target};
        }
        if (target_type == "ptr" && from_bits > 0) {
            std::string out = temp();
            line("  " + out + " = inttoptr " + value.type + " " + value.name + " to ptr");
            return Value{target_type, out, target};
        }
        if (from_bits > 0 && to_bits > 0) {
            std::string out = temp();
            if (from_bits < to_bits) {
                std::string op = is_unsigned_integer_type(value.ir_type) || value.ir_type.primitive == IrPrimitiveKind::Bool ? "zext" : "sext";
                line("  " + out + " = " + op + " " + value.type + " " + value.name + " to " + target_type);
            } else if (from_bits > to_bits) {
                line("  " + out + " = trunc " + value.type + " " + value.name + " to " + target_type);
            } else {
                out = value.name;
            }
            return Value{target_type, out, target};
        }
        int from_float_bits = float_bits(value.ir_type);
        int to_float_bits = float_bits(target);
        if (from_bits > 0 && to_float_bits > 0) {
            std::string out = temp();
            std::string op = is_unsigned_integer_type(value.ir_type) ? "uitofp" : "sitofp";
            line("  " + out + " = " + op + " " + value.type + " " + value.name + " to " + target_type);
            return Value{target_type, out, target};
        }
        if (from_float_bits > 0 && to_bits > 0) {
            std::string out = temp();
            std::string op = is_unsigned_integer_type(target) ? "fptoui" : "fptosi";
            line("  " + out + " = " + op + " " + value.type + " " + value.name + " to " + target_type);
            return Value{target_type, out, target};
        }
        if (from_float_bits > 0 && to_float_bits > 0) {
            std::string out = temp();
            if (from_float_bits < to_float_bits) {
                line("  " + out + " = fpext " + value.type + " " + value.name + " to " + target_type);
            } else if (from_float_bits > to_float_bits) {
                line("  " + out + " = fptrunc " + value.type + " " + value.name + " to " + target_type);
            } else {
                out = value.name;
            }
            return Value{target_type, out, target};
        }
        throw CompileError(where(target.loc) + ": LLVM backend cannot cast " + type_name(value.ir_type) + " to " + type_name(target));
    }

    Value emit_pointer_offset(const IrExpr& expr) {
        Value pointer = cast_value(emit_expr(*ir_expr_operand(expr)), expr.type);
        IrType offset_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value offset = cast_value(emit_expr(*ir_expr_right(expr)), offset_type);
        std::string out = temp();
        line("  " + out + " = getelementptr i8, ptr " + pointer.name + ", i64 " + offset.name);
        return Value{"ptr", out, expr.type};
    }

    Value emit_pointer_add(const IrExpr& expr) {
        Value pointer = cast_value(emit_expr(*ir_expr_operand(expr)), expr.type);
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*ir_expr_right(expr)), index_type);
        IrType element_type = expr.type;
        element_type.qualifier = TypeQualifier::Value;
        std::string out = temp();
        line("  " + out + " = getelementptr " + llvm_type(element_type) +
             ", ptr " + pointer.name + ", i64 " + index.name);
        return Value{"ptr", out, expr.type};
    }

    Value emit_pointer_load(const IrExpr& expr) {
        Value pointer = emit_expr(*ir_expr_operand(expr));
        std::string out = temp();
        std::string element_type = llvm_type(expr.type);
        line("  " + out + " = load " + element_type + ", ptr " + pointer.name);
        return Value{element_type, out, expr.type};
    }

    Value emit_pointer_store(const IrExpr& expr) {
        if (!ir_expr_right(expr)) {
            throw CompileError(where(expr.loc) + ": malformed pointer store expression");
        }
        Value pointer = emit_expr(*ir_expr_operand(expr));
        Value value = cast_value(emit_expr(*ir_expr_right(expr)), ir_expr_right(expr)->type);
        line("  store " + value.type + " " + value.name + ", ptr " + pointer.name);
        return Value{llvm_type(expr.type), "", expr.type};
    }

    Value emit_enum_construct(const IrExpr& expr) {
        std::vector<Value> payloads;
        if (has_aggregate_enum_layout(expr.type)) {
            payloads.reserve(expr.args.size());
            for (const auto& arg : expr.args) {
                payloads.push_back(emit_expr(*arg));
            }
        } else if (ir_expr_has_enum_payload(expr)) {
            payloads.push_back(emit_expr(*ir_expr_payload(expr)));
        }
        return emit_enum_value_from_payloads(expr.loc, expr.type, ir_expr_enum_tag(expr), std::move(payloads));
    }

    Value emit_enum_value_from_payloads(SourceLocation loc,
                                        const IrType& enum_type,
                                        std::uint32_t tag,
                                        std::vector<Value> payloads) {
        if (has_aggregate_enum_layout(enum_type)) {
            std::string llvm_enum_type = llvm_type(enum_type);
            std::string value = "zeroinitializer";
            std::string with_tag = temp();
            line("  " + with_tag + " = insertvalue " + llvm_enum_type + " " + value +
                 ", i32 " + std::to_string(tag) + ", 0");
            value = with_tag;
            for (std::size_t i = 0; i < payloads.size(); ++i) {
                IrType slot_type = enum_type.field_types.at(i + 1);
                Value payload = materialize_enum_payload_for_slot(loc, std::move(payloads[i]), slot_type);
                std::string next = temp();
                line("  " + next + " = insertvalue " + llvm_enum_type + " " + value +
                     ", " + payload.type + " " + payload.name + ", " + std::to_string(i + 1));
                value = next;
            }
            return Value{llvm_enum_type, value, enum_type};
        }
        if (payloads.empty()) {
            return Value{llvm_type(enum_type), std::to_string(tag), enum_type};
        }
        if (payloads.size() != 1) {
            throw CompileError(where(loc) + ": compact enum construction received multiple payloads during LLVM lowering");
        }
        Value payload = cast_value(
            std::move(payloads[0]),
            IrType{TypeQualifier::Value, IrPrimitiveKind::U64, "u64", {}, {}, {}, {}, loc});
        std::string shifted = temp();
        line("  " + shifted + " = shl i64 " + payload.name + ", 32");
        std::string out = temp();
        line("  " + out + " = or i64 " + shifted + ", " + std::to_string(tag));
        return Value{llvm_type(enum_type), out, enum_type};
    }

    Value emit_try_residual_return_value(const IrExpr& expr, const Value& value) {
        if (!ir_expr_try_converts_residual(expr)) return cast_value(value, current_return_);
        std::vector<Value> payloads;
        if (ir_expr_try_residual_has_payload(expr)) {
            Value payload = emit_enum_payload_slot(expr.loc, value, 0);
            payload = cast_enum_payload_slot_to_type(
                expr.loc,
                std::move(payload),
                ir_expr_try_return_residual_payload_type(expr));
            payloads.push_back(std::move(payload));
        }
        return emit_enum_value_from_payloads(
            expr.loc,
            current_return_,
            ir_expr_try_return_residual_tag(expr),
            std::move(payloads));
    }

    Value emit_try(const IrExpr& expr) {
        Value value = emit_expr(*ir_expr_operand(expr));
        Value tag = emit_enum_tag_value(expr.loc, value);
        std::string ok = label("try.ok");
        std::string fail = label("try.return");
        std::string cont = label("try.cont");
        std::string cmp = temp();
        line("  " + cmp + " = icmp eq i32 " + tag.name + ", " + std::to_string(ir_expr_enum_tag(expr)));
        line("  br i1 " + cmp + ", label %" + ok + ", label %" + fail);

        emit_label(fail);
        Value residual = emit_try_residual_return_value(expr, value);
        emit_statements(ir_expr_try_residual_cleanup(expr));
        line("  ret " + residual.type + " " + residual.name);

        emit_label(ok);
        Value payload = emit_enum_payload_slot(expr.loc, value, 0);
        payload = cast_enum_payload_slot_to_type(expr.loc, std::move(payload), ir_expr_enum_payload_type(expr));
        line("  br label %" + cont);

        emit_label(cont);
        std::string out = temp();
        line("  " + out + " = phi " + payload.type + " [" + payload.name + ", %" + ok + "]");
        return Value{payload.type, out, expr.type};
    }

    Value emit_null_coalesce(const IrExpr& expr) {
        Value value = emit_expr(*ir_expr_left(expr));
        Value tag = emit_enum_tag_value(expr.loc, value);
        std::string ok = label("coalesce.ok");
        std::string fallback = label("coalesce.fallback");
        std::string end = label("coalesce.end");
        std::string cmp = temp();
        line("  " + cmp + " = icmp eq i32 " + tag.name + ", " + std::to_string(ir_expr_enum_tag(expr)));
        line("  br i1 " + cmp + ", label %" + ok + ", label %" + fallback);

        emit_label(ok);
        Value payload = emit_enum_payload_slot(expr.loc, value, 0);
        payload = cast_value(payload, ir_expr_enum_payload_type(expr));
        std::string ok_label = current_label_;
        line("  br label %" + end);

        emit_label(fallback);
        Value fallback_value = cast_value(emit_expr(*ir_expr_right(expr)), expr.type);
        std::string fallback_label = current_label_;
        line("  br label %" + end);

        emit_label(end);
        std::string out = temp();
        std::string type = llvm_type(expr.type);
        line("  " + out + " = phi " + type + " [" + payload.name + ", %" + ok_label +
             "], [" + fallback_value.name + ", %" + fallback_label + "]");
        return Value{type, out, expr.type};
    }

    Value emit_format_print(const IrExpr& expr) {
        if (!ir_expr_has_format_print_payload(expr)) {
            throw CompileError(where(expr.loc) + ": format print expression is missing format payload");
        }
        const std::vector<std::string>& format_parts = ir_expr_format_parts(expr);
        const std::vector<IrFormatSpec>& format_specs = ir_expr_format_specs(expr);
        std::string fmt_string = string_ptr("%s");
        for (std::size_t i = 0; i < format_parts.size(); ++i) {
            if (!format_parts[i].empty()) {
                line("  call i32 (ptr, ...) @printf(ptr " + fmt_string + ", ptr " + string_ptr(format_parts[i]) + ")");
            }
            if (i < expr.args.size()) {
                Value arg = emit_expr(*expr.args[i]);
                const IrFormatSpec& spec = i < format_specs.size() ? format_specs[i] : IrFormatSpec{};
                if (arg.ir_type.primitive == IrPrimitiveKind::String) {
                    if (spec.debug) {
                        line("  call i32 (ptr, ...) @printf(ptr " + fmt_string + ", ptr " + string_ptr("\"") + ")");
                    }
                    line("  call i32 (ptr, ...) @printf(ptr " + fmt_string + ", ptr " + arg.name + ")");
                    if (spec.debug) {
                        line("  call i32 (ptr, ...) @printf(ptr " + fmt_string + ", ptr " + string_ptr("\"") + ")");
                    }
                } else if (arg.ir_type.primitive == IrPrimitiveKind::Bool) {
                    line("  call i64 @ari_builtin_write_bool(i1 " + arg.name + ")");
                } else if (arg.ir_type.primitive == IrPrimitiveKind::F32 ||
                           arg.ir_type.primitive == IrPrimitiveKind::F64) {
                    Value wide = arg;
                    if (arg.ir_type.primitive == IrPrimitiveKind::F32) {
                        std::string out = temp();
                        line("  " + out + " = fpext float " + arg.name + " to double");
                        IrType double_type{TypeQualifier::Value, IrPrimitiveKind::F64, "f64", {}, {}, {}, {}, expr.loc};
                        wide = Value{"double", out, double_type};
                    }
                    int precision = spec.precision;
                    std::string printf_format = precision >= 0 ? "%." + std::to_string(precision) + "f" : "%f";
                    line("  call i32 (ptr, ...) @printf(ptr " + string_ptr(printf_format) + ", double " + wide.name + ")");
                } else if (arg.ir_type.primitive == IrPrimitiveKind::U8 || arg.ir_type.primitive == IrPrimitiveKind::I8) {
                    Value byte = cast_value(arg, IrType{TypeQualifier::Value, IrPrimitiveKind::U8, "u8", {}, {}, {}, {}, expr.loc});
                    line("  call i64 @ari_builtin_write_byte(i8 " + byte.name + ")");
                } else if (arg.ir_type.primitive == IrPrimitiveKind::U64) {
                    line("  call i64 @ari_builtin_write_u64(i64 " + arg.name + ")");
                } else {
                    Value wide = cast_value(arg, IrType{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc});
                    line("  call i64 @ari_builtin_write_i64(i64 " + wide.name + ")");
                }
            }
        }
        if (ir_expr_format_print_newline(expr)) line("  call i64 @ari_builtin_newline()");
        return Value{"i64", "0", expr.type};
    }

    Value emit_function_ref(const IrExpr& expr) {
        const std::string& name = ir_expr_name(expr);
        if (extern_symbols_.count(name)) {
            return Value{"ptr", quote_global(extern_symbols_.at(name)), expr.type};
        }
        auto found = function_symbols_.find(name);
        if (found == function_symbols_.end()) {
            throw CompileError(where(expr.loc) + ": LLVM backend cannot find function '" + name + "'");
        }
        return Value{"ptr", quote_global(found->second), expr.type};
    }

    static bool is_function_pointer_type(const IrType& type) {
        return type.qualifier == TypeQualifier::Value &&
               type.primitive == IrPrimitiveKind::Function &&
               !type.args.empty() &&
               type.array_size + 1 == type.args.size();
    }

    static bool is_lambda_closure_type(const IrType& type) {
        return type.qualifier == TypeQualifier::Value &&
               type.primitive == IrPrimitiveKind::Struct &&
               !type.field_names.empty() &&
               type.field_names[0] == "$call" &&
               !type.field_types.empty() &&
               is_function_pointer_type(type.field_types[0]) &&
               !type.args.empty() &&
               type.array_size + 1 == type.args.size();
    }

    Value emit_indirect_call(const IrExpr& expr) {
        const IrType& callee_type = ir_expr_operand(expr)->type;
        if (!is_function_pointer_type(callee_type) && !is_lambda_closure_type(callee_type)) {
            throw CompileError(where(expr.loc) + ": LLVM backend cannot call non-callable value");
        }
        std::size_t param_count = static_cast<std::size_t>(callee_type.array_size);
        IrType result = callee_type.args[param_count];

        std::vector<Value> args;
        std::string callee_name;
        if (is_lambda_closure_type(callee_type)) {
            Value closure = emit_expr(*ir_expr_operand(expr));
            callee_name = temp();
            line("  " + callee_name + " = extractvalue " + closure.type + " " + closure.name + ", 0");
            for (std::size_t i = 1; i < callee_type.field_types.size(); ++i) {
                const IrType& capture_type = callee_type.field_types[i];
                std::string capture = temp();
                line("  " + capture + " = extractvalue " + closure.type + " " + closure.name +
                     ", " + std::to_string(i));
                args.push_back(Value{llvm_type(capture_type), capture, capture_type});
            }
        } else {
            callee_name = emit_expr(*ir_expr_operand(expr)).name;
        }
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            Value arg = emit_expr(*expr.args[i]);
            if (i < param_count) arg = cast_value(arg, callee_type.args[i]);
            args.push_back(arg);
        }

        std::string result_type = llvm_type(result);
        std::string call = "call " + result_type + " " + callee_name + "(";
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (i > 0) call += ", ";
            call += args[i].type + " " + args[i].name;
        }
        call += ")";
        if (is_void_value_type(result)) {
            line("  " + call);
            return Value{"void", "", result};
        }
        std::string out = temp();
        line("  " + out + " = " + call);
        return Value{result_type, out, result};
    }

    Value emit_zone_new(const IrExpr& expr) {
        if (expr.args.size() != 4) {
            throw CompileError(where(expr.loc) + ": malformed zone::new lowering");
        }

        Value zone = emit_expr(*expr.args[0]);
        Value bytes = emit_expr(*expr.args[1]);
        Value align = emit_expr(*expr.args[2]);

        std::string out = temp();
        line("  " + out + " = call ptr @\"ari_builtin_zone_alloc\"(" +
             zone.type + " " + zone.name + ", " +
             bytes.type + " " + bytes.name + ", " +
             align.type + " " + align.name + ")");

        IrType value_type = expr.type;
        value_type.qualifier = TypeQualifier::Value;
        Value value = cast_value(emit_expr(*expr.args[3]), value_type);
        line("  store " + value.type + " " + value.name + ", ptr " + out);
        return Value{"ptr", out, expr.type};
    }

    Value emit_call(const IrExpr& expr) {
        const std::string& name = ir_expr_name(expr);
        if (name == "zone::new") return emit_zone_new(expr);

        std::string symbol;
        IrType result;
        std::vector<IrParam> params;
        if (extern_symbols_.count(name)) {
            symbol = extern_symbols_.at(name);
            result = extern_results_.at(name);
            params = extern_params_.at(name);
        } else if (function_symbols_.count(name)) {
            symbol = function_symbols_.at(name);
            result = function_results_.at(name);
            params = function_params_.at(name);
        } else if (std::optional<std::string> builtin_symbol = ari_builtin_symbol_for_source_name(name)) {
            symbol = *builtin_symbol;
            result = (symbol == "ari_builtin_zone_alloc" ||
                      symbol == "ari_builtin_string_alloc_buffer" ||
                      symbol == "ari_builtin_string_with_capacity" ||
                      symbol == "ari_builtin_string_new" ||
                      symbol == "ari_builtin_string_from_string" ||
                      symbol == "ari_builtin_string_copy_to" ||
                      symbol == "ari_builtin_read_line_owned")
                         ? expr.type
                         : builtin_result_type(symbol, expr.loc);
        } else {
            symbol = function_symbols_.at(name);
            result = function_results_.at(name);
            params = function_params_.at(name);
        }

        std::vector<Value> args;
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            Value arg = emit_expr(*expr.args[i]);
            if (i < params.size()) arg = cast_value(arg, params[i].type);
            args.push_back(arg);
        }

        std::string result_type = llvm_type(result);
        std::string call = "call " + result_type + " " + quote_global(symbol) + "(";
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (i > 0) call += ", ";
            call += args[i].type + " " + args[i].name;
        }
        call += ")";
        if (is_void_value_type(result)) {
            line("  " + call);
            if (is_diverging_builtin_call(expr)) {
                line("  unreachable");
                block_terminated_ = true;
            }
            return Value{"void", "", result};
        }
        std::string out = temp();
        line("  " + out + " = " + call);
        return Value{result_type, out, result};
    }

    Value emit_binary(const IrExpr& expr) {
        Value left = emit_expr(*ir_expr_left(expr));
        Value right = cast_value(emit_expr(*ir_expr_right(expr)), left.ir_type);
        std::string out = temp();
        bool is_unsigned = is_unsigned_integer_type(left.ir_type);
        bool is_float = is_llvm_float_type(left.ir_type);
        switch (expr.op) {
            case IrBinaryOp::Add: line("  " + out + " = " + std::string(is_float ? "fadd " : "add ") + left.type + " " + left.name + ", " + right.name); break;
            case IrBinaryOp::Sub: line("  " + out + " = " + std::string(is_float ? "fsub " : "sub ") + left.type + " " + left.name + ", " + right.name); break;
            case IrBinaryOp::Mul: line("  " + out + " = " + std::string(is_float ? "fmul " : "mul ") + left.type + " " + left.name + ", " + right.name); break;
            case IrBinaryOp::Div: line("  " + out + " = " + std::string(is_float ? "fdiv " : (is_unsigned ? "udiv " : "sdiv ")) + left.type + " " + left.name + ", " + right.name); break;
            case IrBinaryOp::Mod: line("  " + out + " = " + std::string(is_unsigned ? "urem " : "srem ") + left.type + " " + left.name + ", " + right.name); break;
            case IrBinaryOp::BitAnd:
            case IrBinaryOp::LogicalAnd: line("  " + out + " = and " + left.type + " " + left.name + ", " + right.name); break;
            case IrBinaryOp::BitOr:
            case IrBinaryOp::LogicalOr: line("  " + out + " = or " + left.type + " " + left.name + ", " + right.name); break;
            case IrBinaryOp::BitXor: line("  " + out + " = xor " + left.type + " " + left.name + ", " + right.name); break;
            case IrBinaryOp::Shl: line("  " + out + " = shl " + left.type + " " + left.name + ", " + right.name); break;
            case IrBinaryOp::Shr: line("  " + out + " = " + std::string(is_unsigned ? "lshr " : "ashr ") + left.type + " " + left.name + ", " + right.name); break;
            case IrBinaryOp::Eq: line("  " + out + " = " + std::string(is_float ? "fcmp oeq " : "icmp eq ") + left.type + " " + left.name + ", " + right.name); return Value{"i1", out, expr.type};
            case IrBinaryOp::Ne: line("  " + out + " = " + std::string(is_float ? "fcmp one " : "icmp ne ") + left.type + " " + left.name + ", " + right.name); return Value{"i1", out, expr.type};
            case IrBinaryOp::Lt: line("  " + out + " = " + std::string(is_float ? "fcmp olt " : (is_unsigned ? "icmp ult " : "icmp slt ")) + left.type + " " + left.name + ", " + right.name); return Value{"i1", out, expr.type};
            case IrBinaryOp::Le: line("  " + out + " = " + std::string(is_float ? "fcmp ole " : (is_unsigned ? "icmp ule " : "icmp sle ")) + left.type + " " + left.name + ", " + right.name); return Value{"i1", out, expr.type};
            case IrBinaryOp::Gt: line("  " + out + " = " + std::string(is_float ? "fcmp ogt " : (is_unsigned ? "icmp ugt " : "icmp sgt ")) + left.type + " " + left.name + ", " + right.name); return Value{"i1", out, expr.type};
            case IrBinaryOp::Ge: line("  " + out + " = " + std::string(is_float ? "fcmp oge " : (is_unsigned ? "icmp uge " : "icmp sge ")) + left.type + " " + left.name + ", " + right.name); return Value{"i1", out, expr.type};
        }
        return Value{left.type, out, expr.type};
    }
};

} // namespace

std::string emit_llvm_ir(const IrProgram& program, LlvmEmitOptions options) {
    LlvmEmitter emitter(program, options);
    return emitter.emit();
}

} // namespace ari
