#include "codegen.hpp"

#include "ari_builtin.hpp"
#include "layout.hpp"
#include "symbol_mangle.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace ari {

class CodeBuffer {
public:
    std::vector<std::uint8_t> bytes;

    std::size_t size() const { return bytes.size(); }

    void u8(std::uint8_t value) { bytes.push_back(value); }

    void u32(std::uint32_t value) {
        for (int i = 0; i < 4; ++i) u8(static_cast<std::uint8_t>((value >> (i * 8)) & 0xff));
    }

    void u64(std::uint64_t value) {
        for (int i = 0; i < 8; ++i) u8(static_cast<std::uint8_t>((value >> (i * 8)) & 0xff));
    }

    void patch32(std::size_t at, std::int32_t value) {
        std::uint32_t bits = static_cast<std::uint32_t>(value);
        for (int i = 0; i < 4; ++i) bytes[at + static_cast<std::size_t>(i)] = static_cast<std::uint8_t>((bits >> (i * 8)) & 0xff);
    }
};

enum class Reg : std::uint8_t {
    RAX = 0,
    RCX = 1,
    RDX = 2,
    RBX = 3,
    RSP = 4,
    RBP = 5,
    RSI = 6,
    RDI = 7,
    R8 = 8,
    R9 = 9
};

static int reg_code(Reg reg) {
    return static_cast<int>(reg);
}

struct CallPatch {
    std::size_t imm_offset = 0;
    std::string name;
};

struct CompiledFunction {
    std::string name;
    std::vector<std::uint8_t> code;
    std::vector<CallPatch> calls;
    std::vector<CallPatch> addresses;
};

class FunctionEmitter {
public:
    FunctionEmitter(const IrFunction& fn, const std::map<std::string, IrExternAbi>& extern_abis)
        : fn_(fn), extern_abis_(extern_abis) {}

    CompiledFunction emit() {
        for (const auto& param : fn_.params) add_local(param.name, param.type);
        collect_locals(fn_.body);
        stack_size_ = align_to(stack_offset_, 16);

        out_.u8(0x55);
        emit_mov_reg_reg(Reg::RBP, Reg::RSP);
        if (stack_size_ > 0) emit_sub_rsp(stack_size_);
        store_params();
        emit_statements(fn_.body);

        std::size_t epilogue = out_.size();
        out_.u8(0xC9);
        out_.u8(0xC3);
        for (std::size_t patch : return_jumps_) patch_rel32(patch, epilogue);

        CompiledFunction compiled;
        compiled.name = fn_.name;
        compiled.code = std::move(out_.bytes);
        compiled.calls = std::move(calls_);
        compiled.addresses = std::move(addresses_);
        return compiled;
    }

private:
    struct LoopLabels {
        std::vector<std::size_t> break_patches;
        std::vector<std::size_t> plain_continue_patches;
        bool plain_continue_known = false;
        std::size_t plain_continue_target = 0;
        std::size_t value_continue_target = 0;
        std::string source_label;
        std::vector<std::string> update_names;
        bool supports_break_value = false;
        bool has_break_value_target = false;
        int break_value_target_offset = 0;
        IrType break_value_type;
        bool is_loop = true;
    };

    const IrFunction& fn_;
    const std::map<std::string, IrExternAbi>& extern_abis_;
    CodeBuffer out_;
    std::map<std::string, int> locals_;
    std::map<std::string, IrType> local_types_;
    int stack_offset_ = 0;
    int stack_size_ = 0;
    std::vector<std::size_t> return_jumps_;
    std::vector<CallPatch> calls_;
    std::vector<CallPatch> addresses_;
    std::vector<LoopLabels> loops_;

    static int align_to(int value, int alignment) {
        return (value + alignment - 1) / alignment * alignment;
    }

    static int local_slot_count(const IrType& type) {
        if (!is_aggregate_type(type)) return 1;
        int count = 0;
        const std::vector<IrType>& fields = aggregate_field_types(type);
        for (const auto& item : fields) count += local_slot_count(item);
        return count;
    }

    static int local_size_bytes(const IrType& type) {
        std::uint64_t size = 0;
        if (ari_layout_size_bytes(type, size) &&
            size <= static_cast<std::uint64_t>(std::numeric_limits<int>::max())) {
            return static_cast<int>(size);
        }
        return local_slot_count(type) * 8;
    }

    static int local_align_bytes(const IrType& type) {
        std::uint64_t align = 0;
        if (ari_layout_align_bytes(type, align) &&
            align > 0 &&
            align <= static_cast<std::uint64_t>(std::numeric_limits<int>::max())) {
            return static_cast<int>(align);
        }
        return 8;
    }

    static bool is_aggregate_type(const IrType& type) {
        return type.qualifier == TypeQualifier::Value &&
               (type.primitive == IrPrimitiveKind::Tuple ||
                type.primitive == IrPrimitiveKind::Array ||
                type.primitive == IrPrimitiveKind::Struct ||
                has_aggregate_enum_layout(type));
    }

    static const std::vector<IrType>& aggregate_field_types(const IrType& type) {
        if (type.primitive == IrPrimitiveKind::Struct ||
            type.primitive == IrPrimitiveKind::Array ||
            has_aggregate_enum_layout(type)) return type.field_types;
        return type.args;
    }

    static bool has_aggregate_enum_layout(const IrType& type) {
        return type.qualifier == TypeQualifier::Value &&
               type.primitive == IrPrimitiveKind::Enum &&
               !type.field_types.empty();
    }

    static bool is_signed_integer_primitive(IrPrimitiveKind primitive) {
        return primitive == IrPrimitiveKind::I8 ||
               primitive == IrPrimitiveKind::I16 ||
               primitive == IrPrimitiveKind::I32 ||
               primitive == IrPrimitiveKind::I64;
    }

    static bool is_integer_primitive(IrPrimitiveKind primitive) {
        return primitive == IrPrimitiveKind::I8 ||
               primitive == IrPrimitiveKind::I16 ||
               primitive == IrPrimitiveKind::I32 ||
               primitive == IrPrimitiveKind::I64 ||
               primitive == IrPrimitiveKind::U8 ||
               primitive == IrPrimitiveKind::U16 ||
               primitive == IrPrimitiveKind::U32 ||
               primitive == IrPrimitiveKind::U64;
    }

    static unsigned raw_memory_bits(const IrType& type) {
        if (type.qualifier != TypeQualifier::Value) return 64;
        switch (type.primitive) {
            case IrPrimitiveKind::I8:
            case IrPrimitiveKind::U8:
            case IrPrimitiveKind::Bool:
                return 8;
            case IrPrimitiveKind::I16:
            case IrPrimitiveKind::U16:
                return 16;
            case IrPrimitiveKind::I32:
            case IrPrimitiveKind::U32:
            case IrPrimitiveKind::F32:
                return 32;
            case IrPrimitiveKind::I64:
            case IrPrimitiveKind::U64:
            case IrPrimitiveKind::F64:
            case IrPrimitiveKind::String:
            case IrPrimitiveKind::Function:
            case IrPrimitiveKind::Enum:
                return 64;
            case IrPrimitiveKind::F128:
                return 128;
            default:
                return 64;
        }
    }

    static int raw_pointer_stride_bytes(const IrType& type) {
        std::uint64_t size_bytes = 0;
        if (ari_layout_size_bytes(type, size_bytes)) return static_cast<int>(size_bytes);
        return static_cast<int>(raw_memory_bits(type) / 8);
    }

    static int layout_size_bytes(SourceLocation loc, const IrType& type) {
        std::uint64_t size = 0;
        if (!ari_layout_size_bytes(type, size) ||
            size > static_cast<std::uint64_t>(std::numeric_limits<int>::max())) {
            throw CompileError(where(loc) + ": backend cannot compute layout size for " + type_name(type));
        }
        return static_cast<int>(size);
    }

    static int field_offset_bytes(SourceLocation loc, const IrType& type, std::uint64_t index) {
        std::uint64_t offset = 0;
        if (!ari_layout_field_offset_bytes(type, index, offset) ||
            offset > static_cast<std::uint64_t>(std::numeric_limits<int>::max())) {
            throw CompileError(where(loc) + ": backend cannot compute aggregate field offset for " + type_name(type));
        }
        return static_cast<int>(offset);
    }

    static int aggregate_lvalue_field_offset(SourceLocation loc,
                                             int base_offset,
                                             const IrType& type,
                                             std::uint64_t index) {
        return base_offset - field_offset_bytes(loc, type, index);
    }

    static IrType i64_type(SourceLocation loc) {
        IrType type;
        type.primitive = IrPrimitiveKind::I64;
        type.name = "i64";
        type.loc = loc;
        return type;
    }

    static IrType u8_type(SourceLocation loc) {
        IrType type;
        type.primitive = IrPrimitiveKind::U8;
        type.name = "u8";
        type.loc = loc;
        return type;
    }

    void add_local(const std::string& name, const IrType& type) {
        if (locals_.count(name)) return;
        int size = local_size_bytes(type);
        int align = local_align_bytes(type);
        stack_offset_ = align_to(stack_offset_, align);
        locals_[name] = stack_offset_ + size;
        local_types_[name] = type;
        stack_offset_ += size;
    }

    void collect_locals(const std::vector<IrStmtPtr>& statements) {
        for (const auto& stmt : statements) collect_locals(*stmt);
    }

    void collect_expr_locals(const IrExprPtr& expr) {
        if (!expr) return;
        collect_expr_locals(*expr);
    }

    void collect_expr_locals(const IrExpr& expr) {
        collect_expr_locals(expr.operand);
        collect_expr_locals(expr.payload);
        collect_expr_locals(expr.left);
        collect_expr_locals(expr.right);
        collect_expr_locals(expr.condition);
        collect_locals(expr.then_body);
        collect_expr_locals(expr.then_value);
        collect_locals(expr.else_body);
        collect_expr_locals(expr.else_value);
        collect_locals(expr.block_body);
        collect_expr_locals(expr.block_value);
        collect_expr_locals(expr.match_value);
        for (const auto& arg : expr.args) collect_expr_locals(arg);
        for (const auto& arm : expr.match_arms) {
            if (arm.has_value_binding) add_local(arm.value_name, arm.value_type);
            add_payload_binding_locals(arm);
            collect_locals(arm.body);
            collect_expr_locals(arm.value);
        }
    }

    template <typename Arm>
    void add_payload_binding_locals(const Arm& arm) {
        if (!arm.payload_bindings.empty()) {
            for (const auto& binding : arm.payload_bindings) add_local(binding.name, binding.type);
            return;
        }
        if (arm.has_payload_binding) add_local(arm.payload_name, arm.payload_type);
    }

    void collect_locals(const IrStmt& stmt) {
        switch (stmt.kind) {
            case IrStmtKind::Block:
                collect_locals(stmt.statements);
                break;
            case IrStmtKind::VarDecl:
                add_local(stmt.binding.name, stmt.binding.type);
                collect_expr_locals(stmt.binding.init);
                break;
            case IrStmtKind::Assign:
                collect_expr_locals(stmt.rhs);
                break;
            case IrStmtKind::ExprStmt:
            case IrStmtKind::Return:
                collect_expr_locals(stmt.expr);
                break;
            case IrStmtKind::If:
                collect_expr_locals(stmt.condition);
                collect_locals(stmt.then_body);
                collect_locals(stmt.else_body);
                break;
            case IrStmtKind::While:
            case IrStmtKind::WhileLet:
                collect_expr_locals(stmt.condition);
                if (stmt.kind == IrStmtKind::WhileLet && !stmt.match_arms.empty() && stmt.match_arms[0].has_payload_binding) {
                    add_payload_binding_locals(stmt.match_arms[0]);
                }
                if (stmt.kind == IrStmtKind::WhileLet && !stmt.match_arms.empty() && stmt.match_arms[0].has_value_binding) {
                    add_local(stmt.match_arms[0].value_name, stmt.match_arms[0].value_type);
                }
                collect_locals(stmt.loop_body);
                break;
            case IrStmtKind::ForRange:
                add_local(stmt.for_index_name, stmt.for_start->type);
                add_local(stmt.for_end_name, stmt.for_end->type);
                collect_expr_locals(stmt.for_start);
                collect_expr_locals(stmt.for_end);
                if (!stmt.for_binding_name.empty()) add_local(stmt.for_binding_name, stmt.for_binding_type);
                collect_locals(stmt.loop_body);
                break;
            case IrStmtKind::ForVector:
                if (!stmt.for_binding_name.empty()) add_local(stmt.for_binding_name, stmt.for_binding_type);
                for (const auto& value : stmt.for_values) collect_expr_locals(value);
                collect_locals(stmt.loop_body);
                break;
            case IrStmtKind::InitWhile:
                for (const auto& binding : stmt.init_bindings) {
                    add_local(binding.name, binding.type);
                    collect_expr_locals(binding.init);
                }
                collect_locals(stmt.loop_body);
                for (const auto& update : stmt.updates) collect_expr_locals(update);
                break;
            case IrStmtKind::Continue:
                for (const auto& update : stmt.updates) collect_expr_locals(update);
                break;
            case IrStmtKind::Match:
                collect_expr_locals(stmt.match_value);
                for (const auto& arm : stmt.match_arms) {
                    if (arm.has_value_binding) add_local(arm.value_name, arm.value_type);
                    add_payload_binding_locals(arm);
                    collect_locals(arm.body);
                }
                break;
            case IrStmtKind::Break:
                collect_expr_locals(stmt.break_value);
                break;
            default:
                break;
        }
    }

    int local_offset(SourceLocation loc, const std::string& name) const {
        auto found = locals_.find(name);
        if (found == locals_.end()) throw CompileError(where(loc) + ": unknown codegen local '" + name + "'");
        return found->second;
    }

    const IrType& local_type(SourceLocation loc, const std::string& name) const {
        auto found = local_types_.find(name);
        if (found == local_types_.end()) throw CompileError(where(loc) + ": unknown codegen local '" + name + "'");
        return found->second;
    }

    void copy_local_bytes_to_offset(SourceLocation loc, int source_offset, int target_offset, const IrType& target_type) {
        int size = layout_size_bytes(loc, target_type);
        IrType byte_type = u8_type(loc);
        for (int byte = 0; byte < size; ++byte) {
            emit_load_rax_from_local(source_offset - byte, byte_type);
            emit_store_rax_to_local(target_offset - byte, byte_type);
        }
    }

    void copy_pointer_bytes_to_offset(const IrExpr& source, const IrType& target_type, int target_offset) {
        if (has_aggregate_enum_layout(target_type)) {
            throw CompileError(where(source.loc) + ": freestanding backend does not lower multi-payload enum values yet");
        }
        int size = layout_size_bytes(source.loc, target_type);
        IrType byte_type = u8_type(source.loc);
        emit_pointer_lvalue_address(source);
        emit_mov_reg_reg(Reg::RBX, Reg::RAX);
        for (int byte = 0; byte < size; ++byte) {
            emit_mov_reg_reg(Reg::RCX, Reg::RBX);
            emit_add_pointer_offset_reg(Reg::RCX, byte);
            emit_load_rax_from_ptr(Reg::RCX, byte_type);
            emit_store_rax_to_local(target_offset - byte, byte_type);
        }
    }

