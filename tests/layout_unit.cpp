#include "aggregate_abi.hpp"
#include "layout.hpp"
#include "target.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void fail(const std::string& message) {
    std::cerr << "layout unit failure: " << message << "\n";
    ++failures;
}

template <typename T, typename U>
void expect_eq(const T& actual, const U& expected, const std::string& label) {
    if (!(actual == expected)) {
        std::cerr << "layout unit failure: " << label
                  << " expected=" << expected
                  << " actual=" << actual << "\n";
        ++failures;
    }
}

void expect_true(bool value, const std::string& label) {
    if (!value) fail(label);
}

ari::IrType ty(ari::IrPrimitiveKind primitive, const std::string& name) {
    ari::IrType type;
    type.primitive = primitive;
    type.name = name;
    return type;
}

ari::IrType qualified(ari::TypeQualifier qualifier,
                      ari::IrPrimitiveKind primitive,
                      const std::string& name) {
    ari::IrType type = ty(primitive, name);
    type.qualifier = qualifier;
    return type;
}

ari::IrType tuple(std::vector<ari::IrType> fields) {
    ari::IrType type = ty(ari::IrPrimitiveKind::Tuple, "");
    type.args = std::move(fields);
    return type;
}

ari::IrType array_of(ari::IrType element, std::uint64_t count) {
    ari::IrType type = ty(ari::IrPrimitiveKind::Array, "");
    type.args.push_back(std::move(element));
    type.array_size = count;
    return type;
}

ari::IrType record(const std::string& name, std::vector<ari::IrType> fields) {
    ari::IrType type = ty(ari::IrPrimitiveKind::Struct, name);
    type.field_types = std::move(fields);
    return type;
}

ari::IrType aggregate_enum(const std::string& name, std::vector<ari::IrType> storage) {
    ari::IrType type = ty(ari::IrPrimitiveKind::Enum, name);
    type.field_types = std::move(storage);
    return type;
}

ari::IrType vector_storage(std::vector<ari::IrType> fields) {
    ari::IrType type = ty(ari::IrPrimitiveKind::Vector, "Vec");
    type.field_types = std::move(fields);
    return type;
}

void expect_size_align(const ari::IrType& type,
                       std::uint64_t size,
                       std::uint64_t align,
                       const std::string& label) {
    std::uint64_t actual_size = 0;
    std::uint64_t actual_align = 0;
    expect_true(ari::ari_layout_size_bytes(type, actual_size), label + " size available");
    expect_true(ari::ari_layout_align_bytes(type, actual_align), label + " align available");
    expect_eq(actual_size, size, label + " size");
    expect_eq(actual_align, align, label + " align");
}

void expect_offset(const ari::IrType& type,
                   std::uint64_t index,
                   std::uint64_t offset,
                   const std::string& label) {
    std::uint64_t actual = 0;
    expect_true(ari::ari_layout_field_offset_bytes(type, index, actual),
                label + " offset available");
    expect_eq(actual, offset, label + " offset");
}

void expect_abi_kind(ari::NonLocalAggregateAbiKind actual,
                     ari::NonLocalAggregateAbiKind expected,
                     const std::string& label) {
    expect_eq(static_cast<int>(actual), static_cast<int>(expected), label);
}

void expect_abi_reason(ari::NonLocalAggregateAbiReason actual,
                       ari::NonLocalAggregateAbiReason expected,
                       const std::string& label) {
    expect_eq(static_cast<int>(actual), static_cast<int>(expected), label);
}

