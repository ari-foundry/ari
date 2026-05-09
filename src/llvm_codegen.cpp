#include "ari_builtin.hpp"
#include "llvm_codegen.hpp"

#include "common.hpp"
#include "symbol_mangle.hpp"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

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
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Enum &&
           !type.field_types.empty();
}

static bool is_prelude_slice_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::Slice" &&
           type.args.size() == 1 &&
           type.field_names.size() == 2 &&
           type.field_names[0] == "data" &&
           type.field_names[1] == "len";
}

class LlvmEmitter {
public:
    explicit LlvmEmitter(const IrProgram& program) : program_(program) {}

    std::string emit() {
        collect_symbols();
        emit_extern_decls();
        emit_runtime();
        emit_trait_object_vtables();
        for (const auto& fn : program_.functions) emit_function(fn);
        if (program_.require_main) emit_main_wrapper();

        std::ostringstream out;
        out << "; Ari LLVM IR backend\n";
        out << "target triple = \"x86_64-pc-linux-gnu\"\n\n";
        out << "@ari_argc = internal global i32 0\n";
        out << "@ari_argv = internal global ptr null\n\n";
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

    std::string llvm_type(const IrType& type) const {
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
                {
                    std::string text = "{ ";
                    for (std::size_t i = 0; i < type.field_types.size(); ++i) {
                        if (i > 0) text += ", ";
                        text += llvm_type(type.field_types[i]);
                    }
                    text += " }";
                    return text;
                }
            case IrPrimitiveKind::F32: return "float";
            case IrPrimitiveKind::F64: return "double";
            case IrPrimitiveKind::F128: return "fp128";
            case IrPrimitiveKind::String: return "ptr";
            case IrPrimitiveKind::Function: return "ptr";
            case IrPrimitiveKind::Zone: return "ptr";
            case IrPrimitiveKind::TraitObject:
                return "{ ptr, ptr }";
            case IrPrimitiveKind::Struct: {
                std::string text = "{ ";
                for (std::size_t i = 0; i < type.field_types.size(); ++i) {
                    if (i > 0) text += ", ";
                    text += llvm_type(type.field_types[i]);
                }
                text += " }";
                return text;
            }
            case IrPrimitiveKind::Tuple: {
                if (type.args.empty()) return "{}";
                std::string text = "{ ";
                for (std::size_t i = 0; i < type.args.size(); ++i) {
                    if (i > 0) text += ", ";
                    text += llvm_type(type.args[i]);
                }
                text += " }";
                return text;
            }
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
        if (type.primitive == IrPrimitiveKind::Void) return "";
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
               symbol == "malloc" ||
               symbol == "free" ||
               symbol == "exit";
    }

    static IrType builtin_result_type(const std::string& symbol, SourceLocation loc) {
        if (symbol == "ari_builtin_zone_create") {
            return IrType{TypeQualifier::Own, IrPrimitiveKind::Zone, "Zone", {}, {}, {}, {}, loc};
        }
        if (symbol == "ari_builtin_zone_alloc") {
            return IrType{TypeQualifier::Ptr, IrPrimitiveKind::U8, "u8", {}, {}, {}, {}, loc};
        }
        if (symbol == "ari_builtin_context_arg" || symbol == "ari_builtin_read_line") {
            return IrType{TypeQualifier::Value, IrPrimitiveKind::String, "string", {}, {}, {}, {}, loc};
        }
        if (symbol == "ari_builtin_panic" ||
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
        declarations_ << "declare ptr @malloc(i64)\n";
        declarations_ << "declare void @free(ptr)\n";
        declarations_ << "declare void @exit(i32)\n";
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
        std::string fmt_i64 = string_ptr("%lld");
        std::string fmt_bool = string_ptr("%d");
        std::string empty = string_ptr("");
        std::string line_buffer = "getelementptr inbounds ([4096 x i8], ptr @ari_line_buffer, i64 0, i64 0)";

        line("define void @ari_context_init(i32 %argc, ptr %argv) {");
        line("entry:");
        line("  store i32 %argc, ptr @ari_argc");
        line("  store ptr %argv, ptr @ari_argv");
        line("  ret void");
        line("}");
        line();

        line("define void @ari_context_shutdown() {");
        line("entry:");
        line("  ret void");
        line("}");
        line();

        line("define i64 @ari_builtin_context_argc() {");
        line("entry:");
        line("  %argc = load i32, ptr @ari_argc");
        line("  %wide = sext i32 %argc to i64");
        line("  ret i64 %wide");
        line("}");
        line();

        line("define ptr @ari_builtin_context_arg(i64 %index) {");
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

        line("define i64 @ari_builtin_write_i64(i64 %value) {");
        line("entry:");
        line("  call i32 (ptr, ...) @printf(ptr " + fmt_i64 + ", i64 %value)");
        line("  ret i64 0");
        line("}");
        line();

        line("define i64 @ari_builtin_write_bool(i1 %value) {");
        line("entry:");
        line("  %wide = zext i1 %value to i32");
        line("  call i32 (ptr, ...) @printf(ptr " + fmt_bool + ", i32 %wide)");
        line("  ret i64 0");
        line("}");
        line();

        line("define i64 @ari_builtin_write_byte(i8 %value) {");
        line("entry:");
        line("  %wide = zext i8 %value to i32");
        line("  call i32 @putchar(i32 %wide)");
        line("  ret i64 0");
        line("}");
        line();

        line("define i64 @ari_builtin_newline() {");
        line("entry:");
        line("  call i32 @putchar(i32 10)");
        line("  ret i64 0");
        line("}");
        line();

        line("define i64 @ari_builtin_read_byte() {");
        line("entry:");
        line("  %ch = call i32 @getchar()");
        line("  %wide = sext i32 %ch to i64");
        line("  ret i64 %wide");
        line("}");
        line();

        line("define ptr @ari_builtin_read_line() {");
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

        line("define ptr @ari_builtin_zone_create(i64 %capacity) {");
        line("entry:");
        line("  %bad.capacity = icmp sle i64 %capacity, 0");
        line("  br i1 %bad.capacity, label %fail, label %alloc.data");
        line("alloc.data:");
        line("  %data = call ptr @malloc(i64 %capacity)");
        line("  %data.null = icmp eq ptr %data, null");
        line("  br i1 %data.null, label %fail, label %alloc.zone");
        line("alloc.zone:");
        line("  %zone = call ptr @malloc(i64 24)");
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
        line("  ret ptr %zone");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define ptr @ari_builtin_zone_alloc(ptr %zone.slot, i64 %bytes, i64 %align) {");
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
        line("  %offset.slot = getelementptr i64, ptr %zone, i64 1");
        line("  %offset = load i64, ptr %offset.slot");
        line("  %biased = add i64 %offset, %mask");
        line("  %neg.align = sub i64 0, %align");
        line("  %aligned = and i64 %biased, %neg.align");
        line("  %new.offset = add i64 %aligned, %bytes");
        line("  %overflow = icmp ult i64 %new.offset, %aligned");
        line("  %too.big = icmp ugt i64 %new.offset, %capacity");
        line("  %bad.size = or i1 %overflow, %too.big");
        line("  br i1 %bad.size, label %fail, label %commit");
        line("commit:");
        line("  store i64 %new.offset, ptr %offset.slot");
        line("  %data.slot = getelementptr i8, ptr %zone, i64 16");
        line("  %data = load ptr, ptr %data.slot");
        line("  %out = getelementptr i8, ptr %data, i64 %aligned");
        line("  ret ptr %out");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define void @ari_builtin_zone_reset(ptr %zone.slot) {");
        line("entry:");
        line("  %zone = load ptr, ptr %zone.slot");
        line("  %zone.null = icmp eq ptr %zone, null");
        line("  br i1 %zone.null, label %fail, label %reset");
        line("reset:");
        line("  %offset.slot = getelementptr i64, ptr %zone, i64 1");
        line("  store i64 0, ptr %offset.slot");
        line("  ret void");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define void @ari_builtin_zone_destroy(ptr %zone) {");
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

        line("define i64 @ari_builtin_assert(i1 %condition) {");
        line("entry:");
        line("  br i1 %condition, label %ok, label %fail");
        line("ok:");
        line("  ret i64 0");
        line("fail:");
        line("  call void @exit(i32 1)");
        line("  unreachable");
        line("}");
        line();

        line("define i64 @ari_builtin_assert_eq_i64(i64 %left, i64 %right) {");
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

        line("define i64 @ari_builtin_assert_ne_i64(i64 %left, i64 %right) {");
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

        line("define i64 @ari_builtin_assert_eq_bool(i1 %left, i1 %right) {");
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

        line("define i64 @ari_builtin_assert_ne_bool(i1 %left, i1 %right) {");
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

        line("define void @ari_builtin_panic() {");
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

                if (method.result_type.primitive == IrPrimitiveKind::Void) {
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
        collect_expr_locals(expr.operand, locals);
        collect_expr_locals(expr.payload, locals);
        collect_expr_locals(expr.left, locals);
        collect_expr_locals(expr.right, locals);
        collect_expr_locals(expr.condition, locals);
        collect_locals(expr.then_body, locals);
        collect_expr_locals(expr.then_value, locals);
        collect_locals(expr.else_body, locals);
        collect_expr_locals(expr.else_value, locals);
        collect_locals(expr.block_body, locals);
        collect_expr_locals(expr.block_value, locals);
        collect_expr_locals(expr.match_value, locals);
        for (const auto& arg : expr.args) collect_expr_locals(arg, locals);
        for (const auto& arm : expr.match_arms) {
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
                collect_locals(stmt.statements, locals);
                break;
            case IrStmtKind::VarDecl:
                locals.push_back({stmt.binding.name, stmt.binding.type});
                collect_expr_locals(stmt.binding.init, locals);
                break;
            case IrStmtKind::If:
                collect_expr_locals(stmt.condition, locals);
                collect_locals(stmt.then_body, locals);
                collect_locals(stmt.else_body, locals);
                break;
            case IrStmtKind::While:
            case IrStmtKind::InitWhile:
            case IrStmtKind::WhileLet:
                for (const auto& binding : stmt.init_bindings) locals.push_back({binding.name, binding.type});
                for (const auto& binding : stmt.init_bindings) collect_expr_locals(binding.init, locals);
                collect_expr_locals(stmt.condition, locals);
                if (stmt.kind == IrStmtKind::WhileLet && !stmt.match_arms.empty() && stmt.match_arms[0].has_value_binding) {
                    locals.push_back({stmt.match_arms[0].value_name, stmt.match_arms[0].value_type});
                }
                if (stmt.kind == IrStmtKind::WhileLet && !stmt.match_arms.empty() && stmt.match_arms[0].has_payload_binding) {
                    collect_payload_binding_locals(stmt.match_arms[0], locals);
                }
                collect_locals(stmt.loop_body, locals);
                for (const auto& update : stmt.updates) collect_expr_locals(update, locals);
                break;
            case IrStmtKind::ForRange:
                locals.push_back({stmt.for_index_name, stmt.for_start->type});
                locals.push_back({stmt.for_end_name, stmt.for_end->type});
                collect_expr_locals(stmt.for_start, locals);
                collect_expr_locals(stmt.for_end, locals);
                if (!stmt.for_binding_name.empty()) locals.push_back({stmt.for_binding_name, stmt.for_binding_type});
                collect_locals(stmt.loop_body, locals);
                break;
            case IrStmtKind::ForVector:
                if (!stmt.for_binding_name.empty()) locals.push_back({stmt.for_binding_name, stmt.for_binding_type});
                for (const auto& value : stmt.for_values) collect_expr_locals(value, locals);
                collect_locals(stmt.loop_body, locals);
                break;
            case IrStmtKind::Match:
                collect_expr_locals(stmt.match_value, locals);
                for (const auto& arm : stmt.match_arms) {
                    if (arm.has_value_binding) locals.push_back({arm.value_name, arm.value_type});
                    collect_payload_binding_locals(arm, locals);
                    collect_locals(arm.body, locals);
                }
                break;
            case IrStmtKind::Assign:
                collect_expr_locals(stmt.rhs, locals);
                break;
            case IrStmtKind::ExprStmt:
            case IrStmtKind::Return:
                collect_expr_locals(stmt.expr, locals);
                break;
            case IrStmtKind::Continue:
                for (const auto& update : stmt.updates) collect_expr_locals(update, locals);
                break;
            case IrStmtKind::Break:
                collect_expr_locals(stmt.break_value, locals);
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

        line("define " + llvm_type(fn.return_type) + " " + quote_global(function_symbols_.at(fn.name)) + "(" + param_decl(fn.params) + ") {");
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
            if (fn.return_type.primitive == IrPrimitiveKind::Void) line("  ret void");
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
                Value value = emit_expr(*stmt.rhs);
                if (stmt.assign_target) {
                    std::string slot = emit_lvalue_ptr(*stmt.assign_target);
                    line("  store " + value.type + " " + value.name + ", ptr " + slot);
                } else {
                    line("  store " + value.type + " " + value.name + ", ptr " + local_slot(stmt.loc, stmt.assign_name));
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
                    if (stmt.break_value) {
                        if (!target.supports_break_value) {
                            throw CompileError(where(stmt.loc) + ": break value used with a non-value break target during LLVM lowering");
                        }
                        Value value = cast_value(emit_expr(*stmt.break_value), target.break_value_type);
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
        emit_statements(stmt.then_body);
        bool then_term = block_terminated_;
        if (!then_term) line("  br label %" + end_label);

        emit_label(else_label);
        emit_statements(stmt.else_body);
        bool else_term = block_terminated_;
        if (!else_term) line("  br label %" + end_label);

        if (then_term && else_term) {
            block_terminated_ = true;
            return;
        }
        emit_label(end_label);
    }

    void emit_block(const IrStmt& stmt) {
        if (stmt.label.empty()) {
            emit_statements(stmt.statements);
            return;
        }

        std::string end_label = label("block.end");
        loops_.push_back(make_loop_context(end_label, "", "", stmt.label, {}, false));
        emit_statements(stmt.statements);

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
        loops_.push_back(make_loop_context(end_label, cond_label, cond_label, stmt.label));
        emit_label(body_label);
        emit_statements(stmt.loop_body);
        if (!block_terminated_) line("  br label %" + cond_label);
        loops_.pop_back();
        emit_label(end_label);
    }

    void emit_while_let(const IrStmt& stmt) {
        if (stmt.match_arms.empty()) throw CompileError(where(stmt.loc) + ": while-let missing lowered pattern");
        std::string cond_label = label("whilelet.cond");
        std::string body_label = label("whilelet.body");
        std::string end_label = label("whilelet.end");
        std::vector<std::string> arm_labels;
        arm_labels.reserve(stmt.match_arms.size());
        for (std::size_t i = 0; i < stmt.match_arms.size(); ++i) {
            arm_labels.push_back(label("whilelet.arm"));
        }

        line("  br label %" + cond_label);
        emit_label(cond_label);
        Value value = emit_expr(*stmt.match_value);
        std::string subject = value.name;
        std::string subject_type = value.type;
        if (value.ir_type.primitive == IrPrimitiveKind::Enum) {
            Value tag = emit_enum_tag_value(stmt.loc, value);
            subject = tag.name;
            subject_type = tag.type;
        }

        for (std::size_t i = 0; i < stmt.match_arms.size(); ++i) {
            const IrMatchArm& arm = stmt.match_arms[i];
            std::string next = (i + 1 == stmt.match_arms.size()) ? end_label : label("whilelet.test");
            std::string cmp = emit_match_condition(arm, value, subject, subject_type);
            line("  br i1 " + cmp + ", label %" + arm_labels[i] + ", label %" + next);
            if (next != end_label) emit_label(next);
        }

        for (std::size_t i = 0; i < stmt.match_arms.size(); ++i) {
            const IrMatchArm& arm = stmt.match_arms[i];
            emit_label(arm_labels[i]);
            if (arm.has_value_binding) {
                Value bound = cast_value(value, arm.value_type);
                line("  store " + bound.type + " " + bound.name + ", ptr " + local_slot(arm.loc, arm.value_name));
            }
            emit_payload_bindings(arm, value);
            line("  br label %" + body_label);
        }

        loops_.push_back(make_loop_context(end_label, cond_label, cond_label, stmt.label));
        emit_label(body_label);
        emit_statements(stmt.loop_body);
        if (!block_terminated_) line("  br label %" + cond_label);
        loops_.pop_back();
        emit_label(end_label);
    }

    void emit_for_range(const IrStmt& stmt) {
        Value start = emit_expr(*stmt.for_start);
        Value end = emit_expr(*stmt.for_end);
        line("  store " + start.type + " " + start.name + ", ptr " + local_slot(stmt.loc, stmt.for_index_name));
        line("  store " + end.type + " " + end.name + ", ptr " + local_slot(stmt.loc, stmt.for_end_name));

        std::string cond_label = label("for.cond");
        std::string body_label = label("for.body");
        std::string step_label = label("for.step");
        std::string end_label = label("for.end");
        line("  br label %" + cond_label);
        emit_label(cond_label);
        Value index = load_local(stmt.loc, stmt.for_index_name, stmt.for_start->type);
        Value limit = load_local(stmt.loc, stmt.for_end_name, stmt.for_end->type);
        std::string cmp = temp();
        bool unsigned_range = is_unsigned_integer_type(stmt.for_start->type);
        std::string op = stmt.for_inclusive
            ? (unsigned_range ? "ule " : "sle ")
            : (unsigned_range ? "ult " : "slt ");
        line("  " + cmp + " = icmp " + op +
             index.type + " " + index.name + ", " + limit.name);
        line("  br i1 " + cmp + ", label %" + body_label + ", label %" + end_label);

        loops_.push_back(make_loop_context(end_label, step_label, cond_label, stmt.label));
        emit_label(body_label);
        if (!stmt.for_binding_name.empty()) {
            Value current = load_local(stmt.loc, stmt.for_index_name, stmt.for_binding_type);
            line("  store " + current.type + " " + current.name + ", ptr " + local_slot(stmt.loc, stmt.for_binding_name));
        }
        emit_statements(stmt.loop_body);
        if (!block_terminated_) line("  br label %" + step_label);

        emit_label(step_label);
        Value old = load_local(stmt.loc, stmt.for_index_name, stmt.for_start->type);
        if (stmt.for_inclusive) {
            Value step_limit = load_local(stmt.loc, stmt.for_end_name, stmt.for_end->type);
            std::string done = temp();
            std::string inc_label = label("for.inc");
            line("  " + done + " = icmp eq " + old.type + " " + old.name + ", " + step_limit.name);
            line("  br i1 " + done + ", label %" + end_label + ", label %" + inc_label);
            emit_label(inc_label);
        }
        std::string next = temp();
        line("  " + next + " = add " + old.type + " " + old.name + ", 1");
        line("  store " + old.type + " " + next + ", ptr " + local_slot(stmt.loc, stmt.for_index_name));
        line("  br label %" + cond_label);
        loops_.pop_back();
        emit_label(end_label);
    }

    void emit_for_vector(const IrStmt& stmt) {
        std::string end_label = label("forvec.end");
        loops_.push_back(make_loop_context(end_label, "", "", stmt.label));
        for (std::size_t i = 0; i < stmt.for_values.size(); ++i) {
            std::string next_label = i + 1 == stmt.for_values.size() ? end_label : label("forvec.next");
            loops_.back().plain_continue_label = next_label;
            loops_.back().value_continue_label = next_label;
            Value value = emit_expr(*stmt.for_values[i]);
            if (!stmt.for_binding_name.empty()) {
                line("  store " + value.type + " " + value.name + ", ptr " + local_slot(stmt.loc, stmt.for_binding_name));
            }
            emit_statements(stmt.loop_body);
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

        loops_.push_back(make_loop_context(end_label, update_label, cond_label, stmt.label, names));
        emit_label(body_label);
        emit_statements(stmt.loop_body);
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
        if (stmt.break_label.empty()) {
            for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
                if (loop->is_loop) return *loop;
            }
            throw CompileError(where(stmt.loc) + ": break outside loop during LLVM lowering");
        }
        for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
            if (loop->source_label == stmt.break_label) return *loop;
        }
        throw CompileError(where(stmt.loc) + ": unknown loop label '" + stmt.break_label + "' during LLVM lowering");
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
        for (std::size_t i = 0; i < stmt.match_arms.size(); ++i) arm_labels.push_back(label("match.arm"));

        std::string first_test = label("match.test");
        line("  br label %" + first_test);
        emit_label(first_test);
        for (std::size_t i = 0; i < stmt.match_arms.size(); ++i) {
            const IrMatchArm& arm = stmt.match_arms[i];
            if (arm.wildcard) {
                line("  br label %" + arm_labels[i]);
            } else {
                std::string next = (i + 1 == stmt.match_arms.size()) ? arm_labels[i] : label("match.test");
                std::string cmp = emit_match_condition(arm, value, subject, subject_type);
                line("  br i1 " + cmp + ", label %" + arm_labels[i] + ", label %" + next);
                if (next != arm_labels[i]) emit_label(next);
            }
        }

        for (std::size_t i = 0; i < stmt.match_arms.size(); ++i) {
            const IrMatchArm& arm = stmt.match_arms[i];
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
                return Value{llvm_type(expr.type), std::to_string(expr.float_value), expr.type};
            case IrExprKind::String:
                return Value{"ptr", string_ptr(expr.string_value), expr.type};
            case IrExprKind::Null:
                return Value{"ptr", "null", expr.type};
            case IrExprKind::FunctionRef:
                return emit_function_ref(expr);
            case IrExprKind::Local:
                return load_local(expr.loc, expr.name, expr.type);
            case IrExprKind::Borrow:
                if (expr.operand) return Value{"ptr", emit_lvalue_ptr(*expr.operand), expr.type};
                return Value{"ptr", local_slot(expr.loc, expr.name), expr.type};
            case IrExprKind::Unary: {
                Value operand = emit_expr(*expr.operand);
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
                return cast_value(emit_expr(*expr.operand), expr.type);
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
            case IrExprKind::EnumConstruct:
                return emit_enum_construct(expr);
            case IrExprKind::Tuple:
                return emit_tuple(expr);
            case IrExprKind::TupleIndex:
                return emit_tuple_index(expr);
            case IrExprKind::Index:
                if (expr.operand && expr.operand->type.primitive == IrPrimitiveKind::Array) {
                    return emit_array_index(expr);
                }
                if (expr.operand && expr.operand->type.primitive == IrPrimitiveKind::Vector) {
                    return emit_vector_index(expr);
                }
                if (expr.operand && is_prelude_slice_type(expr.operand->type)) {
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

    std::string emit_lvalue_ptr(const IrExpr& expr) {
        if (expr.kind == IrExprKind::Local) return local_slot(expr.loc, expr.name);
        if (expr.kind == IrExprKind::PointerLoad && expr.operand) {
            return emit_expr(*expr.operand).name;
        }
        if (expr.kind == IrExprKind::TupleIndex && expr.operand) {
            std::string base = emit_lvalue_ptr(*expr.operand);
            std::string ptr = temp();
            line("  " + ptr + " = getelementptr inbounds " + llvm_type(expr.operand->type) +
                 ", ptr " + base + ", i32 0, i32 " + std::to_string(expr.tuple_index));
            return ptr;
        }
        if (expr.kind == IrExprKind::Index && expr.operand && expr.right) {
            if (is_prelude_slice_type(expr.operand->type)) {
                return emit_slice_element_ptr(expr);
            }
            if (expr.operand->type.primitive != IrPrimitiveKind::Array &&
                expr.operand->type.primitive != IrPrimitiveKind::Vector) {
                throw CompileError(where(expr.loc) + ": LLVM backend can only index array, vector, or Slice lvalues");
            }
            std::string base = emit_lvalue_ptr(*expr.operand);
            IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
            Value index = cast_value(emit_expr(*expr.right), index_type);
            std::string ptr = temp();
            if (expr.operand->type.primitive == IrPrimitiveKind::Array) {
                emit_array_bounds_check(index, expr.operand->type.array_size);
                line("  " + ptr + " = getelementptr inbounds " + llvm_type(expr.operand->type) +
                     ", ptr " + base + ", i64 0, i64 " + index.name);
            } else {
                emit_vector_bounds_check(index, expr.operand->type, base);
                line("  " + ptr + " = getelementptr inbounds " + llvm_type(expr.operand->type) +
                     ", ptr " + base + ", i32 0, i32 1, i64 " + index.name);
            }
            return ptr;
        }
        throw CompileError(where(expr.loc) + ": LLVM backend can only assign to local aggregate fields yet");
    }

    static bool is_pointer_backed_lvalue(const IrExpr& expr) {
        if (expr.kind == IrExprKind::PointerLoad) return true;
        if ((expr.kind == IrExprKind::TupleIndex ||
             expr.kind == IrExprKind::Index) &&
            expr.operand) {
            return is_pointer_backed_lvalue(*expr.operand);
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
        Value tuple = emit_expr(*expr.operand);
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
        if (!expr.operand || expr.operand->kind != IrExprKind::Local) {
            throw CompileError(where(expr.loc) + ": LLVM backend can only dynamically index local vectors yet");
        }
        if (expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": LLVM backend cannot index unsized Vec storage");
        }
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*expr.right), index_type);
        std::string base = local_slot(expr.operand->loc, expr.operand->name);
        emit_vector_bounds_check(index, expr.operand->type, base);
        std::string ptr = temp();
        line("  " + ptr + " = getelementptr inbounds " + llvm_type(expr.operand->type) +
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
        if (!expr.operand || !expr.left || !expr.right || !is_prelude_slice_type(expr.operand->type)) {
            throw CompileError(where(expr.loc) + ": malformed Slice range during LLVM lowering");
        }
        if (expr.type.args.empty()) {
            throw CompileError(where(expr.loc) + ": malformed Slice range result type");
        }

        Value slice = emit_expr(*expr.operand);
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value start = cast_value(emit_expr(*expr.left), index_type);
        Value end = cast_value(emit_expr(*expr.right), index_type);

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
        if (!expr.operand || !expr.right || !is_prelude_slice_type(expr.operand->type)) {
            throw CompileError(where(expr.loc) + ": malformed Slice index during LLVM lowering");
        }
        Value slice = emit_expr(*expr.operand);
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*expr.right), index_type);
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
        if (!expr.operand || !expr.right || expr.operand->type.primitive != IrPrimitiveKind::Vector ||
            expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.push lowering");
        }
        const IrType& vector_type = expr.operand->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*expr.operand);
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

        Value value = cast_value(emit_expr(*expr.right), element_type);
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
        if (!expr.operand || expr.operand->type.primitive != IrPrimitiveKind::Vector ||
            expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.pop lowering");
        }
        const IrType& vector_type = expr.operand->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*expr.operand);
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

    Value emit_vector_clear(const IrExpr& expr) {
        if (!expr.operand || expr.operand->type.primitive != IrPrimitiveKind::Vector ||
            expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.clear lowering");
        }
        const IrType& vector_type = expr.operand->type;
        std::string base = emit_lvalue_ptr(*expr.operand);
        std::string len_ptr = temp();
        line("  " + len_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 0");
        line("  store i64 0, ptr " + len_ptr);
        return Value{"void", "", expr.type};
    }

    Value emit_vector_truncate(const IrExpr& expr) {
        if (!expr.operand || !expr.right || expr.operand->type.primitive != IrPrimitiveKind::Vector ||
            expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.truncate lowering");
        }
        const IrType& vector_type = expr.operand->type;
        std::string base = emit_lvalue_ptr(*expr.operand);
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value new_len = cast_value(emit_expr(*expr.right), index_type);

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
        if (!expr.operand || !expr.right || !expr.payload ||
            expr.operand->type.primitive != IrPrimitiveKind::Vector ||
            expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.set lowering");
        }
        const IrType& vector_type = expr.operand->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*expr.operand);
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*expr.right), index_type);
        emit_vector_bounds_check(index, vector_type, base);

        Value value = cast_value(emit_expr(*expr.payload), element_type);
        std::string item_ptr = temp();
        line("  " + item_ptr + " = getelementptr inbounds " + llvm_type(vector_type) +
             ", ptr " + base + ", i32 0, i32 1, i64 " + index.name);
        line("  store " + value.type + " " + value.name + ", ptr " + item_ptr);
        return Value{"void", "", expr.type};
    }

    Value emit_vector_swap(const IrExpr& expr) {
        if (!expr.operand || !expr.right || !expr.payload ||
            expr.operand->type.primitive != IrPrimitiveKind::Vector ||
            expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.swap lowering");
        }
        const IrType& vector_type = expr.operand->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*expr.operand);
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value first_index = cast_value(emit_expr(*expr.right), index_type);
        emit_vector_bounds_check(first_index, vector_type, base);
        Value second_index = cast_value(emit_expr(*expr.payload), index_type);
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
        if (!expr.operand || !expr.right ||
            expr.operand->type.primitive != IrPrimitiveKind::Vector ||
            expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.remove lowering");
        }
        const IrType& vector_type = expr.operand->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*expr.operand);
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*expr.right), index_type);
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
        if (!expr.operand || !expr.right || !expr.payload ||
            expr.operand->type.primitive != IrPrimitiveKind::Vector ||
            expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.insert lowering");
        }
        const IrType& vector_type = expr.operand->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*expr.operand);
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*expr.right), index_type);

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