    void copy_local_bytes_to_pointer_base(SourceLocation loc, int source_offset, const IrType& target_type, int byte_offset) {
        int size = layout_size_bytes(loc, target_type);
        IrType byte_type = u8_type(loc);
        for (int byte = 0; byte < size; ++byte) {
            emit_load_rax_from_local(source_offset - byte, byte_type);
            emit_mov_reg_reg(Reg::RCX, Reg::RBX);
            emit_add_pointer_offset_reg(Reg::RCX, byte_offset + byte);
            emit_store_rax_to_ptr(Reg::RCX, byte_type);
        }
    }

    void copy_pointer_bytes_to_pointer_base(const IrExpr& source, const IrType& target_type, int byte_offset) {
        if (has_aggregate_enum_layout(target_type)) {
            throw CompileError(where(source.loc) + ": freestanding backend does not lower multi-payload enum values yet");
        }
        int size = layout_size_bytes(source.loc, target_type);
        IrType byte_type = u8_type(source.loc);
        emit_push(Reg::RBX);
        emit_pointer_lvalue_address(source);
        emit_mov_reg_reg(Reg::RDX, Reg::RAX);
        emit_pop(Reg::RBX);
        for (int byte = 0; byte < size; ++byte) {
            emit_mov_reg_reg(Reg::RCX, Reg::RDX);
            emit_add_pointer_offset_reg(Reg::RCX, byte);
            emit_load_rax_from_ptr(Reg::RCX, byte_type);
            emit_mov_reg_reg(Reg::RCX, Reg::RBX);
            emit_add_pointer_offset_reg(Reg::RCX, byte_offset + byte);
            emit_store_rax_to_ptr(Reg::RCX, byte_type);
        }
    }

    void emit_normalize_aggregate_enum_payload_value(SourceLocation loc, const IrType& type) {
        if (type.qualifier == TypeQualifier::Ptr) return;
        if (type.qualifier != TypeQualifier::Value) return;
        if (is_integer_primitive(type.primitive)) {
            emit_cast_to_type(loc, type);
            return;
        }
        if (type.primitive == IrPrimitiveKind::Bool) {
            emit_cmp_rax_zero();
            emit_setcc(0x95);
            return;
        }
        if (type.primitive == IrPrimitiveKind::Enum && type.field_types.empty()) return;
        if (type.primitive == IrPrimitiveKind::Function ||
            type.primitive == IrPrimitiveKind::String ||
            type.primitive == IrPrimitiveKind::Zone) {
            return;
        }
        throw CompileError(where(loc) + ": freestanding backend cannot store aggregate enum payload of type " +
                           type_name(type) + " yet");
    }

    void emit_cast_aggregate_enum_payload_slot_to_type(SourceLocation loc, const IrType& type) {
        if (type.qualifier == TypeQualifier::Ptr) return;
        if (type.qualifier != TypeQualifier::Value) return;
        if (is_integer_primitive(type.primitive)) {
            emit_cast_to_type(loc, type);
            return;
        }
        if (type.primitive == IrPrimitiveKind::Bool) return;
        if (type.primitive == IrPrimitiveKind::Enum && type.field_types.empty()) return;
        if (type.primitive == IrPrimitiveKind::Function ||
            type.primitive == IrPrimitiveKind::String ||
            type.primitive == IrPrimitiveKind::Zone) {
            return;
        }
        throw CompileError(where(loc) + ": freestanding backend cannot bind aggregate enum payload of type " +
                           type_name(type) + " yet");
    }

    void emit_store_aggregate_enum_construct_to_offset(const IrType& target_type,
                                                       const IrExpr& value,
                                                       int target_offset) {
        if (target_type.field_types.empty()) {
            throw CompileError(where(value.loc) + ": aggregate enum layout is missing during codegen");
        }

        int tag_offset = aggregate_lvalue_field_offset(value.loc, target_offset, target_type, 0);
        emit_mov_reg_imm64(Reg::RAX, value.enum_tag);
        emit_store_rax_to_local(tag_offset, target_type.field_types[0]);

        for (std::size_t i = 1; i < target_type.field_types.size(); ++i) {
            int payload_offset = aggregate_lvalue_field_offset(value.loc, target_offset, target_type, i);
            emit_mov_reg_imm64(Reg::RAX, 0);
            emit_store_rax_to_local(payload_offset, target_type.field_types[i]);
        }

        for (std::size_t i = 0; i < value.args.size(); ++i) {
            std::size_t field_index = i + 1;
            if (field_index >= target_type.field_types.size()) {
                throw CompileError(where(value.loc) + ": aggregate enum payload slot is missing during codegen");
            }
            int payload_offset = aggregate_lvalue_field_offset(value.loc, target_offset, target_type, field_index);
            emit_expr(*value.args[i]);
            emit_normalize_aggregate_enum_payload_value(value.args[i]->loc, value.args[i]->type);
            emit_store_rax_to_local(payload_offset, target_type.field_types[field_index]);
        }
    }

    void emit_store_aggregate_enum_value_to_offset(const IrType& target_type,
                                                   const IrExpr& value,
                                                   int target_offset) {
        if (value.kind == IrExprKind::EnumConstruct) {
            emit_store_aggregate_enum_construct_to_offset(target_type, value, target_offset);
            return;
        }
        if (value.kind == IrExprKind::Block) {
            emit_block_expr_to_offset(value, target_type, target_offset);
            return;
        }
        if (value.kind == IrExprKind::Local && is_aggregate_type(value.type)) {
            copy_local_bytes_to_offset(value.loc, local_offset(value.loc, value.name), target_offset, target_type);
            return;
        }
        if (value.kind == IrExprKind::TupleIndex && is_aggregate_type(value.type) &&
            value.operand && value.operand->kind == IrExprKind::Local) {
            copy_local_bytes_to_offset(value.loc, lvalue_offset(value), target_offset, target_type);
            return;
        }
        throw CompileError(where(value.loc) +
                           ": freestanding backend can only store local multi-payload enum values or constructors yet");
    }

    void emit_store_value_to_pointer_base(SourceLocation loc,
                                          const IrExpr& value,
                                          const IrType& target_type,
                                          int byte_offset) {
        if (!is_aggregate_type(target_type)) {
            emit_push(Reg::RBX);
            emit_expr(value);
            if (is_integer_primitive(target_type.primitive)) emit_cast_to_type(loc, target_type);
            emit_pop(Reg::RCX);
            emit_mov_reg_reg(Reg::RBX, Reg::RCX);
            emit_add_pointer_offset_reg(Reg::RCX, byte_offset);
            emit_store_rax_to_ptr(Reg::RCX, target_type);
            return;
        }

        if (has_aggregate_enum_layout(target_type)) {
            throw CompileError(where(value.loc) + ": freestanding backend does not lower multi-payload enum values yet");
        }

        if (value.kind == IrExprKind::Tuple ||
            (value.kind == IrExprKind::Vector && target_type.primitive == IrPrimitiveKind::Array)) {
            const std::vector<IrType>& fields = aggregate_field_types(target_type);
            for (std::size_t i = 0; i < fields.size(); ++i) {
                int offset = byte_offset + field_offset_bytes(value.loc, target_type, i);
                emit_store_value_to_pointer_base(loc, *value.args[i], fields[i], offset);
            }
            return;
        }

        if (value.kind == IrExprKind::Local && is_aggregate_type(value.type)) {
            copy_local_bytes_to_pointer_base(value.loc, local_offset(value.loc, value.name), target_type, byte_offset);
            return;
        }

        if (value.kind == IrExprKind::TupleIndex && is_aggregate_type(value.type) &&
            value.operand && value.operand->kind == IrExprKind::Local) {
            copy_local_bytes_to_pointer_base(value.loc, lvalue_offset(value), target_type, byte_offset);
            return;
        }

        if (is_pointer_backed_lvalue(value) && is_aggregate_type(value.type)) {
            copy_pointer_bytes_to_pointer_base(value, target_type, byte_offset);
            return;
        }

        throw CompileError(where(value.loc) + ": backend cannot store aggregate values through raw pointers here; bind a literal or copy a local");
    }

    void emit_store_value_to_offset(const IrType& target_type, const IrExpr& value, int target_offset) {
        if (!is_aggregate_type(target_type)) {
            emit_expr(value);
            if (is_integer_primitive(target_type.primitive)) emit_cast_to_type(value.loc, target_type);
            emit_store_rax_to_local(target_offset, target_type);
            return;
        }
        if (has_aggregate_enum_layout(target_type)) {
            emit_store_aggregate_enum_value_to_offset(target_type, value, target_offset);
            return;
        }

        if (value.kind == IrExprKind::Block) {
            emit_block_expr_to_offset(value, target_type, target_offset);
            return;
        }

        if (value.kind == IrExprKind::Tuple ||
            (value.kind == IrExprKind::Vector && target_type.primitive == IrPrimitiveKind::Array)) {
            const std::vector<IrType>& fields = aggregate_field_types(target_type);
            for (std::size_t i = 0; i < fields.size(); ++i) {
                int offset = aggregate_lvalue_field_offset(value.loc, target_offset, target_type, i);
                emit_store_value_to_offset(fields[i], *value.args[i], offset);
            }
            return;
        }

        if (value.kind == IrExprKind::Local && is_aggregate_type(value.type)) {
            copy_local_bytes_to_offset(value.loc, local_offset(value.loc, value.name), target_offset, target_type);
            return;
        }

        if (value.kind == IrExprKind::TupleIndex && is_aggregate_type(value.type) &&
            value.operand && value.operand->kind == IrExprKind::Local) {
            copy_local_bytes_to_offset(value.loc, lvalue_offset(value), target_offset, target_type);
            return;
        }

        if (is_pointer_backed_lvalue(value) && is_aggregate_type(value.type)) {
            copy_pointer_bytes_to_offset(value, target_type, target_offset);
            return;
        }

        throw CompileError(where(value.loc) + ": backend cannot materialize aggregate values here; bind a literal or copy a local");
    }

    void emit_block_expr_to_offset(const IrExpr& expr, const IrType& target_type, int target_offset) {
        if (!expr.label.empty()) {
            LoopLabels labels;
            labels.source_label = expr.label;
            labels.is_loop = false;
            labels.supports_break_value = true;
            labels.has_break_value_target = true;
            labels.break_value_target_offset = target_offset;
            labels.break_value_type = target_type;
            loops_.push_back(labels);
            emit_statements(expr.block_body);
            emit_store_value_to_offset(target_type, *expr.block_value, target_offset);
            std::size_t end = out_.size();
            for (std::size_t patch : loops_.back().break_patches) patch_rel32(patch, end);
            loops_.pop_back();
            return;
        }

        emit_statements(expr.block_body);
        emit_store_value_to_offset(target_type, *expr.block_value, target_offset);
    }

    void emit_rex(bool w, int r, int b) {
        std::uint8_t rex = 0x40;
        if (w) rex |= 0x08;
        if (r & 8) rex |= 0x04;
        if (b & 8) rex |= 0x01;
        if (rex != 0x40) out_.u8(rex);
    }

    void emit_modrm(int mod, int reg, int rm) {
        out_.u8(static_cast<std::uint8_t>(((mod & 3) << 6) | ((reg & 7) << 3) | (rm & 7)));
    }

    void emit_mov_reg_imm64(Reg dst, std::uint64_t value) {
        int d = reg_code(dst);
        emit_rex(true, 0, d);
        out_.u8(static_cast<std::uint8_t>(0xB8 + (d & 7)));
        out_.u64(value);
    }

    void emit_mov_reg_reg(Reg dst, Reg src) {
        int d = reg_code(dst);
        int s = reg_code(src);
        emit_rex(true, s, d);
        out_.u8(0x89);
        emit_modrm(3, s, d);
    }

    void emit_mov_mem_reg(int offset, Reg src) {
        int s = reg_code(src);
        emit_rex(true, s, reg_code(Reg::RBP));
        out_.u8(0x89);
        emit_modrm(2, s, reg_code(Reg::RBP));
        out_.u32(static_cast<std::uint32_t>(-offset));
    }

    void emit_mov_reg_mem(Reg dst, int offset) {
        int d = reg_code(dst);
        emit_rex(true, d, reg_code(Reg::RBP));
        out_.u8(0x8B);
        emit_modrm(2, d, reg_code(Reg::RBP));
        out_.u32(static_cast<std::uint32_t>(-offset));
    }

    void emit_load_rax_from_local(int offset, const IrType& type) {
        int b = reg_code(Reg::RBP);
        bool is_signed = type.qualifier == TypeQualifier::Value && is_signed_integer_primitive(type.primitive);
        switch (raw_memory_bits(type)) {
            case 8:
                if (is_signed) {
                    emit_rex(true, reg_code(Reg::RAX), b);
                    out_.u8(0x0F);
                    out_.u8(0xBE);
                    emit_modrm(2, reg_code(Reg::RAX), b);
                } else {
                    emit_rex(false, reg_code(Reg::RAX), b);
                    out_.u8(0x0F);
                    out_.u8(0xB6);
                    emit_modrm(2, reg_code(Reg::RAX), b);
                }
                out_.u32(static_cast<std::uint32_t>(-offset));
                break;
            case 16:
                if (is_signed) {
                    emit_rex(true, reg_code(Reg::RAX), b);
                    out_.u8(0x0F);
                    out_.u8(0xBF);
                    emit_modrm(2, reg_code(Reg::RAX), b);
                } else {
                    emit_rex(false, reg_code(Reg::RAX), b);
                    out_.u8(0x0F);
                    out_.u8(0xB7);
                    emit_modrm(2, reg_code(Reg::RAX), b);
                }
                out_.u32(static_cast<std::uint32_t>(-offset));
                break;
            case 32:
                if (is_signed) {
                    emit_rex(true, reg_code(Reg::RAX), b);
                    out_.u8(0x63);
                    emit_modrm(2, reg_code(Reg::RAX), b);
                } else {
                    emit_rex(false, reg_code(Reg::RAX), b);
                    out_.u8(0x8B);
                    emit_modrm(2, reg_code(Reg::RAX), b);
                }
                out_.u32(static_cast<std::uint32_t>(-offset));
                break;
            default:
                emit_mov_reg_mem(Reg::RAX, offset);
                break;
        }
    }