void test_primitive_layout() {
    expect_size_align(ty(ari::IrPrimitiveKind::Void, "void"), 0, 1, "void");
    expect_size_align(ty(ari::IrPrimitiveKind::Bool, "bool"), 1, 1, "bool");
    expect_size_align(ty(ari::IrPrimitiveKind::I8, "i8"), 1, 1, "i8");
    expect_size_align(ty(ari::IrPrimitiveKind::U8, "u8"), 1, 1, "u8");
    expect_size_align(ty(ari::IrPrimitiveKind::I16, "i16"), 2, 2, "i16");
    expect_size_align(ty(ari::IrPrimitiveKind::U16, "u16"), 2, 2, "u16");
    expect_size_align(ty(ari::IrPrimitiveKind::I32, "i32"), 4, 4, "i32");
    expect_size_align(ty(ari::IrPrimitiveKind::U32, "u32"), 4, 4, "u32");
    expect_size_align(ty(ari::IrPrimitiveKind::F32, "f32"), 4, 4, "f32");
    expect_size_align(ty(ari::IrPrimitiveKind::I64, "i64"), 8, 8, "i64");
    expect_size_align(ty(ari::IrPrimitiveKind::U64, "u64"), 8, 8, "u64");
    expect_size_align(ty(ari::IrPrimitiveKind::F64, "f64"), 8, 8, "f64");
    expect_size_align(ty(ari::IrPrimitiveKind::F128, "f128"), 16, 16, "f128");
    expect_size_align(ty(ari::IrPrimitiveKind::StaticStr, "static string literal"),
                      8, 8, "static string literal handle");
    expect_size_align(ty(ari::IrPrimitiveKind::Function, "fn"), 8, 8, "function pointer");
    expect_size_align(ty(ari::IrPrimitiveKind::Enum, "Kind"), 8, 8, "fieldless enum tag");
    expect_size_align(qualified(ari::TypeQualifier::Ptr, ari::IrPrimitiveKind::I64, "i64"),
                      8, 8, "raw pointer");
    expect_size_align(qualified(ari::TypeQualifier::Ref, ari::IrPrimitiveKind::I64, "i64"),
                      8, 8, "shared reference");
    expect_size_align(qualified(ari::TypeQualifier::MutRef, ari::IrPrimitiveKind::I64, "i64"),
                      8, 8, "mutable reference");
    expect_size_align(qualified(ari::TypeQualifier::Own, ari::IrPrimitiveKind::I64, "i64"),
                      8, 8, "owning handle");
}

void test_aggregate_layout() {
    ari::IrType i8 = ty(ari::IrPrimitiveKind::I8, "i8");
    ari::IrType u16 = ty(ari::IrPrimitiveKind::U16, "u16");
    ari::IrType i64 = ty(ari::IrPrimitiveKind::I64, "i64");

    ari::IrType mixed = record("Mixed", {i8, i64, u16});
    expect_size_align(mixed, 24, 8, "mixed struct");
    expect_offset(mixed, 0, 0, "mixed struct field 0");
    expect_offset(mixed, 1, 8, "mixed struct field 1");
    expect_offset(mixed, 2, 16, "mixed struct field 2");

    ari::IrType same_tuple = tuple({i8, i64, u16});
    expect_size_align(same_tuple, 24, 8, "mixed tuple");
    expect_offset(same_tuple, 0, 0, "mixed tuple field 0");
    expect_offset(same_tuple, 1, 8, "mixed tuple field 1");
    expect_offset(same_tuple, 2, 16, "mixed tuple field 2");

    ari::IrType u16x3 = array_of(u16, 3);
    expect_size_align(u16x3, 6, 2, "u16 array");
    expect_offset(u16x3, 0, 0, "u16 array field 0");
    expect_offset(u16x3, 1, 2, "u16 array field 1");
    expect_offset(u16x3, 2, 4, "u16 array field 2");

    ari::IrType packet = aggregate_enum("Packet", {
        ty(ari::IrPrimitiveKind::I32, "i32"),
        i64,
        i64,
    });
    expect_size_align(packet, 24, 8, "aggregate enum storage");
    expect_offset(packet, 0, 0, "aggregate enum tag");
    expect_offset(packet, 1, 8, "aggregate enum payload 0");
    expect_offset(packet, 2, 16, "aggregate enum payload 1");

    ari::IrType vector = vector_storage({
        qualified(ari::TypeQualifier::Ptr, ari::IrPrimitiveKind::I64, "i64"),
        i64,
    });
    expect_size_align(vector, 16, 8, "vector storage");
}

