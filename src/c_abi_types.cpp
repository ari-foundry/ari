#include "c_abi_types.hpp"

#include <cstddef>

namespace ari {
namespace {

std::string unqualified_name(const std::string& name) {
    std::size_t split = name.rfind("::");
    if (split == std::string::npos) return name;
    return name.substr(split + 2);
}

bool alias(IrPrimitiveKind kind,
           const char* name,
           IrPrimitiveKind& primitive,
           std::string& canonical) {
    primitive = kind;
    canonical = name;
    return true;
}

} // namespace

bool c_abi_type_alias(const std::string& name, IrPrimitiveKind& primitive, std::string& canonical) {
    std::string base = unqualified_name(name);
    if (base == "c_bool") return alias(IrPrimitiveKind::Bool, "bool", primitive, canonical);
    // Current host ABI policy is x86-64 Linux/glibc: plain C char is signed.
    // Target triples should replace this fixed choice when Ari grows them.
    if (base == "c_char" || base == "c_schar") return alias(IrPrimitiveKind::I8, "i8", primitive, canonical);
    if (base == "c_uchar") return alias(IrPrimitiveKind::U8, "u8", primitive, canonical);
    if (base == "c_short") return alias(IrPrimitiveKind::I16, "i16", primitive, canonical);
    if (base == "c_ushort") return alias(IrPrimitiveKind::U16, "u16", primitive, canonical);
    if (base == "c_int") return alias(IrPrimitiveKind::I32, "i32", primitive, canonical);
    if (base == "c_uint") return alias(IrPrimitiveKind::U32, "u32", primitive, canonical);
    if (base == "c_long" || base == "c_longlong" || base == "isize" ||
        base == "ssize_t" || base == "c_ssize_t" || base == "intptr_t" ||
        base == "ptrdiff_t" || base == "intmax_t" || base == "c_intmax_t") {
        return alias(IrPrimitiveKind::I64, "i64", primitive, canonical);
    }
    if (base == "c_ulong" || base == "c_ulonglong" || base == "usize" ||
        base == "size_t" || base == "c_size_t" || base == "uintptr_t" ||
        base == "uintmax_t" || base == "c_uintmax_t") {
        return alias(IrPrimitiveKind::U64, "u64", primitive, canonical);
    }
    if (base == "c_float") return alias(IrPrimitiveKind::F32, "f32", primitive, canonical);
    if (base == "c_double") return alias(IrPrimitiveKind::F64, "f64", primitive, canonical);
    if (base == "c_void") return alias(IrPrimitiveKind::Void, "void", primitive, canonical);
    return false;
}

} // namespace ari