    void emit_store_rax_to_local(int offset, const IrType& type) {
        int b = reg_code(Reg::RBP);
        switch (raw_memory_bits(type)) {
            case 8:
                emit_rex(false, reg_code(Reg::RAX), b);
                out_.u8(0x88);
                emit_modrm(2, reg_code(Reg::RAX), b);
                out_.u32(static_cast<std::uint32_t>(-offset));
                break;
            case 16:
                out_.u8(0x66);
                emit_rex(false, reg_code(Reg::RAX), b);
                out_.u8(0x89);
                emit_modrm(2, reg_code(Reg::RAX), b);
                out_.u32(static_cast<std::uint32_t>(-offset));
                break;
            case 32:
                emit_rex(false, reg_code(Reg::RAX), b);
                out_.u8(0x89);
                emit_modrm(2, reg_code(Reg::RAX), b);
                out_.u32(static_cast<std::uint32_t>(-offset));
                break;
            default:
                emit_mov_mem_reg(offset, Reg::RAX);
                break;
        }
    }

    void emit_mov_reg_ptr(Reg dst, Reg base) {
        int d = reg_code(dst);
        int b = reg_code(base);
        emit_rex(true, d, b);
        out_.u8(0x8B);
        emit_modrm(0, d, b);
    }

    void emit_load_rax_from_ptr(Reg base, const IrType& type) {
        int b = reg_code(base);
        bool is_signed = type.qualifier == TypeQualifier::Value && is_signed_integer_primitive(type.primitive);
        switch (raw_memory_bits(type)) {
            case 8:
                if (is_signed) {
                    emit_rex(true, reg_code(Reg::RAX), b);
                    out_.u8(0x0F);
                    out_.u8(0xBE);
                    emit_modrm(0, reg_code(Reg::RAX), b);
                } else {
                    emit_rex(false, reg_code(Reg::RAX), b);
                    out_.u8(0x0F);
                    out_.u8(0xB6);
                    emit_modrm(0, reg_code(Reg::RAX), b);
                }
                break;
            case 16:
                if (is_signed) {
                    emit_rex(true, reg_code(Reg::RAX), b);
                    out_.u8(0x0F);
                    out_.u8(0xBF);
                    emit_modrm(0, reg_code(Reg::RAX), b);
                } else {
                    emit_rex(false, reg_code(Reg::RAX), b);
                    out_.u8(0x0F);
                    out_.u8(0xB7);
                    emit_modrm(0, reg_code(Reg::RAX), b);
                }
                break;
            case 32:
                if (is_signed) {
                    emit_rex(true, reg_code(Reg::RAX), b);
                    out_.u8(0x63);
                    emit_modrm(0, reg_code(Reg::RAX), b);
                } else {
                    emit_rex(false, reg_code(Reg::RAX), b);
                    out_.u8(0x8B);
                    emit_modrm(0, reg_code(Reg::RAX), b);
                }
                break;
            default:
                emit_mov_reg_ptr(Reg::RAX, base);
                break;
        }
    }

    void emit_store_rax_to_ptr(Reg base, const IrType& type) {
        int b = reg_code(base);
        switch (raw_memory_bits(type)) {
            case 8:
                emit_rex(false, reg_code(Reg::RAX), b);
                out_.u8(0x88);
                emit_modrm(0, reg_code(Reg::RAX), b);
                break;
            case 16:
                out_.u8(0x66);
                emit_rex(false, reg_code(Reg::RAX), b);
                out_.u8(0x89);
                emit_modrm(0, reg_code(Reg::RAX), b);
                break;
            case 32:
                emit_rex(false, reg_code(Reg::RAX), b);
                out_.u8(0x89);
                emit_modrm(0, reg_code(Reg::RAX), b);
                break;
            default:
                emit_rex(true, reg_code(Reg::RAX), b);
                out_.u8(0x89);
                emit_modrm(0, reg_code(Reg::RAX), b);
                break;
        }
    }

    void emit_mov_reg_stack_arg(Reg dst, int offset) {
        int d = reg_code(dst);
        emit_rex(true, d, reg_code(Reg::RBP));
        out_.u8(0x8B);
        emit_modrm(2, d, reg_code(Reg::RBP));
        out_.u32(static_cast<std::uint32_t>(offset));
    }

    void emit_mov_mem_rsp_reg(int offset, Reg src) {
        int s = reg_code(src);
        emit_rex(true, s, reg_code(Reg::RSP));
        out_.u8(0x89);
        emit_modrm(2, s, reg_code(Reg::RSP));
        out_.u8(0x24);
        out_.u32(static_cast<std::uint32_t>(offset));
    }

    void emit_mov_reg_mem_rsp(Reg dst, int offset) {
        int d = reg_code(dst);
        emit_rex(true, d, reg_code(Reg::RSP));
        out_.u8(0x8B);
        emit_modrm(2, d, reg_code(Reg::RSP));
        out_.u8(0x24);
        out_.u32(static_cast<std::uint32_t>(offset));
    }

    void emit_lea_reg_local(Reg dst, int offset) {
        int d = reg_code(dst);
        emit_rex(true, d, reg_code(Reg::RBP));
        out_.u8(0x8D);
        emit_modrm(2, d, reg_code(Reg::RBP));
        out_.u32(static_cast<std::uint32_t>(-offset));
    }

    void emit_add_reg_reg(Reg dst, Reg src) {
        int d = reg_code(dst);
        int s = reg_code(src);
        emit_rex(true, s, d);
        out_.u8(0x01);
        emit_modrm(3, s, d);
    }

    void emit_sub_reg_reg(Reg dst, Reg src) {
        int d = reg_code(dst);
        int s = reg_code(src);
        emit_rex(true, s, d);
        out_.u8(0x29);
        emit_modrm(3, s, d);
    }

    void emit_add_pointer_offset_reg(Reg reg, int byte_offset) {
        if (byte_offset == 0) return;
        emit_mov_reg_imm64(Reg::RSI, static_cast<std::uint64_t>(byte_offset));
        emit_add_reg_reg(reg, Reg::RSI);
    }

    void emit_and_reg_reg(Reg dst, Reg src) {
        int d = reg_code(dst);
        int s = reg_code(src);
        emit_rex(true, s, d);
        out_.u8(0x21);
        emit_modrm(3, s, d);
    }

    void emit_or_reg_reg(Reg dst, Reg src) {
        int d = reg_code(dst);
        int s = reg_code(src);
        emit_rex(true, s, d);
        out_.u8(0x09);
        emit_modrm(3, s, d);
    }

    void emit_xor_reg_reg(Reg dst, Reg src) {
        int d = reg_code(dst);
        int s = reg_code(src);
        emit_rex(true, s, d);
        out_.u8(0x31);
        emit_modrm(3, s, d);
    }

    void emit_imul_reg_reg(Reg dst, Reg src) {
        int d = reg_code(dst);
        int s = reg_code(src);
        emit_rex(true, d, s);
        out_.u8(0x0F);
        out_.u8(0xAF);
        emit_modrm(3, d, s);
    }

    void emit_cmp_reg_reg(Reg lhs, Reg rhs) {
        int l = reg_code(lhs);
        int r = reg_code(rhs);
        emit_rex(true, r, l);
        out_.u8(0x39);
        emit_modrm(3, r, l);
    }

    void emit_cmp_rax_zero() {
        out_.u8(0x48);
        out_.u8(0x83);
        out_.u8(0xF8);
        out_.u8(0x00);
    }

    void emit_setcc(std::uint8_t cc) {
        out_.u8(0x0F);
        out_.u8(cc);
        out_.u8(0xC0);
        out_.u8(0x0F);
        out_.u8(0xB6);
        out_.u8(0xC0);
    }

    void emit_shift_rax_cl(std::uint8_t modrm) {
        out_.u8(0x48);
        out_.u8(0xD3);
        out_.u8(modrm);
    }

    void emit_shl_rax_imm8(std::uint8_t amount) {
        out_.u8(0x48);
        out_.u8(0xC1);
        out_.u8(0xE0);
        out_.u8(amount);
    }

    void emit_shr_rax_imm8(std::uint8_t amount) {
        out_.u8(0x48);
        out_.u8(0xC1);
        out_.u8(0xE8);
        out_.u8(amount);
    }

    void emit_mov_reg_rsp(Reg dst) {
        int d = reg_code(dst);
        emit_rex(true, d, reg_code(Reg::RSP));
        out_.u8(0x8B);
        emit_modrm(0, d, reg_code(Reg::RSP));
        out_.u8(0x24);
    }

    void emit_sub_rsp(int amount) {
        if (amount <= 127) {
            out_.u8(0x48);
            out_.u8(0x83);
            out_.u8(0xEC);
            out_.u8(static_cast<std::uint8_t>(amount));
        } else {
            out_.u8(0x48);
            out_.u8(0x81);
            out_.u8(0xEC);
            out_.u32(static_cast<std::uint32_t>(amount));
        }
    }

    void emit_add_rsp(int amount) {
        if (amount <= 0) return;
        if (amount <= 127) {
            out_.u8(0x48);
            out_.u8(0x83);
            out_.u8(0xC4);
            out_.u8(static_cast<std::uint8_t>(amount));
        } else {
            out_.u8(0x48);
            out_.u8(0x81);
            out_.u8(0xC4);
            out_.u32(static_cast<std::uint32_t>(amount));
        }
    }

    void emit_push(Reg reg) {
        int r = reg_code(reg);
        emit_rex(false, 0, r);
        out_.u8(static_cast<std::uint8_t>(0x50 + (r & 7)));
    }

    void emit_pop(Reg reg) {
        int r = reg_code(reg);
        emit_rex(false, 0, r);
        out_.u8(static_cast<std::uint8_t>(0x58 + (r & 7)));
    }

    std::size_t emit_jmp_placeholder() {
        out_.u8(0xE9);
        std::size_t pos = out_.size();
        out_.u32(0);
        return pos;
    }

    std::size_t emit_jcc_placeholder(std::uint8_t cc) {
        out_.u8(0x0F);
        out_.u8(cc);
        std::size_t pos = out_.size();
        out_.u32(0);
        return pos;
    }

    void patch_rel32(std::size_t imm_pos, std::size_t target) {
        std::int64_t rel = static_cast<std::int64_t>(target) - static_cast<std::int64_t>(imm_pos + 4);
        out_.patch32(imm_pos, static_cast<std::int32_t>(rel));
    }

    void store_params() {
        static const Reg arg_regs[] = {Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9};
        for (std::size_t i = 0; i < fn_.params.size(); ++i) {
            if (local_slot_count(fn_.params[i].type) == 0) continue;
            if (is_aggregate_type(fn_.params[i].type)) {
                throw CompileError(where(fn_.loc) + ": backend does not lower aggregate parameters yet");
            }
            int offset = local_offset(fn_.loc, fn_.params[i].name);
            if (i < 6) {
                emit_mov_reg_reg(Reg::RAX, arg_regs[i]);
                emit_store_rax_to_local(offset, fn_.params[i].type);
            } else {
                emit_mov_reg_stack_arg(Reg::RAX, 16 + static_cast<int>((i - 6) * 8));
                emit_store_rax_to_local(offset, fn_.params[i].type);
            }
        }
    }

    void emit_statements(const std::vector<IrStmtPtr>& statements) {
        for (const auto& stmt : statements) emit_statement(*stmt);
    }

    void emit_statement(const IrStmt& stmt) {
        switch (stmt.kind) {
            case IrStmtKind::Block:
                emit_block(stmt);
                break;
            case IrStmtKind::VarDecl:
                emit_store_value_to_offset(stmt.binding.type, *stmt.binding.init,
                                           local_offset(stmt.loc, stmt.binding.name));
                break;
            case IrStmtKind::Assign:
                if (stmt.assign_target) {
                    if (is_pointer_backed_lvalue(*stmt.assign_target)) {
                        emit_store_to_pointer_lvalue_target(*stmt.assign_target, *stmt.rhs);
                    } else {
                        emit_store_value_to_offset(stmt.assign_target->type, *stmt.rhs, lvalue_offset(*stmt.assign_target));
                    }
                } else {
                    emit_store_value_to_offset(local_type(stmt.loc, stmt.assign_name), *stmt.rhs,
                                               local_offset(stmt.loc, stmt.assign_name));
                }
                break;
            case IrStmtKind::ExprStmt:
                emit_expr(*stmt.expr);
                break;
            case IrStmtKind::Return:
                if (stmt.expr) {
                    emit_expr(*stmt.expr);
                    emit_normalize_return_value(stmt.loc);
                } else {
                    emit_mov_reg_imm64(Reg::RAX, 0);
                }
                return_jumps_.push_back(emit_jmp_placeholder());
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
                emit_break(stmt);
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
        emit_expr(*stmt.condition);
        emit_cmp_rax_zero();
        std::size_t jump_false = emit_jcc_placeholder(0x84);
        emit_statements(stmt.then_body);
        if (stmt.else_body.empty()) {
            patch_rel32(jump_false, out_.size());
            return;
        }
        std::size_t jump_end = emit_jmp_placeholder();
        patch_rel32(jump_false, out_.size());
        emit_statements(stmt.else_body);
        patch_rel32(jump_end, out_.size());
    }

    void emit_block(const IrStmt& stmt) {
        if (stmt.label.empty()) {
            emit_statements(stmt.statements);
            return;
        }

        LoopLabels labels;
        labels.source_label = stmt.label;
        labels.is_loop = false;
        loops_.push_back(labels);
        emit_statements(stmt.statements);

        std::size_t end = out_.size();
        for (std::size_t patch : loops_.back().break_patches) patch_rel32(patch, end);
        loops_.pop_back();
    }

    LoopLabels& loop_for_continue(SourceLocation loc) {
        for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
            if (loop->is_loop) return *loop;
        }
        throw CompileError(where(loc) + ": continue outside loop during codegen");
    }

    void emit_while(const IrStmt& stmt) {
        std::size_t start = out_.size();
        emit_expr(*stmt.condition);
        emit_cmp_rax_zero();
        std::size_t jump_end = emit_jcc_placeholder(0x84);
        LoopLabels labels;
        labels.plain_continue_known = true;
        labels.plain_continue_target = start;
        labels.value_continue_target = start;
        labels.source_label = stmt.label;
        loops_.push_back(labels);
        emit_statements(stmt.loop_body);
        patch_rel32(emit_jmp_placeholder(), start);
        std::size_t end = out_.size();
        patch_rel32(jump_end, end);
        for (std::size_t patch : loops_.back().break_patches) patch_rel32(patch, end);
        loops_.pop_back();
    }