        Value value = cast_value(emit_expr(*expr.payload), element_type);
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
        if (!expr.operand || !expr.payload ||
            expr.operand->type.primitive != IrPrimitiveKind::Vector ||
            expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec search lowering");
        }
        const IrType& vector_type = expr.operand->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*expr.operand);
        Value needle = cast_value(emit_expr(*expr.payload), element_type);
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
        if (!expr.operand || !expr.payload ||
            expr.operand->type.primitive != IrPrimitiveKind::Vector ||
            expr.operand->type.args.size() != 1) {
            throw CompileError(where(expr.loc) + ": malformed Vec.count lowering");
        }
        const IrType& vector_type = expr.operand->type;
        const IrType& element_type = vector_type.args[0];
        std::string base = emit_lvalue_ptr(*expr.operand);
        Value needle = cast_value(emit_expr(*expr.payload), element_type);
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

    Value emit_compact_enum_payload_value(SourceLocation loc,
                                          const Value& value,
                                          std::uint32_t index,
                                          const IrType& enum_type) {
        Value payload = emit_enum_payload_slot(loc, value, index);
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
                          0
                      )
                    : emit_enum_payload_slot(arm.loc, enum_value, binding.index);
                payload = cast_value(payload, binding.type);
                line("  store " + payload.type + " " + payload.name + ", ptr " + local_slot(arm.loc, binding.name));
            }
            return;
        }
        if (!arm.has_payload_binding) return;
        Value payload = emit_enum_payload_slot(arm.loc, enum_value, arm.payload_index);
        payload = cast_value(payload, arm.payload_type);
        line("  store " + payload.type + " " + payload.name + ", ptr " + local_slot(arm.loc, arm.payload_name));
    }

    Value emit_match_expr(const IrExpr& expr) {
        if (expr.match_arms.empty()) throw CompileError(where(expr.loc) + ": match expression has no arms during LLVM lowering");
        Value value = emit_expr(*expr.match_value);
        std::string subject = value.name;
        std::string subject_type = value.type;
        if (value.ir_type.primitive == IrPrimitiveKind::Enum) {
            Value tag = emit_enum_tag_value(expr.loc, value);
            subject = tag.name;
            subject_type = tag.type;
        }
        std::string end_label = label("match.expr.end");
        std::vector<std::string> arm_labels;
        for (std::size_t i = 0; i < expr.match_arms.size(); ++i) arm_labels.push_back(label("match.expr.arm"));

        std::string first_test = label("match.expr.test");
        line("  br label %" + first_test);
        emit_label(first_test);
        for (std::size_t i = 0; i < expr.match_arms.size(); ++i) {
            const IrMatchExprArm& arm = expr.match_arms[i];
            if (arm.wildcard || i + 1 == expr.match_arms.size()) {
                line("  br label %" + arm_labels[i]);
            } else {
                std::string next = label("match.expr.test");
                std::string cmp = emit_match_condition(arm, value, subject, subject_type);
                line("  br i1 " + cmp + ", label %" + arm_labels[i] + ", label %" + next);
                emit_label(next);
            }
        }

        std::vector<std::pair<Value, std::string>> incoming;
        for (std::size_t i = 0; i < expr.match_arms.size(); ++i) {
            const IrMatchExprArm& arm = expr.match_arms[i];
            emit_label(arm_labels[i]);
            if (arm.has_value_binding) {
                Value bound = cast_value(value, arm.value_type);
                line("  store " + bound.type + " " + bound.name + ", ptr " + local_slot(arm.loc, arm.value_name));
            }
            emit_payload_bindings(arm, value);
            emit_statements(arm.body);
            Value arm_value = cast_value(emit_expr(*arm.value), expr.type);
            std::string incoming_label = current_label_;
            line("  br label %" + end_label);
            incoming.push_back({arm_value, incoming_label});
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
            std::string payload_cmp = temp();
            std::string both = temp();
            line("  " + payload_cmp + " = icmp eq " + payload.type + " " + payload.name + ", " +
                 match_payload_literal_constant(payload_condition));
            line("  " + both + " = and i1 " + condition + ", " + payload_cmp);
            condition = both;
        }
        for (const auto& payload_condition : arm.payload_range_conditions) {
            Value payload = emit_enum_payload_slot(arm.loc, value, payload_condition.index);
            payload = cast_value(payload, payload_condition.type);
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
                Value nested_payload = emit_enum_payload_slot(arm.loc, nested, 0);
                nested_payload = cast_value(nested_payload, payload_condition.payload_type);
                std::string payload_cmp = temp();
                std::string both = temp();
                line("  " + payload_cmp + " = icmp eq " + nested_payload.type + " " + nested_payload.name + ", " +
                     nested_payload_literal_constant(payload_condition));
                line("  " + both + " = and i1 " + condition + ", " + payload_cmp);
                condition = both;
            }

            if (payload_condition.has_payload_range) {
                Value nested_payload = emit_enum_payload_slot(arm.loc, nested, 0);
                nested_payload = cast_value(nested_payload, payload_condition.payload_type);
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
        if (condition.is_bool) return condition.bool_value ? "1" : "0";
        return std::to_string(condition.value);
    }

    static std::string match_payload_range_start_constant(const IrPayloadRangeCondition& condition) {
        return (condition.start_negative ? "-" : "") + std::to_string(condition.start_int);
    }

    static std::string match_payload_range_end_constant(const IrPayloadRangeCondition& condition) {
        return (condition.end_negative ? "-" : "") + std::to_string(condition.end_int);
    }

    static std::string nested_payload_literal_constant(const IrPayloadEnumCondition& condition) {
        if (condition.payload_literal_is_bool) return condition.payload_literal_bool ? "1" : "0";
        return (condition.payload_literal_negative ? "-" : "") + std::to_string(condition.payload_literal_int);
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
        Value cond = emit_expr(*expr.condition);
        std::string then_label = label("if.expr.then");
        std::string else_label = label("if.expr.else");
        std::string end_label = label("if.expr.end");
        line("  br i1 " + cond.name + ", label %" + then_label + ", label %" + else_label);

        emit_label(then_label);
        emit_statements(expr.then_body);
        Value then_value = cast_value(emit_expr(*expr.then_value), expr.type);
        std::string then_incoming = current_label_;
        line("  br label %" + end_label);

        emit_label(else_label);
        emit_statements(expr.else_body);
        Value else_value = cast_value(emit_expr(*expr.else_value), expr.type);
        std::string else_incoming = current_label_;
        line("  br label %" + end_label);

        emit_label(end_label);
        std::string out = temp();
        std::string type = llvm_type(expr.type);
        line("  " + out + " = phi " + type + " [" + then_value.name + ", %" + then_incoming +
             "], [" + else_value.name + ", %" + else_incoming + "]");
        return Value{type, out, expr.type};
    }

    Value emit_block_expr(const IrExpr& expr) {
        if (!expr.label.empty()) {
            std::string type = llvm_type(expr.type);
            std::string slot = temp();
            std::string end_label = label("block.expr.end");
            line("  " + slot + " = alloca " + type);

            LoopContext context;
            context.break_label = end_label;
            context.source_label = expr.label;
            context.is_loop = false;
            context.supports_break_value = true;
            context.break_value_slot = slot;
            context.break_value_type = expr.type;
            loops_.push_back(context);

            emit_statements(expr.block_body);
            bool body_terminated = block_terminated_;
            bool has_break = loops_.back().has_break;
            loops_.pop_back();

            if (!body_terminated) {
                Value value = cast_value(emit_expr(*expr.block_value), expr.type);
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
        emit_statements(expr.block_body);
        return emit_expr(*expr.block_value);
    }

    Value emit_trait_object_cast(const IrExpr& expr) {
        Value source = emit_expr(*expr.operand);
        std::string source_slot = temp();
        line("  " + source_slot + " = alloca " + source.type);
        line("  store " + source.type + " " + source.name + ", ptr " + source_slot);

        std::string object_type = llvm_type(expr.type);
        std::string with_data = temp();
        line("  " + with_data + " = insertvalue " + object_type + " undef, ptr " + source_slot + ", 0");
        std::string with_vtable = temp();
        line("  " + with_vtable + " = insertvalue " + object_type + " " + with_data +
             ", ptr " + quote_global(expr.name) + ", 1");
        return Value{object_type, with_vtable, expr.type};
    }

    Value emit_trait_object_call(const IrExpr& expr) {
        if (expr.call_param_types.empty()) {
            throw CompileError(where(expr.loc) + ": malformed trait object call");
        }

        Value object = emit_expr(*expr.operand);
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
        args.push_back(Value{"ptr", data, expr.call_param_types[0]});
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            Value arg = emit_expr(*expr.args[i]);
            if (i + 1 < expr.call_param_types.size()) arg = cast_value(arg, expr.call_param_types[i + 1]);
            args.push_back(arg);
        }

        std::string result_type = llvm_type(expr.type);
        std::string call = "call " + result_type + " " + callee + "(";
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (i > 0) call += ", ";
            call += args[i].type + " " + args[i].name;
        }
        call += ")";

        if (expr.type.primitive == IrPrimitiveKind::Void) {
            line("  " + call);
            return Value{"void", "", expr.type};
        }
        std::string out = temp();
        line("  " + out + " = " + call);
        return Value{result_type, out, expr.type};
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
        Value pointer = cast_value(emit_expr(*expr.operand), expr.type);
        IrType offset_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value offset = cast_value(emit_expr(*expr.right), offset_type);
        std::string out = temp();
        line("  " + out + " = getelementptr i8, ptr " + pointer.name + ", i64 " + offset.name);
        return Value{"ptr", out, expr.type};
    }

    Value emit_pointer_add(const IrExpr& expr) {
        Value pointer = cast_value(emit_expr(*expr.operand), expr.type);
        IrType index_type{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc};
        Value index = cast_value(emit_expr(*expr.right), index_type);
        IrType element_type = expr.type;
        element_type.qualifier = TypeQualifier::Value;
        std::string out = temp();
        line("  " + out + " = getelementptr " + llvm_type(element_type) +
             ", ptr " + pointer.name + ", i64 " + index.name);
        return Value{"ptr", out, expr.type};
    }

    Value emit_pointer_load(const IrExpr& expr) {
        Value pointer = emit_expr(*expr.operand);
        std::string out = temp();
        std::string element_type = llvm_type(expr.type);
        line("  " + out + " = load " + element_type + ", ptr " + pointer.name);
        return Value{element_type, out, expr.type};
    }

    Value emit_pointer_store(const IrExpr& expr) {
        if (!expr.right) {
            throw CompileError(where(expr.loc) + ": malformed pointer store expression");
        }
        Value pointer = emit_expr(*expr.operand);
        Value value = cast_value(emit_expr(*expr.right), expr.right->type);
        line("  store " + value.type + " " + value.name + ", ptr " + pointer.name);
        return Value{llvm_type(expr.type), "", expr.type};
    }

    Value emit_enum_construct(const IrExpr& expr) {
        if (has_aggregate_enum_layout(expr.type)) {
            std::string enum_type = llvm_type(expr.type);
            std::string value = "zeroinitializer";
            std::string with_tag = temp();
            line("  " + with_tag + " = insertvalue " + enum_type + " " + value +
                 ", i32 " + std::to_string(expr.enum_tag) + ", 0");
            value = with_tag;
            IrType slot_type = enum_payload_storage_type(expr.loc);
            for (std::size_t i = 0; i < expr.args.size(); ++i) {
                Value payload = cast_value(emit_expr(*expr.args[i]), slot_type);
                std::string next = temp();
                line("  " + next + " = insertvalue " + enum_type + " " + value +
                     ", " + payload.type + " " + payload.name + ", " + std::to_string(i + 1));
                value = next;
            }
            return Value{enum_type, value, expr.type};
        }
        if (!expr.has_payload) return Value{"i64", std::to_string(expr.enum_tag), expr.type};
        Value payload = cast_value(emit_expr(*expr.payload), expr.payload_type);
        payload = cast_value(payload, IrType{TypeQualifier::Value, IrPrimitiveKind::U64, "u64", {}, {}, {}, {}, expr.loc});
        std::string shifted = temp();
        line("  " + shifted + " = shl i64 " + payload.name + ", 32");
        std::string out = temp();
        line("  " + out + " = or i64 " + shifted + ", " + std::to_string(expr.enum_tag));
        return Value{"i64", out, expr.type};
    }

    Value emit_try_residual_return_value(const IrExpr& expr, const Value& value) {
        if (!expr.try_converts_residual) return cast_value(value, current_return_);
        if (!expr.try_residual_has_payload) {
            return Value{llvm_type(current_return_), std::to_string(expr.try_return_residual_tag), current_return_};
        }

        std::string shifted_down = temp();
        line("  " + shifted_down + " = lshr " + value.type + " " + value.name + ", 32");
        Value payload{
            value.type,
            shifted_down,
            IrType{TypeQualifier::Value, IrPrimitiveKind::U64, "u64", {}, {}, {}, {}, expr.loc}
        };
        payload = cast_value(payload, expr.try_return_residual_payload_type);
        payload = cast_value(payload, IrType{TypeQualifier::Value, IrPrimitiveKind::U64, "u64", {}, {}, {}, {}, expr.loc});
        std::string shifted_up = temp();
        line("  " + shifted_up + " = shl i64 " + payload.name + ", 32");
        std::string out = temp();
        line("  " + out + " = or i64 " + shifted_up + ", " + std::to_string(expr.try_return_residual_tag));
        return Value{llvm_type(current_return_), out, current_return_};
    }

    Value emit_try(const IrExpr& expr) {
        Value value = emit_expr(*expr.operand);
        Value tag = emit_enum_tag_value(expr.loc, value);
        std::string ok = label("try.ok");
        std::string fail = label("try.return");
        std::string cont = label("try.cont");
        std::string cmp = temp();
        line("  " + cmp + " = icmp eq i32 " + tag.name + ", " + std::to_string(expr.enum_tag));
        line("  br i1 " + cmp + ", label %" + ok + ", label %" + fail);

        emit_label(fail);
        Value residual = emit_try_residual_return_value(expr, value);
        line("  ret " + residual.type + " " + residual.name);

        emit_label(ok);
        Value payload = emit_enum_payload_slot(expr.loc, value, 0);
        payload = cast_value(payload, expr.payload_type);
        line("  br label %" + cont);

        emit_label(cont);
        std::string out = temp();
        line("  " + out + " = phi " + payload.type + " [" + payload.name + ", %" + ok + "]");
        return Value{payload.type, out, expr.type};
    }

    Value emit_null_coalesce(const IrExpr& expr) {
        Value value = emit_expr(*expr.left);
        Value tag = emit_enum_tag_value(expr.loc, value);
        std::string ok = label("coalesce.ok");
        std::string fallback = label("coalesce.fallback");
        std::string end = label("coalesce.end");
        std::string cmp = temp();
        line("  " + cmp + " = icmp eq i32 " + tag.name + ", " + std::to_string(expr.enum_tag));
        line("  br i1 " + cmp + ", label %" + ok + ", label %" + fallback);

        emit_label(ok);
        Value payload = emit_enum_payload_slot(expr.loc, value, 0);
        payload = cast_value(payload, expr.payload_type);
        std::string ok_label = current_label_;
        line("  br label %" + end);

        emit_label(fallback);
        Value fallback_value = cast_value(emit_expr(*expr.right), expr.type);
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
        std::string fmt_string = string_ptr("%s");
        for (std::size_t i = 0; i < expr.format_parts.size(); ++i) {
            if (!expr.format_parts[i].empty()) {
                line("  call i32 (ptr, ...) @printf(ptr " + fmt_string + ", ptr " + string_ptr(expr.format_parts[i]) + ")");
            }
            if (i < expr.args.size()) {
                Value arg = emit_expr(*expr.args[i]);
                if (arg.ir_type.primitive == IrPrimitiveKind::Bool) {
                    line("  call i64 @ari_builtin_write_bool(i1 " + arg.name + ")");
                } else if (arg.ir_type.primitive == IrPrimitiveKind::U8 || arg.ir_type.primitive == IrPrimitiveKind::I8) {
                    Value byte = cast_value(arg, IrType{TypeQualifier::Value, IrPrimitiveKind::U8, "u8", {}, {}, {}, {}, expr.loc});
                    line("  call i64 @ari_builtin_write_byte(i8 " + byte.name + ")");
                } else {
                    Value wide = cast_value(arg, IrType{TypeQualifier::Value, IrPrimitiveKind::I64, "i64", {}, {}, {}, {}, expr.loc});
                    line("  call i64 @ari_builtin_write_i64(i64 " + wide.name + ")");
                }
            }
        }
        if (expr.print_newline) line("  call i64 @ari_builtin_newline()");
        return Value{"i64", "0", expr.type};
    }

    Value emit_function_ref(const IrExpr& expr) {
        if (extern_symbols_.count(expr.name)) {
            return Value{"ptr", quote_global(extern_symbols_.at(expr.name)), expr.type};
        }
        auto found = function_symbols_.find(expr.name);
        if (found == function_symbols_.end()) {
            throw CompileError(where(expr.loc) + ": LLVM backend cannot find function '" + expr.name + "'");
        }
        return Value{"ptr", quote_global(found->second), expr.type};
    }

    Value emit_indirect_call(const IrExpr& expr) {
        Value callee = emit_expr(*expr.operand);
        const IrType& callee_type = expr.operand->type;
        if (callee_type.primitive != IrPrimitiveKind::Function ||
            callee_type.args.empty() ||
            callee_type.array_size + 1 != callee_type.args.size()) {
            throw CompileError(where(expr.loc) + ": LLVM backend cannot call non-function value");
        }
        std::size_t param_count = static_cast<std::size_t>(callee_type.array_size);
        IrType result = callee_type.args[param_count];

        std::vector<Value> args;
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            Value arg = emit_expr(*expr.args[i]);
            if (i < param_count) arg = cast_value(arg, callee_type.args[i]);
            args.push_back(arg);
        }

        std::string result_type = llvm_type(result);
        std::string call = "call " + result_type + " " + callee.name + "(";
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (i > 0) call += ", ";
            call += args[i].type + " " + args[i].name;
        }
        call += ")";
        if (result.primitive == IrPrimitiveKind::Void) {
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
        if (expr.name == "zone::new") return emit_zone_new(expr);

        std::string symbol;
        IrType result;
        std::vector<IrParam> params;
        if (extern_symbols_.count(expr.name)) {
            symbol = extern_symbols_.at(expr.name);
            result = extern_results_.at(expr.name);
            params = extern_params_.at(expr.name);
        } else if (function_symbols_.count(expr.name)) {
            symbol = function_symbols_.at(expr.name);
            result = function_results_.at(expr.name);
            params = function_params_.at(expr.name);
        } else if (std::optional<std::string> builtin_symbol = ari_builtin_symbol_for_source_name(expr.name)) {
            symbol = *builtin_symbol;
            result = symbol == "ari_builtin_zone_alloc" ? expr.type : builtin_result_type(symbol, expr.loc);
        } else {
            symbol = function_symbols_.at(expr.name);
            result = function_results_.at(expr.name);
            params = function_params_.at(expr.name);
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
        if (result.primitive == IrPrimitiveKind::Void) {
            line("  " + call);
            return Value{"void", "", result};
        }
        std::string out = temp();
        line("  " + out + " = " + call);
        return Value{result_type, out, result};
    }

    Value emit_binary(const IrExpr& expr) {
        Value left = emit_expr(*expr.left);
        Value right = cast_value(emit_expr(*expr.right), left.ir_type);
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

std::string emit_llvm_ir(const IrProgram& program) {
    LlvmEmitter emitter(program);
    return emitter.emit();
}

} // namespace ari