void test_aggregate_abi_classifier() {
    ari::TargetInfo linux64 = ari::resolve_target_info("x86_64-pc-linux-gnu");
    ari::TargetInfo windows64 = ari::resolve_target_info("x86_64-pc-windows-msvc");

    ari::IrType i64 = ty(ari::IrPrimitiveKind::I64, "i64");
    ari::IrType pair = record("Pair", {i64, i64});
    ari::NonLocalAggregateAbi pair_abi =
        ari::classify_nonlocal_aggregate_abi(pair, linux64);
    expect_abi_kind(pair_abi.kind, ari::NonLocalAggregateAbiKind::Direct,
                    "Pair direct aggregate ABI");
    expect_eq(pair_abi.size_bytes, static_cast<std::uint64_t>(16), "Pair ABI size");
    expect_eq(pair_abi.align_bytes, static_cast<std::uint64_t>(8), "Pair ABI align");

    ari::IrType wide = record("Wide", {i64, i64, i64});
    ari::NonLocalAggregateAbi wide_abi =
        ari::classify_nonlocal_aggregate_abi(wide, linux64);
    expect_abi_kind(wide_abi.kind, ari::NonLocalAggregateAbiKind::Indirect,
                    "Wide indirect aggregate ABI");
    expect_abi_reason(wide_abi.reason, ari::NonLocalAggregateAbiReason::IndirectByLayout,
                      "Wide indirect reason");

    ari::IrType pair_array = array_of(i64, 2);
    ari::NonLocalAggregateAbi pair_array_abi =
        ari::classify_nonlocal_aggregate_abi(pair_array, linux64);
    expect_abi_kind(pair_array_abi.kind, ari::NonLocalAggregateAbiKind::Direct,
                    "array pair direct aggregate ABI");
    expect_eq(pair_array_abi.size_bytes, static_cast<std::uint64_t>(16), "array pair ABI size");
    expect_eq(pair_array_abi.align_bytes, static_cast<std::uint64_t>(8), "array pair ABI align");

    ari::IrType wide_array = array_of(i64, 3);
    ari::NonLocalAggregateAbi wide_array_abi =
        ari::classify_nonlocal_aggregate_abi(wide_array, linux64);
    expect_abi_kind(wide_array_abi.kind, ari::NonLocalAggregateAbiKind::Indirect,
                    "wide array indirect aggregate ABI");
    expect_abi_reason(wide_array_abi.reason, ari::NonLocalAggregateAbiReason::IndirectByLayout,
                      "wide array indirect reason");

    ari::IrType small_enum = aggregate_enum("SmallEnum", {
        ty(ari::IrPrimitiveKind::I32, "i32"),
        i64,
    });
    ari::NonLocalAggregateAbi small_enum_abi =
        ari::classify_nonlocal_aggregate_abi(small_enum, linux64);
    expect_abi_kind(small_enum_abi.kind, ari::NonLocalAggregateAbiKind::Direct,
                    "small aggregate enum direct ABI");
    expect_eq(small_enum_abi.size_bytes, static_cast<std::uint64_t>(16),
              "small aggregate enum ABI size");
    expect_eq(small_enum_abi.align_bytes, static_cast<std::uint64_t>(8),
              "small aggregate enum ABI align");

    ari::IrType wide_enum = aggregate_enum("WideEnum", {
        ty(ari::IrPrimitiveKind::I32, "i32"),
        i64,
        i64,
    });
    ari::NonLocalAggregateAbi wide_enum_abi =
        ari::classify_nonlocal_aggregate_abi(wide_enum, linux64);
    expect_abi_kind(wide_enum_abi.kind, ari::NonLocalAggregateAbiKind::Indirect,
                    "wide aggregate enum indirect ABI");
    expect_abi_reason(wide_enum_abi.reason, ari::NonLocalAggregateAbiReason::IndirectByLayout,
                      "wide aggregate enum indirect reason");

    ari::IrType empty = record("Empty", {});
    ari::NonLocalAggregateAbi empty_abi =
        ari::classify_nonlocal_aggregate_abi(empty, linux64);
    expect_abi_kind(empty_abi.kind, ari::NonLocalAggregateAbiKind::Unsupported,
                    "Empty aggregate unsupported ABI");
    expect_abi_reason(empty_abi.reason, ari::NonLocalAggregateAbiReason::ZeroSized,
                      "Empty aggregate zero-sized reason");

    ari::IrType root_vec = ty(ari::IrPrimitiveKind::Vector, "Vec");
    ari::NonLocalAggregateAbi vec_abi =
        ari::classify_nonlocal_aggregate_abi(root_vec, linux64);
    expect_abi_kind(vec_abi.kind, ari::NonLocalAggregateAbiKind::Unsupported,
                    "root Vec unavailable ABI");
    expect_abi_reason(vec_abi.reason, ari::NonLocalAggregateAbiReason::LayoutUnavailable,
                      "root Vec layout unavailable reason");

    ari::NonLocalAggregateAbi windows_abi =
        ari::classify_nonlocal_aggregate_abi(pair, windows64);
    expect_abi_kind(windows_abi.kind, ari::NonLocalAggregateAbiKind::Unsupported,
                    "windows aggregate ABI unsupported");
    expect_abi_reason(windows_abi.reason, ari::NonLocalAggregateAbiReason::TargetUnsupported,
                      "windows aggregate ABI reason");

    ari::NonLocalAggregateAbi scalar_abi =
        ari::classify_nonlocal_aggregate_abi(i64, linux64);
    expect_abi_kind(scalar_abi.kind, ari::NonLocalAggregateAbiKind::NotAggregate,
                    "scalar ABI is not aggregate");
}

} // namespace

int main() {
    test_primitive_layout();
    test_aggregate_layout();
    test_aggregate_abi_classifier();
    if (failures != 0) return 1;
    std::cout << "layout unit ok\n";
    return 0;
}