    void emit_while_let(const IrStmt& stmt) {
        if (stmt.match_arms.empty()) throw CompileError(where(stmt.loc) + ": while-let missing lowered pattern");
        std::size_t cond = out_.size();
        emit_expr(*stmt.match_value);
        emit_push(Reg::RAX);

        std::vector<std::size_t> jump_body;
        for (const auto& arm : stmt.match_arms) {
            std::vector<std::size_t> jump_next = emit_match_arm_fail_jumps(arm);
            emit_match_arm_bindings(arm);
            emit_pop(Reg::RCX);
            jump_body.push_back(emit_jmp_placeholder());
            for (std::size_t patch : jump_next) patch_rel32(patch, out_.size());
        }

        emit_pop(Reg::RCX);
        std::size_t jump_failed = emit_jmp_placeholder();

        std::size_t body = out_.size();
        for (std::size_t patch : jump_body) patch_rel32(patch, body);

        LoopLabels labels;
        labels.plain_continue_known = true;
        labels.plain_continue_target = cond;
        labels.value_continue_target = cond;
        labels.source_label = stmt.label;
        loops_.push_back(labels);
        emit_statements(stmt.loop_body);
        patch_rel32(emit_jmp_placeholder(), cond);

        std::size_t end = out_.size();
        patch_rel32(jump_failed, end);
        for (std::size_t patch : loops_.back().break_patches) patch_rel32(patch, end);
        loops_.pop_back();
    }

    void emit_for_range(const IrStmt& stmt) {
        emit_expr(*stmt.for_start);
        emit_store_rax_to_local(local_offset(stmt.loc, stmt.for_index_name),
                                local_type(stmt.loc, stmt.for_index_name));
        emit_expr(*stmt.for_end);
        emit_store_rax_to_local(local_offset(stmt.loc, stmt.for_end_name),
                                local_type(stmt.loc, stmt.for_end_name));

        std::size_t cond = out_.size();
        emit_load_rax_from_local(local_offset(stmt.loc, stmt.for_index_name),
                                 local_type(stmt.loc, stmt.for_index_name));
        emit_push(Reg::RAX);
        emit_load_rax_from_local(local_offset(stmt.loc, stmt.for_end_name),
                                 local_type(stmt.loc, stmt.for_end_name));
        emit_pop(Reg::RCX);
        emit_cmp_reg_reg(Reg::RCX, Reg::RAX);
        bool unsigned_range = is_unsigned_integer_type(stmt.for_start->type);
        std::size_t jump_end = emit_jcc_placeholder(stmt.for_inclusive
            ? (unsigned_range ? 0x87 : 0x8F)
            : (unsigned_range ? 0x83 : 0x8D));

        if (!stmt.for_binding_name.empty()) {
            emit_load_rax_from_local(local_offset(stmt.loc, stmt.for_index_name),
                                     local_type(stmt.loc, stmt.for_index_name));
            emit_store_rax_to_local(local_offset(stmt.loc, stmt.for_binding_name),
                                    local_type(stmt.loc, stmt.for_binding_name));
        }

        LoopLabels labels;
        labels.plain_continue_known = false;
        labels.value_continue_target = cond;
        labels.source_label = stmt.label;
        loops_.push_back(labels);
        emit_statements(stmt.loop_body);

        std::size_t update = out_.size();
        loops_.back().plain_continue_target = update;
        loops_.back().plain_continue_known = true;
        for (std::size_t patch : loops_.back().plain_continue_patches) patch_rel32(patch, update);
        std::size_t inclusive_done = 0;
        if (stmt.for_inclusive) {
            emit_load_rax_from_local(local_offset(stmt.loc, stmt.for_index_name),
                                     local_type(stmt.loc, stmt.for_index_name));
            emit_push(Reg::RAX);
            emit_load_rax_from_local(local_offset(stmt.loc, stmt.for_end_name),
                                     local_type(stmt.loc, stmt.for_end_name));
            emit_pop(Reg::RCX);
            emit_cmp_reg_reg(Reg::RCX, Reg::RAX);
            inclusive_done = emit_jcc_placeholder(0x84);
        }
        emit_load_rax_from_local(local_offset(stmt.loc, stmt.for_index_name),
                                 local_type(stmt.loc, stmt.for_index_name));
        emit_mov_reg_imm64(Reg::RCX, 1);
        emit_add_reg_reg(Reg::RAX, Reg::RCX);
        emit_store_rax_to_local(local_offset(stmt.loc, stmt.for_index_name),
                                local_type(stmt.loc, stmt.for_index_name));
        patch_rel32(emit_jmp_placeholder(), cond);

        std::size_t end = out_.size();
        patch_rel32(jump_end, end);
        if (inclusive_done) patch_rel32(inclusive_done, end);
        for (std::size_t patch : loops_.back().break_patches) patch_rel32(patch, end);
        loops_.pop_back();
    }

    void emit_for_vector(const IrStmt& stmt) {
        LoopLabels labels;
        labels.source_label = stmt.label;
        loops_.push_back(labels);
        std::vector<std::size_t> pending_continue;

        for (const auto& value : stmt.for_values) {
            std::size_t iteration_start = out_.size();
            for (std::size_t patch : pending_continue) patch_rel32(patch, iteration_start);
            pending_continue.clear();

            if (!stmt.for_binding_name.empty()) {
                emit_store_value_to_offset(stmt.for_binding_type, *value, local_offset(stmt.loc, stmt.for_binding_name));
            } else {
                emit_expr(*value);
            }

            std::size_t continue_start = loops_.back().plain_continue_patches.size();
            emit_statements(stmt.loop_body);
            auto& patches = loops_.back().plain_continue_patches;
            pending_continue.insert(pending_continue.end(), patches.begin() + static_cast<std::ptrdiff_t>(continue_start), patches.end());
            patches.erase(patches.begin() + static_cast<std::ptrdiff_t>(continue_start), patches.end());
        }

        std::size_t end = out_.size();
        for (std::size_t patch : pending_continue) patch_rel32(patch, end);
        for (std::size_t patch : loops_.back().plain_continue_patches) patch_rel32(patch, end);
        for (std::size_t patch : loops_.back().break_patches) patch_rel32(patch, end);
        loops_.pop_back();
    }

    void emit_init_while(const IrStmt& stmt) {
        std::vector<std::string> names;
        for (const auto& binding : stmt.init_bindings) {
            names.push_back(binding.name);
            emit_expr(*binding.init);
            emit_store_rax_to_local(local_offset(binding.loc, binding.name), binding.type);
        }

        std::size_t cond = out_.size();
        emit_expr(*stmt.condition);
        emit_cmp_rax_zero();
        std::size_t jump_end = emit_jcc_placeholder(0x84);

        LoopLabels labels;
        labels.plain_continue_known = false;
        labels.plain_continue_target = 0;
        labels.value_continue_target = cond;
        labels.source_label = stmt.label;
        labels.update_names = names;
        loops_.push_back(labels);

        emit_statements(stmt.loop_body);
        std::size_t update = out_.size();
        loops_.back().plain_continue_target = update;
        loops_.back().plain_continue_known = true;
        for (std::size_t patch : loops_.back().plain_continue_patches) patch_rel32(patch, update);
        emit_parallel_update(stmt.loc, names, stmt.updates);
        patch_rel32(emit_jmp_placeholder(), cond);

        std::size_t end = out_.size();
        patch_rel32(jump_end, end);
        for (std::size_t patch : loops_.back().break_patches) patch_rel32(patch, end);
        loops_.pop_back();
    }

    void emit_parallel_update(SourceLocation loc, const std::vector<std::string>& names, const std::vector<IrExprPtr>& values) {
        if (values.empty()) return;
        if (names.size() != values.size()) throw CompileError(where(loc) + ": update count mismatch");
        for (const auto& value : values) {
            emit_expr(*value);
            emit_push(Reg::RAX);
        }
        for (std::size_t i = values.size(); i > 0; --i) {
            emit_pop(Reg::RAX);
            emit_store_rax_to_local(local_offset(loc, names[i - 1]), local_type(loc, names[i - 1]));
        }
    }

    void emit_continue(const IrStmt& stmt) {
        LoopLabels& loop = loop_for_continue(stmt.loc);
        if (!stmt.updates.empty()) {
            if (stmt.updates.size() != loop.update_names.size()) {
                throw CompileError(where(stmt.loc) + ": continue update count mismatch");
            }
            emit_parallel_update(stmt.loc, loop.update_names, stmt.updates);
            patch_rel32(emit_jmp_placeholder(), loop.value_continue_target);
        } else {
            std::size_t patch = emit_jmp_placeholder();
            if (loop.plain_continue_known) patch_rel32(patch, loop.plain_continue_target);
            else loop.plain_continue_patches.push_back(patch);
        }
    }

    void emit_break(const IrStmt& stmt) {
        LoopLabels& target = loop_for_break(stmt);
        if (stmt.break_value) {
            if (target.has_break_value_target) {
                emit_store_value_to_offset(target.break_value_type, *stmt.break_value, target.break_value_target_offset);
            } else {
                emit_expr(*stmt.break_value);
            }
        }
        target.break_patches.push_back(emit_jmp_placeholder());
    }

    LoopLabels& loop_for_break(const IrStmt& stmt) {
        if (stmt.break_label.empty()) {
            for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
                if (loop->is_loop) return *loop;
            }
            throw CompileError(where(stmt.loc) + ": break outside loop during codegen");
        }
        for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
            if (loop->source_label == stmt.break_label) return *loop;
        }
        throw CompileError(where(stmt.loc) + ": unknown loop label '" + stmt.break_label + "' during codegen");
    }

    void emit_match(const IrStmt& stmt) {
        if (stmt.match_value && has_aggregate_enum_layout(stmt.match_value->type)) {
            emit_aggregate_enum_match(stmt);
            return;
        }

        emit_expr(*stmt.match_value);
        emit_push(Reg::RAX);

        std::vector<std::size_t> end_patches;
        for (const auto& arm : stmt.match_arms) {
            bool has_next_patch = false;

            std::vector<std::size_t> next_patches;
            if (!arm.wildcard) {
                next_patches = emit_match_arm_fail_jumps(arm);
                has_next_patch = true;
            }

            emit_match_arm_bindings(arm);
            emit_pop(Reg::RCX);
            emit_statements(arm.body);
            end_patches.push_back(emit_jmp_placeholder());

            if (has_next_patch) {
                for (std::size_t patch : next_patches) patch_rel32(patch, out_.size());
            }
        }

        emit_pop(Reg::RCX);
        std::size_t end = out_.size();
        for (std::size_t patch : end_patches) patch_rel32(patch, end);
    }

    int aggregate_enum_match_base_offset(const IrExpr& value) const {
        if (value.kind == IrExprKind::Local) return local_offset(value.loc, value.name);
        if (value.kind == IrExprKind::TupleIndex && value.operand) return lvalue_offset(value);
        throw CompileError(where(value.loc) +
                           ": freestanding backend can only match local multi-payload enum values yet");
    }

    template <typename Arm>
    std::vector<std::size_t> emit_aggregate_enum_match_arm_fail_jumps(const Arm& arm,
                                                                      int match_base_offset,
                                                                      const IrType& enum_type) {
        require_aggregate_enum_match_arm_supported(arm);

        int tag_offset = aggregate_lvalue_field_offset(arm.loc, match_base_offset, enum_type, 0);
        if (enum_type.field_types.empty()) {
            throw CompileError(where(arm.loc) + ": aggregate enum layout is missing during codegen");
        }
        const IrType& tag_type = enum_type.field_types[0];
        emit_load_rax_from_local(tag_offset, tag_type);
        emit_mov_reg_imm64(Reg::RCX, arm.enum_tag);
        emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
        std::vector<std::size_t> patches;
        patches.push_back(emit_jcc_placeholder(0x85));

        for (const auto& condition : arm.payload_literal_conditions) {
            emit_load_aggregate_enum_payload_slot(condition.index, arm.loc, match_base_offset, enum_type);
            emit_mov_reg_imm64(Reg::RCX, aggregate_payload_literal_bits(condition));
            emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
            patches.push_back(emit_jcc_placeholder(0x85));
        }

        for (const auto& condition : arm.payload_range_conditions) {
            if (condition.compact_enum_payload) {
                throw CompileError(where(arm.loc) +
                                   ": freestanding backend does not lower nested aggregate enum payload test patterns yet");
            }
            emit_load_aggregate_enum_payload_slot(condition.index, arm.loc, match_base_offset, enum_type);
            emit_cast_aggregate_enum_payload_slot_to_type(arm.loc, condition.type);
            emit_mov_reg_imm64(Reg::RCX, payload_range_start_bits(condition));
            emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
            patches.push_back(emit_jcc_placeholder(condition.is_unsigned ? 0x82 : 0x8C));

            emit_load_aggregate_enum_payload_slot(condition.index, arm.loc, match_base_offset, enum_type);
            emit_cast_aggregate_enum_payload_slot_to_type(arm.loc, condition.type);
            emit_mov_reg_imm64(Reg::RCX, payload_range_end_bits(condition));
            emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
            patches.push_back(emit_jcc_placeholder(payload_range_past_end_cc(condition)));
        }

        for (const auto& condition : arm.payload_enum_conditions) {
            emit_load_aggregate_enum_payload_slot(condition.index, arm.loc, match_base_offset, enum_type);
            emit_mov_reg_imm64(Reg::RCX, 0xffffffffULL);
            emit_and_reg_reg(Reg::RAX, Reg::RCX);
            emit_mov_reg_imm64(Reg::RCX, condition.tag);
            emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
            patches.push_back(emit_jcc_placeholder(0x85));

            if (condition.has_payload_literal) {
                emit_load_aggregate_enum_payload_slot(condition.index, arm.loc, match_base_offset, enum_type);
                emit_shr_rax_imm8(32);
                emit_cast_aggregate_enum_payload_slot_to_type(arm.loc, condition.payload_type);
                emit_mov_reg_imm64(Reg::RCX, nested_payload_literal_bits(condition));
                emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
                patches.push_back(emit_jcc_placeholder(0x85));
            }

            if (condition.has_payload_range) {
                emit_load_aggregate_enum_payload_slot(condition.index, arm.loc, match_base_offset, enum_type);
                emit_shr_rax_imm8(32);
                emit_cast_aggregate_enum_payload_slot_to_type(arm.loc, condition.payload_type);
                emit_mov_reg_imm64(Reg::RCX, nested_payload_range_start_bits(condition));
                emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
                patches.push_back(emit_jcc_placeholder(condition.range_is_unsigned ? 0x82 : 0x8C));

                emit_load_aggregate_enum_payload_slot(condition.index, arm.loc, match_base_offset, enum_type);
                emit_shr_rax_imm8(32);
                emit_cast_aggregate_enum_payload_slot_to_type(arm.loc, condition.payload_type);
                emit_mov_reg_imm64(Reg::RCX, nested_payload_range_end_bits(condition));
                emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
                patches.push_back(emit_jcc_placeholder(nested_payload_range_past_end_cc(condition)));
            }
        }

        return patches;
    }

    template <typename Arm>
    void require_aggregate_enum_match_arm_supported(const Arm& arm) const {
        if (arm.has_literal || arm.has_range) {
            throw CompileError(where(arm.loc) +
                               ": freestanding backend does not lower aggregate enum payload test patterns yet");
        }
    }

    std::size_t aggregate_enum_payload_field_index(SourceLocation loc,
                                                   const IrType& enum_type,
                                                   std::uint32_t payload_index) const {
        std::size_t field_index = static_cast<std::size_t>(payload_index) + 1;
        if (field_index >= enum_type.field_types.size()) {
            throw CompileError(where(loc) + ": aggregate enum payload slot is missing during codegen");
        }
        return field_index;
    }

    void emit_load_aggregate_enum_payload_slot(std::uint32_t payload_index,
                                               SourceLocation loc,
                                               int match_base_offset,
                                               const IrType& enum_type) {
        std::size_t field_index = aggregate_enum_payload_field_index(loc, enum_type, payload_index);
        int payload_offset = aggregate_lvalue_field_offset(loc, match_base_offset, enum_type, field_index);
        emit_load_rax_from_local(payload_offset, enum_type.field_types[field_index]);
    }

    static std::uint64_t aggregate_payload_literal_bits(const IrPayloadLiteralCondition& condition) {
        if (condition.is_bool) return condition.bool_value ? 1 : 0;
        return condition.value;
    }

    static std::uint64_t nested_payload_literal_bits(const IrPayloadEnumCondition& condition) {
        if (condition.payload_literal_is_bool) return condition.payload_literal_bool ? 1 : 0;
        return condition.payload_literal_negative ? 0 - condition.payload_literal_int
                                                  : condition.payload_literal_int;
    }

    static std::uint64_t nested_payload_range_start_bits(const IrPayloadEnumCondition& condition) {
        return condition.range_start_negative ? 0 - condition.range_start_int : condition.range_start_int;
    }

    static std::uint64_t nested_payload_range_end_bits(const IrPayloadEnumCondition& condition) {
        return condition.range_end_negative ? 0 - condition.range_end_int : condition.range_end_int;
    }

    static std::uint8_t nested_payload_range_past_end_cc(const IrPayloadEnumCondition& condition) {
        if (condition.range_inclusive) return condition.range_is_unsigned ? 0x87 : 0x8F;
        return condition.range_is_unsigned ? 0x83 : 0x8D;
    }

    void emit_aggregate_enum_payload_binding(SourceLocation loc,
                                             int match_base_offset,
                                             const IrType& enum_type,
                                             std::uint32_t payload_index,
                                             const std::string& name,
                                             const IrType& type) {
        emit_load_aggregate_enum_payload_slot(payload_index, loc, match_base_offset, enum_type);
        emit_cast_aggregate_enum_payload_slot_to_type(loc, type);
        emit_store_rax_to_local(local_offset(loc, name), type);
    }

    template <typename Arm>
    void emit_aggregate_enum_match_arm_bindings(const Arm& arm,
                                                int match_base_offset,
                                                const IrType& enum_type) {
        require_aggregate_enum_match_arm_supported(arm);

        if (arm.has_value_binding) {
            copy_local_bytes_to_offset(arm.loc,
                                       match_base_offset,
                                       local_offset(arm.loc, arm.value_name),
                                       arm.value_type);
        }

        if (!arm.payload_bindings.empty()) {
            for (const auto& binding : arm.payload_bindings) {
                if (binding.compact_enum_payload) {
                    emit_load_aggregate_enum_payload_slot(binding.index, arm.loc, match_base_offset, enum_type);
                    emit_shr_rax_imm8(32);
                    emit_cast_aggregate_enum_payload_slot_to_type(arm.loc, binding.type);
                    emit_store_rax_to_local(local_offset(arm.loc, binding.name), binding.type);
                    continue;
                }
                emit_aggregate_enum_payload_binding(arm.loc,
                                                    match_base_offset,
                                                    enum_type,
                                                    binding.index,
                                                    binding.name,
                                                    binding.type);
            }
            return;
        }

        if (arm.has_payload_binding) {
            emit_aggregate_enum_payload_binding(arm.loc,
                                                match_base_offset,
                                                enum_type,
                                                arm.payload_index,
                                                arm.payload_name,
                                                arm.payload_type);
        }
    }

    void emit_aggregate_enum_match(const IrStmt& stmt) {
        if (!stmt.match_value) throw CompileError(where(stmt.loc) + ": match value missing during codegen");
        int match_base_offset = aggregate_enum_match_base_offset(*stmt.match_value);
        const IrType& enum_type = stmt.match_value->type;

        std::vector<std::size_t> end_patches;
        for (const auto& arm : stmt.match_arms) {
            bool has_next_patch = false;
            std::vector<std::size_t> next_patches;
            if (!arm.wildcard) {
                next_patches = emit_aggregate_enum_match_arm_fail_jumps(arm, match_base_offset, enum_type);
                has_next_patch = true;
            }

            emit_aggregate_enum_match_arm_bindings(arm, match_base_offset, enum_type);
            emit_statements(arm.body);
            end_patches.push_back(emit_jmp_placeholder());

            if (has_next_patch) {
                for (std::size_t patch : next_patches) patch_rel32(patch, out_.size());
            }
        }

        std::size_t end = out_.size();
        for (std::size_t patch : end_patches) patch_rel32(patch, end);
    }

    template <typename Arm>
    void emit_match_arm_bindings(const Arm& arm) {
        if (arm.has_value_binding) {
            emit_mov_reg_rsp(Reg::RAX);
            emit_store_rax_to_local(local_offset(arm.loc, arm.value_name), arm.value_type);
        }
        if (arm.has_payload_binding) {
            emit_mov_reg_rsp(Reg::RAX);
            emit_shr_rax_imm8(32);
            emit_cast_payload_to_type(arm.loc, arm.payload_type);
            emit_store_rax_to_local(local_offset(arm.loc, arm.payload_name), arm.payload_type);
        }
    }

    void emit_expr(const IrExpr& expr) {
        switch (expr.kind) {
            case IrExprKind::Integer:
                emit_mov_reg_imm64(Reg::RAX, expr.int_negative ? 0 - expr.int_value : expr.int_value);
                break;
            case IrExprKind::Float:
                throw CompileError(where(expr.loc) + ": backend does not lower float values yet");
            case IrExprKind::String:
                throw CompileError(where(expr.loc) + ": backend does not lower string values yet");
            case IrExprKind::Bool:
                emit_mov_reg_imm64(Reg::RAX, expr.bool_value ? 1 : 0);
                break;
            case IrExprKind::Null:
                emit_mov_reg_imm64(Reg::RAX, 0);
                break;
            case IrExprKind::FunctionRef:
                emit_function_ref(expr);
                break;
            case IrExprKind::Local:
                if (is_aggregate_type(expr.type)) {
                    throw CompileError(where(expr.loc) + ": backend cannot materialize aggregate values; use field or index access");
                }
                emit_load_rax_from_local(local_offset(expr.loc, expr.name), expr.type);
                break;
            case IrExprKind::Borrow:
                emit_lea_reg_local(Reg::RAX, expr.operand ? lvalue_offset(*expr.operand) : local_offset(expr.loc, expr.name));
                break;
            case IrExprKind::Unary:
                emit_unary(expr);
                break;
            case IrExprKind::Cast:
                emit_cast(expr);
                break;
            case IrExprKind::PointerOffset:
                emit_pointer_offset(expr);
                break;
            case IrExprKind::PointerAdd:
                emit_pointer_add(expr);
                break;
            case IrExprKind::PointerLoad:
                emit_pointer_load(expr);
                break;
            case IrExprKind::PointerStore:
                emit_pointer_store(expr);
                break;
            case IrExprKind::Try:
                emit_try(expr);
                break;
            case IrExprKind::NullCoalesce:
                emit_null_coalesce(expr);
                break;
            case IrExprKind::EnumConstruct:
                emit_enum_construct(expr);
                break;
            case IrExprKind::Tuple:
                if (expr.args.empty()) {
                    emit_mov_reg_imm64(Reg::RAX, 0);
                    break;
                }
                throw CompileError(where(expr.loc) + ": backend cannot materialize tuple values; bind the tuple or index it");
            case IrExprKind::TupleIndex:
                emit_tuple_index(expr);
                break;
            case IrExprKind::Index:
                if (expr.operand && expr.operand->type.primitive == IrPrimitiveKind::Array) {
                    emit_array_index(expr);
                    break;
                }
                throw CompileError(where(expr.loc) + ": backend does not lower vector indexing yet");
            case IrExprKind::SliceRange:
                throw CompileError(where(expr.loc) + ": freestanding backend does not lower Slice range expressions yet");
            case IrExprKind::Vector:
                if (expr.type.primitive == IrPrimitiveKind::Array) {
                    throw CompileError(where(expr.loc) + ": backend cannot materialize array values; bind the array or index it");
                }
                throw CompileError(where(expr.loc) + ": backend does not lower vector values yet");
            case IrExprKind::VectorPush:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.push yet");
            case IrExprKind::VectorPop:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.pop yet");
            case IrExprKind::VectorClear:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.clear yet");
            case IrExprKind::VectorTruncate:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.truncate yet");
            case IrExprKind::VectorSet:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.set yet");
            case IrExprKind::VectorSwap:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.swap yet");
            case IrExprKind::VectorRemove:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.remove yet");
            case IrExprKind::VectorInsert:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.insert yet");
            case IrExprKind::VectorContains:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.contains yet");
            case IrExprKind::VectorIndexOf:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.index_of yet");
            case IrExprKind::VectorCount:
                throw CompileError(where(expr.loc) + ": backend does not lower Vec.count yet");
            case IrExprKind::Noop:
                emit_mov_reg_imm64(Reg::RAX, 0);
                break;
            case IrExprKind::FormatPrint:
                emit_format_print(expr);
                break;
            case IrExprKind::Match:
                emit_match_expr(expr);
                break;
            case IrExprKind::If:
                emit_if_expr(expr);
                break;
            case IrExprKind::Block:
                emit_block_expr(expr);
                break;
            case IrExprKind::IndirectCall:
                emit_indirect_call(expr);
                break;
            case IrExprKind::TraitObjectCall:
                throw CompileError(where(expr.loc) + ": backend does not lower trait object dispatch yet");
            case IrExprKind::Call:
                emit_call(expr);
                break;
            case IrExprKind::Binary:
                emit_binary(expr);
                break;
        }
    }

    void emit_tuple_index(const IrExpr& expr) {
        if (is_pointer_backed_lvalue(expr)) {
            if (is_aggregate_type(expr.type)) {
                throw CompileError(where(expr.loc) + ": backend cannot materialize nested aggregate values; index a scalar field");
            }
            require_pointer_scalar_codegen(expr.loc, expr.type, "pointer loads");
            emit_pointer_lvalue_address(expr);
            emit_load_rax_from_ptr(Reg::RAX, expr.type);
            return;
        }

        const IrExpr& operand = *expr.operand;
        if (operand.kind == IrExprKind::Local || operand.kind == IrExprKind::TupleIndex) {
            int base = lvalue_offset(operand);
            int offset = aggregate_lvalue_field_offset(expr.loc, base, operand.type, expr.tuple_index);
            if (expr.type.primitive == IrPrimitiveKind::Tuple ||
                expr.type.primitive == IrPrimitiveKind::Array ||
                expr.type.primitive == IrPrimitiveKind::Struct) {
                throw CompileError(where(expr.loc) + ": backend cannot materialize nested aggregate values; index a scalar field");
            }
            emit_load_rax_from_local(offset, expr.type);
            return;
        }

        if (operand.kind == IrExprKind::Tuple || operand.kind == IrExprKind::Vector) {
            const IrExpr& item = *operand.args[static_cast<std::size_t>(expr.tuple_index)];
            if (expr.type.primitive == IrPrimitiveKind::Tuple ||
                expr.type.primitive == IrPrimitiveKind::Array ||
                expr.type.primitive == IrPrimitiveKind::Struct) {
                throw CompileError(where(expr.loc) + ": backend cannot materialize nested aggregate values; index a scalar field");
            }
            emit_expr(item);
            return;
        }

        throw CompileError(where(expr.loc) + ": backend can only index tuple locals and tuple literals");
    }

    void emit_array_index(const IrExpr& expr) {
        if (is_pointer_backed_lvalue(expr)) {
            if (is_aggregate_type(expr.type)) {
                throw CompileError(where(expr.loc) + ": backend cannot materialize nested aggregate values; index a scalar field");
            }
            require_pointer_scalar_codegen(expr.loc, expr.type, "pointer loads");
            emit_pointer_lvalue_address(expr);
            emit_load_rax_from_ptr(Reg::RAX, expr.type);
            return;
        }

        if (!expr.operand || expr.operand->kind != IrExprKind::Local) {
            throw CompileError(where(expr.loc) + ": backend can only dynamically index local arrays yet");
        }
        if (is_aggregate_type(expr.type)) {
            throw CompileError(where(expr.loc) + ": backend cannot materialize nested aggregate values; index a scalar field");
        }
        int base = local_offset(expr.operand->loc, expr.operand->name);
        int stride = layout_size_bytes(expr.loc, expr.type);
        emit_expr(*expr.right);
        emit_array_bounds_check(expr.operand->type.array_size);
        emit_mov_reg_imm64(Reg::RCX, static_cast<std::uint64_t>(stride));
        emit_imul_reg_reg(Reg::RAX, Reg::RCX);
        emit_lea_reg_local(Reg::RCX, base);
        emit_add_reg_reg(Reg::RCX, Reg::RAX);
        emit_load_rax_from_ptr(Reg::RCX, expr.type);
    }

    void emit_array_bounds_check(std::uint64_t array_size) {
        emit_mov_reg_imm64(Reg::RCX, 0);
        emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
        std::size_t fail_low = emit_jcc_placeholder(0x8C);

        emit_mov_reg_imm64(Reg::RCX, array_size);
        emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
        std::size_t fail_high = emit_jcc_placeholder(0x8D);

        std::size_t jump_ok = emit_jmp_placeholder();
        std::size_t fail = out_.size();
        patch_rel32(fail_low, fail);
        patch_rel32(fail_high, fail);
        emit_direct_call("panic");

        std::size_t ok = out_.size();
        patch_rel32(jump_ok, ok);
    }

    int lvalue_offset(const IrExpr& expr) const {
        if (expr.kind == IrExprKind::Local) return local_offset(expr.loc, expr.name);
        if (expr.kind == IrExprKind::TupleIndex && expr.operand) {
            int base = lvalue_offset(*expr.operand);
            return aggregate_lvalue_field_offset(expr.loc, base, expr.operand->type, expr.tuple_index);
        }
        throw CompileError(where(expr.loc) + ": backend can only assign to local fields yet");
    }

    void emit_match_expr(const IrExpr& expr) {
        if (expr.type.primitive == IrPrimitiveKind::Tuple) {
            throw CompileError(where(expr.loc) + ": backend cannot materialize tuple-valued match expressions yet");
        }
        if (expr.match_arms.empty()) throw CompileError(where(expr.loc) + ": match expression has no arms during codegen");

        if (expr.match_value && has_aggregate_enum_layout(expr.match_value->type)) {
            emit_aggregate_enum_match_expr(expr);
            return;
        }

        emit_expr(*expr.match_value);
        emit_push(Reg::RAX);

        std::vector<std::size_t> end_patches;
        for (std::size_t i = 0; i < expr.match_arms.size(); ++i) {
            const IrMatchExprArm& arm = expr.match_arms[i];
            bool has_next_patch = false;
            std::vector<std::size_t> next_patches;

            if (!arm.wildcard && i + 1 != expr.match_arms.size()) {
                next_patches = emit_match_arm_fail_jumps(arm);
                has_next_patch = true;
            }

            if (arm.has_value_binding) {
                emit_mov_reg_rsp(Reg::RAX);
                emit_store_rax_to_local(local_offset(arm.loc, arm.value_name), arm.value_type);
            }
            if (arm.has_payload_binding) {
                emit_mov_reg_rsp(Reg::RAX);
                emit_shr_rax_imm8(32);
                emit_cast_payload_to_type(arm.loc, arm.payload_type);
                emit_store_rax_to_local(local_offset(arm.loc, arm.payload_name), arm.payload_type);
            }
            emit_statements(arm.body);
            emit_expr(*arm.value);
            emit_pop(Reg::RCX);
            end_patches.push_back(emit_jmp_placeholder());

            if (has_next_patch) {
                for (std::size_t patch : next_patches) patch_rel32(patch, out_.size());
            }
        }

        std::size_t end = out_.size();
        for (std::size_t patch : end_patches) patch_rel32(patch, end);
    }

    void emit_aggregate_enum_match_expr(const IrExpr& expr) {
        if (!expr.match_value) throw CompileError(where(expr.loc) + ": match value missing during codegen");
        int match_base_offset = aggregate_enum_match_base_offset(*expr.match_value);
        const IrType& enum_type = expr.match_value->type;

        std::vector<std::size_t> end_patches;
        for (std::size_t i = 0; i < expr.match_arms.size(); ++i) {
            const IrMatchExprArm& arm = expr.match_arms[i];
            bool has_next_patch = false;
            std::vector<std::size_t> next_patches;

            if (!arm.wildcard && i + 1 != expr.match_arms.size()) {
                next_patches = emit_aggregate_enum_match_arm_fail_jumps(arm, match_base_offset, enum_type);
                has_next_patch = true;
            }

            emit_aggregate_enum_match_arm_bindings(arm, match_base_offset, enum_type);
            emit_statements(arm.body);
            emit_expr(*arm.value);
            end_patches.push_back(emit_jmp_placeholder());

            if (has_next_patch) {
                for (std::size_t patch : next_patches) patch_rel32(patch, out_.size());
            }
        }

        std::size_t end = out_.size();
        for (std::size_t patch : end_patches) patch_rel32(patch, end);
    }

    template <typename Arm>
    std::vector<std::size_t> emit_match_arm_fail_jumps(const Arm& arm) {
        if (!arm.payload_literal_conditions.empty() || !arm.payload_enum_conditions.empty()) {
            throw CompileError(where(arm.loc) + ": freestanding backend does not lower aggregate enum payload test patterns yet");
        }
        if (!arm.payload_range_conditions.empty()) return emit_compact_payload_range_fail_jumps(arm);
        if (arm.has_range) return emit_match_range_fail_jumps(arm);
        emit_match_arm_test(arm);
        return {emit_jcc_placeholder(0x85)};
    }

    template <typename Arm>
    std::vector<std::size_t> emit_compact_payload_range_fail_jumps(const Arm& arm) {
        std::vector<std::size_t> patches;
        emit_match_arm_test(arm);
        patches.push_back(emit_jcc_placeholder(0x85));

        for (const auto& condition : arm.payload_range_conditions) {
            if (!condition.compact_enum_payload || condition.index != 0) {
                throw CompileError(where(arm.loc) + ": freestanding backend does not lower aggregate enum payload test patterns yet");
            }

            emit_mov_reg_rsp(Reg::RAX);
            emit_shr_rax_imm8(32);
            emit_cast_payload_to_type(arm.loc, condition.type);
            emit_mov_reg_imm64(Reg::RCX, payload_range_start_bits(condition));
            emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
            patches.push_back(emit_jcc_placeholder(condition.is_unsigned ? 0x82 : 0x8C));

            emit_mov_reg_rsp(Reg::RAX);
            emit_shr_rax_imm8(32);
            emit_cast_payload_to_type(arm.loc, condition.type);
            emit_mov_reg_imm64(Reg::RCX, payload_range_end_bits(condition));
            emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
            patches.push_back(emit_jcc_placeholder(payload_range_past_end_cc(condition)));
        }

        return patches;
    }

    template <typename Arm>
    std::vector<std::size_t> emit_match_range_fail_jumps(const Arm& arm) {
        emit_mov_reg_rsp(Reg::RAX);
        emit_mov_reg_imm64(Reg::RCX, match_range_start_bits(arm));
        emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
        std::size_t below_start = emit_jcc_placeholder(arm.range_is_unsigned ? 0x82 : 0x8C);

        emit_mov_reg_rsp(Reg::RAX);
        emit_mov_reg_imm64(Reg::RCX, match_range_end_bits(arm));
        emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
        std::size_t past_end = emit_jcc_placeholder(range_past_end_cc(arm));
        return {below_start, past_end};
    }

    template <typename Arm>
    void emit_match_arm_test(const Arm& arm) {
        emit_mov_reg_rsp(Reg::RAX);
        if (arm.has_literal) {
            emit_mov_reg_imm64(Reg::RCX, match_literal_bits(arm));
        } else {
            emit_mov_reg_imm64(Reg::RCX, 0xffffffffULL);
            emit_and_reg_reg(Reg::RAX, Reg::RCX);
            emit_mov_reg_imm64(Reg::RCX, arm.enum_tag);
        }
        emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
    }

    template <typename Arm>
    static std::uint64_t match_literal_bits(const Arm& arm) {
        if (arm.literal_is_bool) return arm.literal_bool ? 1 : 0;
        return arm.literal_negative ? 0 - arm.literal_int : arm.literal_int;
    }

    template <typename Arm>
    static std::uint64_t match_range_start_bits(const Arm& arm) {
        return arm.range_start_negative ? 0 - arm.range_start_int : arm.range_start_int;
    }

    template <typename Arm>
    static std::uint64_t match_range_end_bits(const Arm& arm) {
        return arm.range_end_negative ? 0 - arm.range_end_int : arm.range_end_int;
    }

    template <typename Arm>
    static std::uint8_t range_past_end_cc(const Arm& arm) {
        if (arm.range_inclusive) return arm.range_is_unsigned ? 0x87 : 0x8F;
        return arm.range_is_unsigned ? 0x83 : 0x8D;
    }

    static std::uint64_t payload_range_start_bits(const IrPayloadRangeCondition& condition) {
        return condition.start_negative ? 0 - condition.start_int : condition.start_int;
    }

    static std::uint64_t payload_range_end_bits(const IrPayloadRangeCondition& condition) {
        return condition.end_negative ? 0 - condition.end_int : condition.end_int;
    }

    static std::uint8_t payload_range_past_end_cc(const IrPayloadRangeCondition& condition) {
        if (condition.inclusive) return condition.is_unsigned ? 0x87 : 0x8F;
        return condition.is_unsigned ? 0x83 : 0x8D;
    }

    void emit_if_expr(const IrExpr& expr) {
        if (expr.type.primitive == IrPrimitiveKind::Tuple) {
            throw CompileError(where(expr.loc) + ": backend cannot materialize tuple-valued if expressions yet");
        }
        emit_expr(*expr.condition);
        emit_cmp_rax_zero();
        std::size_t jump_else = emit_jcc_placeholder(0x84);
        emit_statements(expr.then_body);
        emit_expr(*expr.then_value);
        std::size_t jump_end = emit_jmp_placeholder();
        patch_rel32(jump_else, out_.size());
        emit_statements(expr.else_body);
        emit_expr(*expr.else_value);
        patch_rel32(jump_end, out_.size());
    }

    void emit_block_expr(const IrExpr& expr) {
        if (is_aggregate_type(expr.type)) {
            throw CompileError(where(expr.loc) + ": backend cannot materialize aggregate-valued block expressions here; bind the block result first");
        }
        if (!expr.label.empty()) {
            LoopLabels labels;
            labels.source_label = expr.label;
            labels.is_loop = false;
            loops_.push_back(labels);
            emit_statements(expr.block_body);
            emit_expr(*expr.block_value);
            std::size_t end = out_.size();
            for (std::size_t patch : loops_.back().break_patches) patch_rel32(patch, end);
            loops_.pop_back();
            return;
        }
        emit_statements(expr.block_body);
        emit_expr(*expr.block_value);
    }

    void emit_call(const IrExpr& expr) {
        static const Reg arg_regs[] = {Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9};
        reject_c_extern_use(expr.loc, expr.name, "call");
        if (std::optional<std::string> blocked = ari_builtin_freestanding_blocked_feature(expr.name)) {
            throw CompileError(where(expr.loc) + ": freestanding backend does not lower " +
                               *blocked + " yet; use the LLVM host backend");
        }
        if (expr.args.size() > static_cast<std::size_t>(0xffff)) {
            throw CompileError(where(expr.loc) + ": backend supports up to 65535 call arguments");
        }

        int arg_area = static_cast<int>(expr.args.size() * 8);
        if (arg_area > 0) emit_sub_rsp(arg_area);
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            emit_expr(*expr.args[i]);
            emit_mov_mem_rsp_reg(static_cast<int>(i * 8), Reg::RAX);
        }
        std::size_t reg_count = expr.args.size() < 6 ? expr.args.size() : 6;
        for (std::size_t i = 0; i < reg_count; ++i) {
            emit_mov_reg_mem_rsp(arg_regs[i], static_cast<int>(i * 8));
        }
        emit_add_rsp(static_cast<int>(reg_count * 8));
        out_.u8(0xE8);
        std::size_t pos = out_.size();
        out_.u32(0);
        calls_.push_back(CallPatch{pos, expr.name});
        if (expr.args.size() > reg_count) emit_add_rsp(static_cast<int>((expr.args.size() - reg_count) * 8));
    }

    void emit_function_ref(const IrExpr& expr) {
        reject_c_extern_use(expr.loc, expr.name, "take the address of");
        out_.u8(0x48);
        out_.u8(0x8D);
        out_.u8(0x05);
        std::size_t pos = out_.size();
        out_.u32(0);
        addresses_.push_back(CallPatch{pos, expr.name});
    }

    void reject_c_extern_use(SourceLocation loc, const std::string& name, const std::string& operation) const {
        auto found = extern_abis_.find(name);
        if (found == extern_abis_.end() || found->second != IrExternAbi::C) return;
        throw CompileError(where(loc) + ": freestanding backend cannot " + operation +
                           " extern \"C\" function '" + name +
                           "'; use the LLVM host backend or provide an Ari runtime shim");
    }

    void emit_indirect_call(const IrExpr& expr) {
        static const Reg arg_regs[] = {Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9};
        if (expr.operand->kind != IrExprKind::Local && expr.operand->kind != IrExprKind::FunctionRef) {
            throw CompileError(where(expr.loc) + ": freestanding backend only lowers direct function pointer calls yet");
        }
        if (expr.args.size() > static_cast<std::size_t>(0xffff)) {
            throw CompileError(where(expr.loc) + ": backend supports up to 65535 call arguments");
        }

        int arg_area = static_cast<int>(expr.args.size() * 8);
        if (arg_area > 0) emit_sub_rsp(arg_area);
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            emit_expr(*expr.args[i]);
            emit_mov_mem_rsp_reg(static_cast<int>(i * 8), Reg::RAX);
        }
        std::size_t reg_count = expr.args.size() < 6 ? expr.args.size() : 6;
        for (std::size_t i = 0; i < reg_count; ++i) {
            emit_mov_reg_mem_rsp(arg_regs[i], static_cast<int>(i * 8));
        }
        emit_add_rsp(static_cast<int>(reg_count * 8));
        emit_expr(*expr.operand);
        out_.u8(0xFF);
        out_.u8(0xD0);
        if (expr.args.size() > reg_count) emit_add_rsp(static_cast<int>((expr.args.size() - reg_count) * 8));
    }

    void emit_direct_call(const std::string& name) {
        out_.u8(0xE8);
        std::size_t pos = out_.size();
        out_.u32(0);
        calls_.push_back(CallPatch{pos, name});
    }

    void emit_format_literal(const std::string& text) {
        for (unsigned char c : text) {
            emit_mov_reg_imm64(Reg::RDI, c);
            emit_direct_call("write_byte");
        }
    }

    void emit_format_argument(const IrExpr& expr) {
        emit_expr(expr);
        if (expr.type.qualifier == TypeQualifier::Value && expr.type.primitive == IrPrimitiveKind::Bool) {
            emit_mov_reg_reg(Reg::RDI, Reg::RAX);
            emit_direct_call("write_bool");
            return;
        }
        emit_cast_to_type(expr.loc, expr.type);
        emit_mov_reg_reg(Reg::RDI, Reg::RAX);
        emit_direct_call("write_i64");
    }

    void emit_format_print(const IrExpr& expr) {
        for (std::size_t i = 0; i < expr.format_parts.size(); ++i) {
            emit_format_literal(expr.format_parts[i]);
            if (i < expr.args.size()) emit_format_argument(*expr.args[i]);
        }
        if (expr.print_newline) {
            emit_direct_call("newline");
        }
        emit_mov_reg_imm64(Reg::RAX, 0);
    }

    void emit_unary(const IrExpr& expr) {
        switch (expr.unary_op) {
            case IrUnaryOp::Not:
                emit_expr(*expr.operand);
                emit_cmp_rax_zero();
                emit_setcc(0x94);
                break;
            case IrUnaryOp::BitNot:
                emit_expr(*expr.operand);
                out_.u8(0x48);
                out_.u8(0xF7);
                out_.u8(0xD0);
                emit_cast_to_type(expr.loc, expr.type);
                break;
        }
    }

    void emit_cast(const IrExpr& expr) {
        emit_expr(*expr.operand);
        emit_cast_to_type(expr.loc, expr.type);
    }

    void emit_pointer_offset(const IrExpr& expr) {
        emit_expr(*expr.operand);
        emit_push(Reg::RAX);
        emit_expr(*expr.right);
        emit_pop(Reg::RCX);
        emit_add_reg_reg(Reg::RAX, Reg::RCX);
    }

    void emit_pointer_add(const IrExpr& expr) {
        IrType element_type = expr.type;
        element_type.qualifier = TypeQualifier::Value;
        emit_expr(*expr.operand);
        emit_push(Reg::RAX);
        emit_expr(*expr.right);
        emit_mov_reg_imm64(Reg::RCX, static_cast<std::uint64_t>(raw_pointer_stride_bytes(element_type)));
        emit_imul_reg_reg(Reg::RAX, Reg::RCX);
        emit_pop(Reg::RCX);
        emit_add_reg_reg(Reg::RAX, Reg::RCX);
    }

    void emit_pointer_load(const IrExpr& expr) {
        require_pointer_scalar_codegen(expr.loc, expr.type, "pointer loads");
        emit_expr(*expr.operand);
        emit_load_rax_from_ptr(Reg::RAX, expr.type);
    }

    void emit_pointer_store(const IrExpr& expr) {
        if (!expr.right) {
            throw CompileError(where(expr.loc) + ": malformed pointer store expression");
        }
        emit_store_to_pointer(expr.loc, *expr.operand, *expr.right, expr.right->type);
    }

    void emit_store_to_pointer_target(const IrExpr& target, const IrExpr& value) {
        if (!target.operand) {
            throw CompileError(where(target.loc) + ": malformed pointer dereference assignment target");
        }
        emit_store_to_pointer(target.loc, *target.operand, value, target.type);
    }

    void emit_store_to_pointer_lvalue_target(const IrExpr& target, const IrExpr& value) {
        if (is_aggregate_type(target.type)) {
            emit_pointer_lvalue_address(target);
            emit_mov_reg_reg(Reg::RBX, Reg::RAX);
            emit_store_value_to_pointer_base(target.loc, value, target.type, 0);
            return;
        }

        require_pointer_scalar_codegen(target.loc, target.type, "pointer stores");
        emit_pointer_lvalue_address(target);
        emit_push(Reg::RAX);
        emit_expr(value);
        if (is_integer_primitive(target.type.primitive)) emit_cast_to_type(target.loc, target.type);
        emit_pop(Reg::RCX);
        emit_store_rax_to_ptr(Reg::RCX, target.type);
    }

    void emit_store_to_pointer(SourceLocation loc,
                               const IrExpr& pointer,
                               const IrExpr& value,
                               const IrType& element_type) {
        if (is_aggregate_type(element_type)) {
            emit_expr(pointer);
            emit_mov_reg_reg(Reg::RBX, Reg::RAX);
            emit_store_value_to_pointer_base(loc, value, element_type, 0);
            return;
        }

        require_pointer_scalar_codegen(loc, element_type, "pointer stores");
        emit_expr(pointer);
        emit_push(Reg::RAX);
        emit_expr(value);
        if (is_integer_primitive(element_type.primitive)) emit_cast_to_type(loc, element_type);
        emit_pop(Reg::RCX);
        emit_store_rax_to_ptr(Reg::RCX, element_type);
    }

    void emit_normalize_return_value(SourceLocation loc) {
        if (fn_.return_type.qualifier == TypeQualifier::Value &&
            is_integer_primitive(fn_.return_type.primitive)) {
            emit_cast_to_type(loc, fn_.return_type);
            return;
        }
        if (fn_.return_type.qualifier == TypeQualifier::Value &&
            fn_.return_type.primitive == IrPrimitiveKind::Bool) {
            emit_cmp_rax_zero();
            emit_setcc(0x95);
        }
    }

    void require_pointer_scalar_codegen(SourceLocation loc, const IrType& type, const std::string& operation) const {
        if (is_aggregate_type(type)) {
            throw CompileError(where(loc) + ": freestanding backend can only materialize raw-pointer aggregates when copying into a local or pointer target");
        }
        if (type.primitive == IrPrimitiveKind::F32 ||
            type.primitive == IrPrimitiveKind::F64 ||
            type.primitive == IrPrimitiveKind::F128 ||
            type.primitive == IrPrimitiveKind::String) {
            throw CompileError(where(loc) + ": freestanding backend does not lower " + operation + " for " + type_name(type) + " yet");
        }
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

    void emit_pointer_lvalue_address(const IrExpr& expr) {
        if (expr.kind == IrExprKind::PointerLoad && expr.operand) {
            emit_expr(*expr.operand);
            return;
        }
        if (expr.kind == IrExprKind::TupleIndex && expr.operand) {
            emit_pointer_lvalue_address(*expr.operand);
            int offset = field_offset_bytes(expr.loc, expr.operand->type, expr.tuple_index);
            if (offset != 0) {
                emit_mov_reg_imm64(Reg::RCX, static_cast<std::uint64_t>(offset));
                emit_add_reg_reg(Reg::RAX, Reg::RCX);
            }
            return;
        }
        if (expr.kind == IrExprKind::Index && expr.operand && expr.right) {
            if (expr.operand->type.primitive != IrPrimitiveKind::Array) {
                throw CompileError(where(expr.loc) + ": freestanding backend can only dynamically index raw pointers to fixed arrays yet");
            }
            emit_pointer_lvalue_address(*expr.operand);
            emit_push(Reg::RAX);
            emit_expr(*expr.right);
            emit_array_bounds_check(expr.operand->type.array_size);
            emit_mov_reg_imm64(Reg::RCX, static_cast<std::uint64_t>(layout_size_bytes(expr.loc, expr.type)));
            emit_imul_reg_reg(Reg::RAX, Reg::RCX);
            emit_pop(Reg::RCX);
            emit_add_reg_reg(Reg::RCX, Reg::RAX);
            emit_mov_reg_reg(Reg::RAX, Reg::RCX);
            return;
        }
        throw CompileError(where(expr.loc) + ": freestanding backend can only address raw-pointer lvalues");
    }

    void emit_cast_to_type(SourceLocation loc, const IrType& type) {
        if (type.qualifier == TypeQualifier::Ptr) return;
        switch (type.primitive) {
            case IrPrimitiveKind::I8:
                out_.u8(0x48);
                out_.u8(0x0F);
                out_.u8(0xBE);
                out_.u8(0xC0);
                break;
            case IrPrimitiveKind::U8:
                out_.u8(0x0F);
                out_.u8(0xB6);
                out_.u8(0xC0);
                break;
            case IrPrimitiveKind::I16:
                out_.u8(0x48);
                out_.u8(0x0F);
                out_.u8(0xBF);
                out_.u8(0xC0);
                break;
            case IrPrimitiveKind::U16:
                out_.u8(0x0F);
                out_.u8(0xB7);
                out_.u8(0xC0);
                break;
            case IrPrimitiveKind::I32:
                out_.u8(0x48);
                out_.u8(0x63);
                out_.u8(0xC0);
                break;
            case IrPrimitiveKind::U32:
                out_.u8(0x89);
                out_.u8(0xC0);
                break;
            case IrPrimitiveKind::I64:
            case IrPrimitiveKind::U64:
                break;
            default:
                throw CompileError(where(loc) + ": backend only lowers integer casts yet");
        }
    }

    void emit_cast_payload_to_type(SourceLocation loc, const IrType& type) {
        if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) return;
        emit_cast_to_type(loc, type);
    }

    void emit_enum_construct(const IrExpr& expr) {
        if (has_aggregate_enum_layout(expr.type)) {
            throw CompileError(where(expr.loc) + ": freestanding backend does not lower multi-payload enum values yet");
        }
        if (!expr.has_payload) {
            emit_mov_reg_imm64(Reg::RAX, expr.enum_tag);
            return;
        }

        emit_expr(*expr.payload);
        emit_cast_payload_to_type(expr.loc, expr.payload_type);
        emit_shl_rax_imm8(32);
        emit_mov_reg_imm64(Reg::RCX, expr.enum_tag);
        emit_or_reg_reg(Reg::RAX, Reg::RCX);
    }

    void emit_try(const IrExpr& expr) {
        emit_expr(*expr.operand);
        emit_push(Reg::RAX);
        emit_mov_reg_rsp(Reg::RAX);
        emit_mov_reg_imm64(Reg::RCX, 0xffffffffULL);
        emit_and_reg_reg(Reg::RAX, Reg::RCX);
        emit_mov_reg_imm64(Reg::RCX, expr.enum_tag);
        emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
        std::size_t jump_fail = emit_jcc_placeholder(0x85);

        emit_mov_reg_rsp(Reg::RAX);
        emit_shr_rax_imm8(32);
        emit_cast_payload_to_type(expr.loc, expr.payload_type);
        emit_pop(Reg::RCX);
        std::size_t jump_end = emit_jmp_placeholder();

        patch_rel32(jump_fail, out_.size());
        if (expr.try_converts_residual) {
            if (expr.try_residual_has_payload) {
                emit_mov_reg_rsp(Reg::RAX);
                emit_shr_rax_imm8(32);
                emit_cast_payload_to_type(expr.loc, expr.try_return_residual_payload_type);
                emit_shl_rax_imm8(32);
                emit_mov_reg_imm64(Reg::RCX, expr.try_return_residual_tag);
                emit_or_reg_reg(Reg::RAX, Reg::RCX);
                emit_pop(Reg::RCX);
            } else {
                emit_pop(Reg::RCX);
                emit_mov_reg_imm64(Reg::RAX, expr.try_return_residual_tag);
            }
        } else {
            emit_pop(Reg::RAX);
        }
        return_jumps_.push_back(emit_jmp_placeholder());

        patch_rel32(jump_end, out_.size());
    }

    void emit_null_coalesce(const IrExpr& expr) {
        emit_expr(*expr.left);
        emit_push(Reg::RAX);
        emit_mov_reg_rsp(Reg::RAX);
        emit_mov_reg_imm64(Reg::RCX, 0xffffffffULL);
        emit_and_reg_reg(Reg::RAX, Reg::RCX);
        emit_mov_reg_imm64(Reg::RCX, expr.enum_tag);
        emit_cmp_reg_reg(Reg::RAX, Reg::RCX);
        std::size_t jump_fallback = emit_jcc_placeholder(0x85);

        emit_mov_reg_rsp(Reg::RAX);
        emit_shr_rax_imm8(32);
        emit_cast_payload_to_type(expr.loc, expr.payload_type);
        emit_pop(Reg::RCX);
        std::size_t jump_end = emit_jmp_placeholder();

        patch_rel32(jump_fallback, out_.size());
        emit_pop(Reg::RCX);
        emit_expr(*expr.right);
        emit_cast_payload_to_type(expr.loc, expr.type);

        patch_rel32(jump_end, out_.size());
    }

    void emit_binary(const IrExpr& expr) {
        if (expr.op == IrBinaryOp::LogicalAnd || expr.op == IrBinaryOp::LogicalOr) {
            emit_logical(expr);
            return;
        }

        emit_expr(*expr.left);
        emit_push(Reg::RAX);
        emit_expr(*expr.right);
        emit_pop(Reg::RCX);

        switch (expr.op) {
            case IrBinaryOp::Add:
                emit_add_reg_reg(Reg::RAX, Reg::RCX);
                break;
            case IrBinaryOp::Sub:
                emit_sub_reg_reg(Reg::RCX, Reg::RAX);
                emit_mov_reg_reg(Reg::RAX, Reg::RCX);
                break;
            case IrBinaryOp::Mul:
                emit_imul_reg_reg(Reg::RCX, Reg::RAX);
                emit_mov_reg_reg(Reg::RAX, Reg::RCX);
                break;
            case IrBinaryOp::Div:
            case IrBinaryOp::Mod:
                emit_mov_reg_reg(Reg::RBX, Reg::RAX);
                emit_mov_reg_reg(Reg::RAX, Reg::RCX);
                if (is_unsigned_integer_type(expr.left->type)) {
                    emit_xor_reg_reg(Reg::RDX, Reg::RDX);
                    out_.u8(0x48);
                    out_.u8(0xF7);
                    out_.u8(0xF3);
                } else {
                    out_.u8(0x48);
                    out_.u8(0x99);
                    out_.u8(0x48);
                    out_.u8(0xF7);
                    out_.u8(0xFB);
                }
                if (expr.op == IrBinaryOp::Mod) emit_mov_reg_reg(Reg::RAX, Reg::RDX);
                break;
            case IrBinaryOp::BitAnd:
                emit_and_reg_reg(Reg::RAX, Reg::RCX);
                break;
            case IrBinaryOp::BitOr:
                emit_or_reg_reg(Reg::RAX, Reg::RCX);
                break;
            case IrBinaryOp::BitXor:
                emit_xor_reg_reg(Reg::RAX, Reg::RCX);
                break;
            case IrBinaryOp::Shl:
                emit_mov_reg_reg(Reg::RDX, Reg::RAX);
                emit_mov_reg_reg(Reg::RAX, Reg::RCX);
                emit_mov_reg_reg(Reg::RCX, Reg::RDX);
                emit_shift_rax_cl(0xE0);
                break;
            case IrBinaryOp::Shr:
                emit_mov_reg_reg(Reg::RDX, Reg::RAX);
                emit_mov_reg_reg(Reg::RAX, Reg::RCX);
                emit_mov_reg_reg(Reg::RCX, Reg::RDX);
                emit_shift_rax_cl(is_unsigned_integer_type(expr.left->type) ? 0xE8 : 0xF8);
                break;
            case IrBinaryOp::Eq:
                emit_cmp_reg_reg(Reg::RCX, Reg::RAX);
                emit_setcc(0x94);
                break;
            case IrBinaryOp::Ne:
                emit_cmp_reg_reg(Reg::RCX, Reg::RAX);
                emit_setcc(0x95);
                break;
            case IrBinaryOp::Lt:
                emit_cmp_reg_reg(Reg::RCX, Reg::RAX);
                emit_setcc(0x9C);
                break;
            case IrBinaryOp::Le:
                emit_cmp_reg_reg(Reg::RCX, Reg::RAX);
                emit_setcc(0x9E);
                break;
            case IrBinaryOp::Gt:
                emit_cmp_reg_reg(Reg::RCX, Reg::RAX);
                emit_setcc(0x9F);
                break;
            case IrBinaryOp::Ge:
                emit_cmp_reg_reg(Reg::RCX, Reg::RAX);
                emit_setcc(0x9D);
                break;
            case IrBinaryOp::LogicalOr:
            case IrBinaryOp::LogicalAnd:
                break;
        }
    }

    void emit_logical(const IrExpr& expr) {
        if (expr.op == IrBinaryOp::LogicalAnd) {
            emit_expr(*expr.left);
            emit_cmp_rax_zero();
            std::size_t jump_false_left = emit_jcc_placeholder(0x84);
            emit_expr(*expr.right);
            emit_cmp_rax_zero();
            std::size_t jump_false_right = emit_jcc_placeholder(0x84);
            emit_mov_reg_imm64(Reg::RAX, 1);
            std::size_t jump_end = emit_jmp_placeholder();
            std::size_t false_target = out_.size();
            patch_rel32(jump_false_left, false_target);
            patch_rel32(jump_false_right, false_target);
            emit_mov_reg_imm64(Reg::RAX, 0);
            patch_rel32(jump_end, out_.size());
            return;
        }

        emit_expr(*expr.left);
        emit_cmp_rax_zero();
        std::size_t jump_true_left = emit_jcc_placeholder(0x85);
        emit_expr(*expr.right);
        emit_cmp_rax_zero();
        std::size_t jump_true_right = emit_jcc_placeholder(0x85);
        emit_mov_reg_imm64(Reg::RAX, 0);
        std::size_t jump_end = emit_jmp_placeholder();
        std::size_t true_target = out_.size();
        patch_rel32(jump_true_left, true_target);
        patch_rel32(jump_true_right, true_target);
        emit_mov_reg_imm64(Reg::RAX, 1);
        patch_rel32(jump_end, out_.size());
    }

    static bool is_unsigned_integer_type(const IrType& type) {
        if (type.qualifier != TypeQualifier::Value) return false;
        return type.primitive == IrPrimitiveKind::U8 ||
               type.primitive == IrPrimitiveKind::U16 ||
               type.primitive == IrPrimitiveKind::U32 ||
               type.primitive == IrPrimitiveKind::U64;
    }
};

class ProgramEmitter {
public:
    explicit ProgramEmitter(const IrProgram& program) : program_(program) {}

    EmittedProgram emit() {
        collect_extern_abis();
        emit_entry();
        std::vector<CompiledFunction> functions;
        for (const auto& fn : program_.functions) {
            FunctionEmitter emitter(fn, extern_abis_);
            functions.push_back(emitter.emit());
        }
        for (std::size_t i = 0; i < functions.size(); ++i) {
            auto& fn = functions[i];
            std::size_t offset = code_.size();
            function_offsets_[fn.name] = offset;
            for (std::uint8_t byte : fn.code) code_.u8(byte);
            const IrFunction& ir_fn = program_.functions[i];
            symbols_.push_back(CodeSymbol{
                ir_fn.link_name.empty() ? mangle_function_name(ir_fn.name) : ir_fn.link_name,
                static_cast<std::uint64_t>(offset),
                static_cast<std::uint64_t>(fn.code.size())
            });
            for (const auto& call : fn.calls) {
                global_calls_.push_back(CallPatch{function_offsets_[fn.name] + call.imm_offset, call.name});
            }
            for (const auto& address : fn.addresses) {
                global_addresses_.push_back(CallPatch{function_offsets_[fn.name] + address.imm_offset, address.name});
            }
        }
        emit_builtin_functions();
        patch_calls();
        return EmittedProgram{std::move(code_.bytes), std::move(symbols_)};
    }

private:
    const IrProgram& program_;
    CodeBuffer code_;
    std::map<std::string, std::size_t> function_offsets_;
    std::map<std::string, IrExternAbi> extern_abis_;
    std::vector<CallPatch> global_calls_;
    std::vector<CallPatch> global_addresses_;
    std::vector<CodeSymbol> symbols_;

    void collect_extern_abis() {
        for (const auto& fn : program_.extern_functions) {
            extern_abis_[fn.name] = fn.abi;
        }
    }

    std::size_t emit_jmp_placeholder() {
        code_.u8(0xE9);
        std::size_t pos = code_.size();
        code_.u32(0);
        return pos;
    }

    std::size_t emit_jcc_placeholder(std::uint8_t cc) {
        code_.u8(0x0F);
        code_.u8(cc);
        std::size_t pos = code_.size();
        code_.u32(0);
        return pos;
    }

    void patch_rel32(std::size_t imm_pos, std::size_t target) {
        std::int64_t rel = static_cast<std::int64_t>(target) - static_cast<std::int64_t>(imm_pos + 4);
        code_.patch32(imm_pos, static_cast<std::int32_t>(rel));
    }

    void emit_entry() {
        code_.u8(0xE8);
        std::size_t main_call = code_.size();
        code_.u32(0);
        global_calls_.push_back(CallPatch{main_call, "main"});
        code_.u8(0x89);
        code_.u8(0xC7);
        code_.u8(0xB8);
        code_.u32(60);
        code_.u8(0x0F);
        code_.u8(0x05);
    }

    void emit_builtin_prologue(int stack_bytes) {
        code_.u8(0x55);
        code_.u8(0x48);
        code_.u8(0x89);
        code_.u8(0xE5);
        code_.u8(0x48);
        code_.u8(0x83);
        code_.u8(0xEC);
        code_.u8(static_cast<std::uint8_t>(stack_bytes));
    }

    void emit_builtin_epilogue() {
        code_.u8(0xC9);
        code_.u8(0xC3);
    }

    void emit_mov_rax_imm64(std::uint64_t value) {
        code_.u8(0x48);
        code_.u8(0xB8);
        code_.u64(value);
    }

    void emit_mov_rdi_imm64(std::uint64_t value) {
        code_.u8(0x48);
        code_.u8(0xBF);
        code_.u64(value);
    }

    void emit_mov_rdx_imm64(std::uint64_t value) {
        code_.u8(0x48);
        code_.u8(0xBA);
        code_.u64(value);
    }

    void emit_mov_r9_imm64(std::uint64_t value) {
        code_.u8(0x49);
        code_.u8(0xB9);
        code_.u64(value);
    }

    void emit_sys_write_from_local(int offset, std::uint64_t length) {
        emit_mov_rax_imm64(1);
        emit_mov_rdi_imm64(1);
        code_.u8(0x48);
        code_.u8(0x8D);
        code_.u8(0xB5);
        code_.u32(static_cast<std::uint32_t>(-offset));
        emit_mov_rdx_imm64(length);
        code_.u8(0x0F);
        code_.u8(0x05);
    }

    void emit_sys_exit_1() {
        emit_mov_rax_imm64(60);
        emit_mov_rdi_imm64(1);
        code_.u8(0x0F);
        code_.u8(0x05);
    }

    void emit_builtin_write_byte() {
        emit_builtin_prologue(16);
        code_.u8(0x48);
        code_.u8(0x89);
        code_.u8(0xBD);
        code_.u32(0xfffffff8U);
        emit_sys_write_from_local(8, 1);
        emit_builtin_epilogue();
    }

    void emit_builtin_read_byte() {
        emit_builtin_prologue(16);
        emit_mov_rax_imm64(0);
        emit_mov_rdi_imm64(0);
        code_.u8(0x48);
        code_.u8(0x8D);
        code_.u8(0xB5);
        code_.u32(0xfffffff8U);
        emit_mov_rdx_imm64(1);
        code_.u8(0x0F);
        code_.u8(0x05);
        code_.u8(0x48);
        code_.u8(0x83);
        code_.u8(0xF8);
        code_.u8(0x01);
        std::size_t eof = emit_jcc_placeholder(0x85);
        code_.u8(0x48);
        code_.u8(0x0F);
        code_.u8(0xB6);
        code_.u8(0x85);
        code_.u32(0xfffffff8U);
        std::size_t done = emit_jmp_placeholder();
        patch_rel32(eof, code_.size());
        emit_mov_rax_imm64(static_cast<std::uint64_t>(-1));
        patch_rel32(done, code_.size());
        emit_builtin_epilogue();
    }

    void emit_builtin_assert() {
        emit_builtin_prologue(0);
        code_.u8(0x48);
        code_.u8(0x83);
        code_.u8(0xFF);
        code_.u8(0x00);
        std::size_t ok = emit_jcc_placeholder(0x85);
        emit_sys_exit_1();
        patch_rel32(ok, code_.size());
        emit_mov_rax_imm64(0);
        emit_builtin_epilogue();
    }

    void emit_builtin_assert_compare(std::uint8_t pass_cc) {
        emit_builtin_prologue(0);
        code_.u8(0x48);
        code_.u8(0x39);
        code_.u8(0xF7);
        std::size_t ok = emit_jcc_placeholder(pass_cc);
        emit_sys_exit_1();
        patch_rel32(ok, code_.size());
        emit_mov_rax_imm64(0);
        emit_builtin_epilogue();
    }

    void emit_builtin_panic() {
        emit_builtin_prologue(0);
        emit_sys_exit_1();
    }

    void emit_builtin_newline() {
        emit_builtin_prologue(16);
        emit_mov_rax_imm64(10);
        code_.u8(0x48);
        code_.u8(0x89);
        code_.u8(0x85);
        code_.u32(0xfffffff8U);
        emit_sys_write_from_local(8, 1);
        emit_builtin_epilogue();
    }

    void emit_builtin_write_bool() {
        emit_builtin_prologue(16);
        emit_mov_rax_imm64(static_cast<std::uint64_t>('0'));
        code_.u8(0x48);
        code_.u8(0x83);
        code_.u8(0xFF);
        code_.u8(0x00);
        std::size_t is_false = emit_jcc_placeholder(0x84);
        emit_mov_rax_imm64(static_cast<std::uint64_t>('1'));
        patch_rel32(is_false, code_.size());
        code_.u8(0x48);
        code_.u8(0x89);
        code_.u8(0x85);
        code_.u32(0xfffffff8U);
        emit_sys_write_from_local(8, 1);
        emit_builtin_epilogue();
    }

    void emit_builtin_write_i64() {
        emit_builtin_prologue(64);
        code_.u8(0x48);
        code_.u8(0x89);
        code_.u8(0xF8);
        code_.u8(0x48);
        code_.u8(0x8D);
        code_.u8(0x75);
        code_.u8(0xFF);
        emit_mov_r9_imm64(0);
        code_.u8(0x48);
        code_.u8(0x83);
        code_.u8(0xF8);
        code_.u8(0x00);
        std::size_t positive = emit_jcc_placeholder(0x8D);
        emit_mov_r9_imm64(1);
        code_.u8(0x48);
        code_.u8(0xF7);
        code_.u8(0xD8);
        patch_rel32(positive, code_.size());

        code_.u8(0x48);
        code_.u8(0xBB);
        code_.u64(10);
        code_.u8(0x48);
        code_.u8(0x83);
        code_.u8(0xF8);
        code_.u8(0x00);
        std::size_t has_digits = emit_jcc_placeholder(0x85);
        code_.u8(0xC6);
        code_.u8(0x06);
        code_.u8(static_cast<std::uint8_t>('0'));
        code_.u8(0x48);
        code_.u8(0xFF);
        code_.u8(0xCE);
        std::size_t after_digits_zero = emit_jmp_placeholder();

        std::size_t digit_loop = code_.size();
        patch_rel32(has_digits, digit_loop);
        code_.u8(0x48);
        code_.u8(0x31);
        code_.u8(0xD2);
        code_.u8(0x48);
        code_.u8(0xF7);
        code_.u8(0xF3);
        code_.u8(0x80);
        code_.u8(0xC2);
        code_.u8(static_cast<std::uint8_t>('0'));
        code_.u8(0x88);
        code_.u8(0x16);
        code_.u8(0x48);
        code_.u8(0xFF);
        code_.u8(0xCE);
        code_.u8(0x48);
        code_.u8(0x83);
        code_.u8(0xF8);
        code_.u8(0x00);
        std::size_t more_digits = emit_jcc_placeholder(0x85);
        patch_rel32(more_digits, digit_loop);

        std::size_t after_digits = code_.size();
        patch_rel32(after_digits_zero, after_digits);
        code_.u8(0x49);
        code_.u8(0x83);
        code_.u8(0xF9);
        code_.u8(0x00);
        std::size_t emit_number = emit_jcc_placeholder(0x84);
        code_.u8(0xC6);
        code_.u8(0x06);
        code_.u8(static_cast<std::uint8_t>('-'));
        code_.u8(0x48);
        code_.u8(0xFF);
        code_.u8(0xCE);

        patch_rel32(emit_number, code_.size());
        code_.u8(0x48);
        code_.u8(0xFF);
        code_.u8(0xC6);
        code_.u8(0x48);
        code_.u8(0x89);
        code_.u8(0xEA);
        code_.u8(0x48);
        code_.u8(0x29);
        code_.u8(0xF2);
        emit_mov_rax_imm64(1);
        emit_mov_rdi_imm64(1);
        code_.u8(0x0F);
        code_.u8(0x05);
        emit_builtin_epilogue();
    }

    void register_builtin_aliases(const std::string& symbol, std::size_t offset) {
        for (const auto& alias : ari_builtin_source_aliases()) {
            if (alias.symbol == symbol) function_offsets_[alias.source_name] = offset;
        }
    }

    void emit_builtin_functions() {
        std::size_t write_byte = code_.size();
        register_builtin_aliases("ari_builtin_write_byte", write_byte);
        emit_builtin_write_byte();

        std::size_t newline = code_.size();
        register_builtin_aliases("ari_builtin_newline", newline);
        emit_builtin_newline();

        std::size_t read_byte = code_.size();
        register_builtin_aliases("ari_builtin_read_byte", read_byte);
        emit_builtin_read_byte();

        std::size_t write_bool = code_.size();
        register_builtin_aliases("ari_builtin_write_bool", write_bool);
        emit_builtin_write_bool();

        std::size_t write_i64 = code_.size();
        register_builtin_aliases("ari_builtin_write_i64", write_i64);
        emit_builtin_write_i64();

        std::size_t assert_fn = code_.size();
        register_builtin_aliases("ari_builtin_assert", assert_fn);
        emit_builtin_assert();

        std::size_t assert_eq = code_.size();
        register_builtin_aliases("ari_builtin_assert_eq_i64", assert_eq);
        register_builtin_aliases("ari_builtin_assert_eq_bool", assert_eq);
        emit_builtin_assert_compare(0x84);

        std::size_t assert_ne = code_.size();
        register_builtin_aliases("ari_builtin_assert_ne_i64", assert_ne);
        register_builtin_aliases("ari_builtin_assert_ne_bool", assert_ne);
        emit_builtin_assert_compare(0x85);

        std::size_t panic = code_.size();
        register_builtin_aliases("ari_builtin_panic", panic);
        emit_builtin_panic();
    }

    void patch_calls() {
        for (const auto& call : global_calls_) {
            auto found = function_offsets_.find(call.name);
            if (found == function_offsets_.end()) throw CompileError("backend cannot find function '" + call.name + "'");
            std::int64_t rel = static_cast<std::int64_t>(found->second) - static_cast<std::int64_t>(call.imm_offset + 4);
            code_.patch32(call.imm_offset, static_cast<std::int32_t>(rel));
        }
        for (const auto& address : global_addresses_) {
            auto found = function_offsets_.find(address.name);
            if (found == function_offsets_.end()) throw CompileError("backend cannot find function '" + address.name + "'");
            std::int64_t rel = static_cast<std::int64_t>(found->second) - static_cast<std::int64_t>(address.imm_offset + 4);
            code_.patch32(address.imm_offset, static_cast<std::int32_t>(rel));
        }
    }
};

EmittedProgram emit_program_with_symbols(const IrProgram& program) {
    ProgramEmitter emitter(program);
    return emitter.emit();
}

std::vector<std::uint8_t> emit_program(const IrProgram& program) {
    EmittedProgram emitted = emit_program_with_symbols(program);
    return std::move(emitted.code);
}

} // namespace ari
