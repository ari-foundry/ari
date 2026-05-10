#include "pattern_coverage.hpp"

#include "common.hpp"
#include "constant_semantics.hpp"
#include "pattern_semantics.hpp"
#include "type_semantics.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

void add_integer_coverage_interval(ScalarMatchCoverage& coverage,
                                   std::uint64_t start,
                                   std::uint64_t end) {
    if (start > end) return;
    coverage.integer_intervals.push_back({start, end});
    std::sort(coverage.integer_intervals.begin(), coverage.integer_intervals.end());

    std::vector<std::pair<std::uint64_t, std::uint64_t>> merged;
    for (const auto& interval : coverage.integer_intervals) {
        if (merged.empty()) {
            merged.push_back(interval);
            continue;
        }
        auto& last = merged.back();
        bool adjacent = last.second != std::numeric_limits<std::uint64_t>::max() &&
                        interval.first == last.second + 1;
        if (interval.first <= last.second || adjacent) {
            if (interval.second > last.second) last.second = interval.second;
        } else {
            merged.push_back(interval);
        }
    }
    coverage.integer_intervals = std::move(merged);
}

std::string join_product_pattern_parts(const std::vector<std::string>& parts,
                                       const std::string& separator) {
    std::string out;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) out += separator;
        out += parts[i];
    }
    return out;
}

std::string ordered_integer_product_literal(std::uint64_t ordered_value,
                                            const IrType& type) {
    std::string suffix = type_name(type);
    if (is_unsigned_integer_primitive(type.primitive)) {
        return std::to_string(ordered_value) + suffix;
    }

    std::uint64_t bias = signed_negative_limit(type.primitive);
    if (ordered_value < bias) {
        return "-" + std::to_string(bias - ordered_value) + suffix;
    }
    return std::to_string(ordered_value - bias) + suffix;
}

bool product_interval_covers_domain(const ProductInterval& interval,
                                    const IrType& type) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        return interval.start == 0 && interval.end == 1;
    }
    if (is_value_integer_type(type)) {
        return interval.start == 0 && interval.end == integer_pattern_max_order_value(type);
    }
    return false;
}

const std::vector<IrType>& product_coverage_field_types(const IrType& type) {
    if (type.primitive == IrPrimitiveKind::Struct ||
        type.primitive == IrPrimitiveKind::Array) {
        return type.field_types;
    }
    return type.args;
}

IrExpr make_pattern_integer_literal(SourceLocation loc,
                                    std::uint64_t value,
                                    bool negative,
                                    const std::string& suffix,
                                    const IrType& default_type) {
    IrExpr literal;
    literal.kind = IrExprKind::Integer;
    literal.loc = loc;
    literal.int_value = value;
    literal.int_negative = negative;
    literal.type = suffix.empty()
        ? default_type
        : integer_literal_suffix_type(suffix, loc);
    return literal;
}

std::string format_product_missing_case(const IrType& type,
                                        const ProductRect& rect,
                                        std::size_t& dimension,
                                        const std::set<std::string>& tuple_struct_names) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        if (dimension >= rect.size()) return "_";
        const ProductInterval interval = rect[dimension++];
        if (product_interval_covers_domain(interval, type)) return "_";
        if (interval.start == interval.end) return interval.start == 0 ? "false" : "true";
        return "_";
    }
    if (is_value_integer_type(type)) {
        if (dimension >= rect.size()) return "_";
        const ProductInterval interval = rect[dimension++];
        if (product_interval_covers_domain(interval, type)) return "_";
        if (interval.start == interval.end) {
            return ordered_integer_product_literal(interval.start, type);
        }
        return ordered_integer_product_literal(interval.start, type) + "..=" +
               ordered_integer_product_literal(interval.end, type);
    }
    if (type.primitive == IrPrimitiveKind::Tuple ||
        type.primitive == IrPrimitiveKind::Array ||
        type.primitive == IrPrimitiveKind::Struct) {
        std::vector<std::string> parts;
        const std::vector<IrType>& fields = product_coverage_field_types(type);
        parts.reserve(fields.size());
        for (const auto& field_type : fields) {
            parts.push_back(format_product_missing_case(field_type, rect, dimension, tuple_struct_names));
        }

        if (type.primitive == IrPrimitiveKind::Tuple) {
            return "(" + join_product_pattern_parts(parts, ", ") + ")";
        }
        if (type.primitive == IrPrimitiveKind::Array) {
            return "[" + join_product_pattern_parts(parts, ", ") + "]";
        }

        if (tuple_struct_names.count(type.name)) {
            return type.name + "(" + join_product_pattern_parts(parts, ", ") + ")";
        }

        std::vector<std::string> named_parts;
        named_parts.reserve(parts.size());
        for (std::size_t i = 0; i < parts.size(); ++i) {
            std::string field_name = i < type.field_names.size()
                ? type.field_names[i]
                : ("_" + std::to_string(i));
            named_parts.push_back(field_name + ": " + parts[i]);
        }
        return type.name + " { " + join_product_pattern_parts(named_parts, ", ") + " }";
    }
    return "_";
}

std::string enum_payload_pattern_coverage_key(const IrMatchArm& arm) {
    if (!arm.payload_literal_conditions.empty() ||
        !arm.payload_range_conditions.empty() ||
        !arm.payload_enum_conditions.empty()) {
        std::string key = std::to_string(arm.enum_tag);
        for (const auto& condition : arm.payload_literal_conditions) {
            key += ":L" + std::to_string(condition.index) + ":" + std::to_string(condition.bits());
        }
        for (const auto& condition : arm.payload_range_conditions) {
            key += ":R" + std::to_string(condition.index) + ":" +
                   (condition.start_negative ? "-" : "") + std::to_string(condition.start_int) + ":" +
                   (condition.end_negative ? "-" : "") + std::to_string(condition.end_int) + ":" +
                   (condition.inclusive ? "1" : "0");
        }
        for (const auto& condition : arm.payload_enum_conditions) {
            key += ":E" + std::to_string(condition.index) + ":" + condition.enum_type.name + ":" +
                   std::to_string(condition.tag);
            if (condition.has_payload_literal) {
                key += std::string(":L") + std::to_string(condition.nested_payload_index) + ":" +
                       (condition.payload_literal_negative ? "-" : "") +
                       std::to_string(condition.payload_literal_is_bool
                                          ? (condition.payload_literal.boolean ? 1ULL : 0ULL)
                                          : condition.payload_literal.integer) + ":" +
                       (condition.payload_literal_is_bool ? "B" : "I") + ":" +
                       (condition.payload_literal_is_bool && condition.payload_literal.boolean ? "1" : "0");
            }
            if (condition.has_payload_range) {
                key += std::string(":R") + std::to_string(condition.nested_payload_index) + ":" +
                       (condition.range_start_negative ? "-" : "") +
                       std::to_string(condition.range_start_int) + ":" +
                       (condition.range_end_negative ? "-" : "") +
                       std::to_string(condition.range_end_int) + ":" +
                       (condition.range_inclusive ? "1" : "0");
            }
        }
        return key;
    }
    if (arm.literal_is_bool) return arm.literal_bool ? "1" : "0";
    return std::to_string(arm.literal_int);
}

} // namespace

std::string bool_product_value(bool value) {
    return value ? "b1" : "b0";
}

std::string integer_product_value(std::uint64_t ordered_value) {
    return "i" + std::to_string(ordered_value);
}

bool finite_scalar_product_domain(const IrType& type, std::vector<std::string>& out) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        out = {bool_product_value(false), bool_product_value(true)};
        return true;
    }
    if (!is_value_integer_type(type)) return false;

    std::uint64_t max = integer_pattern_max_order_value(type);
    if (max >= kMaxFiniteProductCoverageValues) return false;
    out.clear();
    out.reserve(static_cast<std::size_t>(max + 1));
    for (std::uint64_t value = 0; value <= max; ++value) {
        out.push_back(integer_product_value(value));
    }
    return true;
}

bool finite_product_coverage_domain(const IrType& type, std::vector<std::string>& out) {
    if (finite_scalar_product_domain(type, out)) return true;
    if (type.primitive != IrPrimitiveKind::Tuple &&
        type.primitive != IrPrimitiveKind::Array &&
        type.primitive != IrPrimitiveKind::Struct) {
        return false;
    }

    std::vector<std::vector<std::string>> domains;
    for (const auto& field_type : product_coverage_field_types(type)) {
        std::vector<std::string> field_domain;
        if (!finite_product_coverage_domain(field_type, field_domain)) return false;
        domains.push_back(std::move(field_domain));
    }
    return combine_finite_product_domains(domains, out);
}

std::string product_coverage_join(const std::string& prefix, const std::string& suffix) {
    if (prefix.empty()) return suffix;
    return prefix + "\x1f" + suffix;
}

bool combine_finite_product_domains(const std::vector<std::vector<std::string>>& domains,
                                    std::vector<std::string>& out) {
    std::vector<std::string> result{""};
    for (const auto& domain : domains) {
        if (domain.empty()) return false;
        if (result.size() > kMaxFiniteProductCoverageValues / domain.size()) return false;
        std::vector<std::string> next;
        next.reserve(result.size() * domain.size());
        for (const auto& prefix : result) {
            for (const auto& value : domain) {
                next.push_back(product_coverage_join(prefix, value));
            }
        }
        result = std::move(next);
    }
    out = std::move(result);
    return true;
}

bool symbolic_product_coverage_domain(const IrType& type, ProductRect& out) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        out.push_back(ProductInterval{0, 1});
        return true;
    }
    if (is_value_integer_type(type)) {
        out.push_back(ProductInterval{0, integer_pattern_max_order_value(type)});
        return true;
    }
    if (type.primitive != IrPrimitiveKind::Tuple &&
        type.primitive != IrPrimitiveKind::Array &&
        type.primitive != IrPrimitiveKind::Struct) {
        return false;
    }
    for (const auto& field_type : product_coverage_field_types(type)) {
        if (!symbolic_product_coverage_domain(field_type, out)) return false;
    }
    return true;
}

bool symbolic_product_coverage_domain_rects(const IrType& type, std::vector<ProductRect>& out) {
    ProductRect rect;
    if (!symbolic_product_coverage_domain(type, rect)) return false;
    out = {std::move(rect)};
    return true;
}

void require_positional_product_pattern_arity(const Pattern& pattern,
                                              const std::vector<IrType>& fields) {
    const char* pattern_name = pattern.kind == PatternKind::Array ? "array" : "tuple";
    if (pattern.has_rest) {
        if (pattern.elements.size() > fields.size()) {
            fail(pattern.loc,
                 std::string(pattern_name) + " match pattern has " + std::to_string(pattern.elements.size()) +
                 " non-rest elements but value has " + std::to_string(fields.size()));
        }
        return;
    }
    if (pattern.elements.size() != fields.size()) {
        fail(pattern.loc,
             std::string(pattern_name) + " match pattern has " + std::to_string(pattern.elements.size()) +
             " elements but value has " + std::to_string(fields.size()));
    }
}

bool finite_integer_pattern_values(const Pattern& pattern,
                                   const IrType& type,
                                   std::vector<std::string>& out) {
    if (!is_value_integer_type(type)) return false;
    IrExpr literal = make_pattern_integer_literal(
        pattern.loc,
        pattern.int_value,
        pattern.int_negative,
        pattern.literal_suffix,
        type
    );
    if (!integer_literal_fits(literal, literal.type)) {
        fail(pattern.loc, "integer literal " + integer_literal_name(literal) +
                          " is out of range for " + type_name(literal.type));
    }
    require_assignable(pattern.loc, type, literal.type);
    out = {integer_product_value(integer_pattern_order_value(pattern.int_value, pattern.int_negative, type))};
    return true;
}

bool finite_integer_range_pattern_values(const Pattern& pattern,
                                         const IrType& type,
                                         std::vector<std::string>& out) {
    if (!is_value_integer_type(type)) return false;
    if (!range_start_le_end(pattern, type)) {
        fail(pattern.loc, "range pattern start must be <= end");
    }
    std::uint64_t start = 0;
    std::uint64_t end = 0;
    if (!integer_range_coverage_interval(pattern, type, start, end)) return true;
    if (end - start + 1 > kMaxFiniteProductCoverageValues) return false;
    out.clear();
    for (std::uint64_t value = start; value <= end; ++value) {
        out.push_back(integer_product_value(value));
    }
    return true;
}

bool finite_constant_product_values(SourceLocation loc,
                                    const ConstantValue& value,
                                    const IrType& type,
                                    std::vector<std::string>& out) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        if (!value.is_bool) fail(loc, "bool tuple constant pattern must have type bool");
        out = {bool_product_value(value.bool_value)};
        return true;
    }
    if (!is_value_integer_type(type)) return false;
    if (value.is_bool || !is_value_integer_type(value.type)) {
        fail(loc, "integer tuple constant pattern must have an integer type");
    }
    require_assignable(loc, type, value.type);
    out = {integer_product_value(integer_pattern_order_value(value.int_value, value.int_negative, type))};
    return true;
}

bool finite_positional_product_pattern_values(const Pattern& pattern,
                                              const IrType& type,
                                              const ProductPatternCoverageHooks& hooks,
                                              std::vector<std::string>& out) {
    const std::vector<IrType>& fields = product_coverage_field_types(type);
    require_positional_product_pattern_arity(pattern, fields);

    std::vector<std::vector<std::string>> domains;
    domains.reserve(fields.size());
    for (std::size_t field_index = 0; field_index < fields.size(); ++field_index) {
        const Pattern* item = positional_product_field_pattern(pattern, fields.size(), field_index);

        std::vector<std::string> field_values;
        bool ok = item
            ? finite_product_pattern_values(*item, fields[field_index], hooks, field_values)
            : finite_product_coverage_domain(fields[field_index], field_values);
        if (!ok) return false;
        domains.push_back(std::move(field_values));
    }
    return combine_finite_product_domains(domains, out);
}

bool finite_tuple_struct_product_pattern_values(const Pattern& pattern,
                                                const IrType& type,
                                                const ProductPatternCoverageHooks& hooks,
                                                std::vector<std::string>& out) {
    if (!hooks.tuple_struct_pattern_matches(pattern.loc, pattern.case_name, type)) return false;
    if (!pattern.has_payload_pattern || !pattern.payload_pattern) return false;
    const std::vector<IrType>& fields = product_coverage_field_types(type);
    const Pattern& payload = *pattern.payload_pattern;
    if (payload.kind == PatternKind::Tuple) {
        return finite_positional_product_pattern_values(payload, type, hooks, out);
    }
    if (fields.size() != 1) return false;
    return finite_product_pattern_values(payload, fields[0], hooks, out);
}

bool finite_struct_product_pattern_values(const Pattern& pattern,
                                          const IrType& type,
                                          const ProductPatternCoverageHooks& hooks,
                                          std::vector<std::string>& out) {
    hooks.require_struct_pattern_matches(pattern.loc, pattern.case_name, type);
    if (pattern.field_names.size() != pattern.elements.size()) {
        throw CompileError("internal error: struct match pattern field/value arity mismatch");
    }
    if (!pattern.has_rest && pattern.field_names.size() != type.field_names.size()) {
        fail(pattern.loc, "struct match pattern must mention all fields or use '..'");
    }

    std::vector<const Pattern*> field_patterns(type.field_names.size(), nullptr);
    std::set<std::string> seen_fields;
    for (std::size_t i = 0; i < pattern.field_names.size(); ++i) {
        const std::string& field_name = pattern.field_names[i];
        if (!seen_fields.insert(field_name).second) {
            fail(pattern.elements[i].loc, "duplicate field '" + field_name + "' in struct match pattern");
        }
        std::size_t field_index = hooks.struct_field_index(pattern.elements[i].loc, type, field_name);
        field_patterns[field_index] = &pattern.elements[i];
    }

    std::vector<std::vector<std::string>> domains;
    domains.reserve(type.field_types.size());
    for (std::size_t field_index = 0; field_index < type.field_types.size(); ++field_index) {
        std::vector<std::string> field_values;
        bool ok = field_patterns[field_index]
            ? finite_product_pattern_values(*field_patterns[field_index], type.field_types[field_index], hooks, field_values)
            : finite_product_coverage_domain(type.field_types[field_index], field_values);
        if (!ok) return false;
        domains.push_back(std::move(field_values));
    }
    return combine_finite_product_domains(domains, out);
}

bool finite_product_pattern_values(const Pattern& pattern,
                                   const IrType& type,
                                   const ProductPatternCoverageHooks& hooks,
                                   std::vector<std::string>& out) {
    switch (pattern.kind) {
        case PatternKind::Wildcard:
        case PatternKind::Binding:
            return finite_product_coverage_domain(type, out);
        case PatternKind::Alias:
            if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
            return finite_product_pattern_values(*pattern.alias_pattern, type, hooks, out);
        case PatternKind::Or: {
            if (pattern_has_binding(pattern)) {
                fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
            }
            std::set<std::string> values;
            for (const auto& alternative : pattern.alternatives) {
                std::vector<std::string> alternative_values;
                if (!finite_product_pattern_values(alternative, type, hooks, alternative_values)) return false;
                values.insert(alternative_values.begin(), alternative_values.end());
            }
            out.assign(values.begin(), values.end());
            return true;
        }
        case PatternKind::BoolLiteral:
            if (type.qualifier != TypeQualifier::Value || type.primitive != IrPrimitiveKind::Bool) return false;
            out = {bool_product_value(pattern.bool_value)};
            return true;
        case PatternKind::IntegerLiteral:
            return finite_integer_pattern_values(pattern, type, out);
        case PatternKind::Range:
            return finite_integer_range_pattern_values(pattern, type, out);
        case PatternKind::EnumCase: {
            ConstantValue constant_pattern;
            if (hooks.try_constant_pattern_value(pattern, constant_pattern)) {
                return finite_constant_product_values(pattern.loc, constant_pattern, type, out);
            }
            if (type.primitive == IrPrimitiveKind::Struct) {
                return finite_tuple_struct_product_pattern_values(pattern, type, hooks, out);
            }
            return false;
        }
        case PatternKind::Tuple:
            if (type.primitive != IrPrimitiveKind::Tuple) return false;
            return finite_positional_product_pattern_values(pattern, type, hooks, out);
        case PatternKind::Array:
            if (type.primitive != IrPrimitiveKind::Array) return false;
            return finite_positional_product_pattern_values(pattern, type, hooks, out);
        case PatternKind::Struct:
            if (type.primitive != IrPrimitiveKind::Struct) return false;
            return finite_struct_product_pattern_values(pattern, type, hooks, out);
    }
    return false;
}

bool symbolic_integer_pattern_rects(const Pattern& pattern,
                                    const IrType& type,
                                    std::vector<ProductRect>& out) {
    if (!is_value_integer_type(type)) return false;
    IrExpr literal = make_pattern_integer_literal(
        pattern.loc,
        pattern.int_value,
        pattern.int_negative,
        pattern.literal_suffix,
        type
    );
    if (!integer_literal_fits(literal, literal.type)) {
        fail(pattern.loc, "integer literal " + integer_literal_name(literal) +
                          " is out of range for " + type_name(literal.type));
    }
    require_assignable(pattern.loc, type, literal.type);
    std::uint64_t point = integer_pattern_order_value(pattern.int_value, pattern.int_negative, type);
    out = {ProductRect{ProductInterval{point, point}}};
    return true;
}

bool symbolic_integer_range_pattern_rects(const Pattern& pattern,
                                          const IrType& type,
                                          std::vector<ProductRect>& out) {
    if (!is_value_integer_type(type)) return false;
    if (!range_start_le_end(pattern, type)) {
        fail(pattern.loc, "range pattern start must be <= end");
    }
    std::uint64_t start = 0;
    std::uint64_t end = 0;
    if (!integer_range_coverage_interval(pattern, type, start, end)) {
        out.clear();
        return true;
    }
    out = {ProductRect{ProductInterval{start, end}}};
    return true;
}

bool symbolic_constant_product_rects(SourceLocation loc,
                                     const ConstantValue& value,
                                     const IrType& type,
                                     std::vector<ProductRect>& out) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        if (!value.is_bool) fail(loc, "bool tuple constant pattern must have type bool");
        std::uint64_t point = value.bool_value ? 1 : 0;
        out = {ProductRect{ProductInterval{point, point}}};
        return true;
    }
    if (!is_value_integer_type(type)) return false;
    if (value.is_bool || !is_value_integer_type(value.type)) {
        fail(loc, "integer tuple constant pattern must have an integer type");
    }
    require_assignable(loc, type, value.type);
    std::uint64_t point = integer_pattern_order_value(value.int_value, value.int_negative, type);
    out = {ProductRect{ProductInterval{point, point}}};
    return true;
}

bool symbolic_positional_product_pattern_rects(const Pattern& pattern,
                                               const IrType& type,
                                               const ProductPatternCoverageHooks& hooks,
                                               std::vector<ProductRect>& out) {
    const std::vector<IrType>& fields = product_coverage_field_types(type);
    require_positional_product_pattern_arity(pattern, fields);

    std::vector<std::vector<ProductRect>> domains;
    domains.reserve(fields.size());
    for (std::size_t field_index = 0; field_index < fields.size(); ++field_index) {
        const Pattern* item = positional_product_field_pattern(pattern, fields.size(), field_index);

        std::vector<ProductRect> field_rects;
        bool ok = item
            ? symbolic_product_pattern_rects(*item, fields[field_index], hooks, field_rects)
            : symbolic_product_coverage_domain_rects(fields[field_index], field_rects);
        if (!ok) return false;
        domains.push_back(std::move(field_rects));
    }
    return combine_product_rect_domains(domains, kMaxSymbolicProductRectangles, out);
}

bool symbolic_tuple_struct_product_pattern_rects(const Pattern& pattern,
                                                 const IrType& type,
                                                 const ProductPatternCoverageHooks& hooks,
                                                 std::vector<ProductRect>& out) {
    if (!hooks.tuple_struct_pattern_matches(pattern.loc, pattern.case_name, type)) return false;
    if (!pattern.has_payload_pattern || !pattern.payload_pattern) return false;
    const std::vector<IrType>& fields = product_coverage_field_types(type);
    const Pattern& payload = *pattern.payload_pattern;
    if (payload.kind == PatternKind::Tuple) {
        return symbolic_positional_product_pattern_rects(payload, type, hooks, out);
    }
    if (fields.size() != 1) return false;
    return symbolic_product_pattern_rects(payload, fields[0], hooks, out);
}

bool symbolic_struct_product_pattern_rects(const Pattern& pattern,
                                           const IrType& type,
                                           const ProductPatternCoverageHooks& hooks,
                                           std::vector<ProductRect>& out) {
    hooks.require_struct_pattern_matches(pattern.loc, pattern.case_name, type);
    if (pattern.field_names.size() != pattern.elements.size()) {
        throw CompileError("internal error: struct match pattern field/value arity mismatch");
    }
    if (!pattern.has_rest && pattern.field_names.size() != type.field_names.size()) {
        fail(pattern.loc, "struct match pattern must mention all fields or use '..'");
    }

    std::vector<const Pattern*> field_patterns(type.field_names.size(), nullptr);
    std::set<std::string> seen_fields;
    for (std::size_t i = 0; i < pattern.field_names.size(); ++i) {
        const std::string& field_name = pattern.field_names[i];
        if (!seen_fields.insert(field_name).second) {
            fail(pattern.elements[i].loc, "duplicate field '" + field_name + "' in struct match pattern");
        }
        std::size_t field_index = hooks.struct_field_index(pattern.elements[i].loc, type, field_name);
        field_patterns[field_index] = &pattern.elements[i];
    }

    std::vector<std::vector<ProductRect>> domains;
    domains.reserve(type.field_types.size());
    for (std::size_t field_index = 0; field_index < type.field_types.size(); ++field_index) {
        std::vector<ProductRect> field_rects;
        bool ok = field_patterns[field_index]
            ? symbolic_product_pattern_rects(*field_patterns[field_index], type.field_types[field_index], hooks, field_rects)
            : symbolic_product_coverage_domain_rects(type.field_types[field_index], field_rects);
        if (!ok) return false;
        domains.push_back(std::move(field_rects));
    }
    return combine_product_rect_domains(domains, kMaxSymbolicProductRectangles, out);
}

bool symbolic_product_pattern_rects(const Pattern& pattern,
                                    const IrType& type,
                                    const ProductPatternCoverageHooks& hooks,
                                    std::vector<ProductRect>& out) {
    switch (pattern.kind) {
        case PatternKind::Wildcard:
        case PatternKind::Binding:
            return symbolic_product_coverage_domain_rects(type, out);
        case PatternKind::Alias:
            if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
            return symbolic_product_pattern_rects(*pattern.alias_pattern, type, hooks, out);
        case PatternKind::Or: {
            if (pattern_has_binding(pattern)) {
                fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
            }
            out.clear();
            for (const auto& alternative : pattern.alternatives) {
                std::vector<ProductRect> alternative_rects;
                if (!symbolic_product_pattern_rects(alternative, type, hooks, alternative_rects)) return false;
                out.insert(out.end(), alternative_rects.begin(), alternative_rects.end());
                if (out.size() > kMaxSymbolicProductRectangles) return false;
            }
            return true;
        }
        case PatternKind::BoolLiteral:
            if (type.qualifier != TypeQualifier::Value || type.primitive != IrPrimitiveKind::Bool) return false;
            {
                std::uint64_t point = pattern.bool_value ? 1 : 0;
                out = {ProductRect{ProductInterval{point, point}}};
            }
            return true;
        case PatternKind::IntegerLiteral:
            return symbolic_integer_pattern_rects(pattern, type, out);
        case PatternKind::Range:
            return symbolic_integer_range_pattern_rects(pattern, type, out);
        case PatternKind::EnumCase: {
            ConstantValue constant_pattern;
            if (hooks.try_constant_pattern_value(pattern, constant_pattern)) {
                return symbolic_constant_product_rects(pattern.loc, constant_pattern, type, out);
            }
            if (type.primitive == IrPrimitiveKind::Struct) {
                return symbolic_tuple_struct_product_pattern_rects(pattern, type, hooks, out);
            }
            return false;
        }
        case PatternKind::Tuple:
            if (type.primitive != IrPrimitiveKind::Tuple) return false;
            return symbolic_positional_product_pattern_rects(pattern, type, hooks, out);
        case PatternKind::Array:
            if (type.primitive != IrPrimitiveKind::Array) return false;
            return symbolic_positional_product_pattern_rects(pattern, type, hooks, out);
        case PatternKind::Struct:
            if (type.primitive != IrPrimitiveKind::Struct) return false;
            return symbolic_struct_product_pattern_rects(pattern, type, hooks, out);
    }
    return false;
}

bool note_finite_product_match_coverage(ProductMatchCoverage& coverage,
                                        const std::vector<std::string>& values) {
    bool added = false;
    for (const auto& value : values) {
        added = coverage.covered_products.insert(value).second || added;
    }
    return !added;
}

bool note_symbolic_product_match_coverage(ProductMatchCoverage& coverage,
                                          const std::vector<ProductRect>& rects,
                                          bool finite_handled) {
    if (!coverage.has_symbolic_universe || rects.empty()) return false;
    if (!finite_handled && product_rects_are_covered_by(rects, coverage.covered_symbolic_products)) {
        return true;
    }
    coverage.covered_symbolic_products.insert(
        coverage.covered_symbolic_products.end(),
        rects.begin(),
        rects.end()
    );
    return false;
}

bool product_match_coverage_is_exhaustive(const ProductMatchCoverage& coverage) {
    if (coverage.has_irrefutable_arm) return true;
    if (coverage.has_finite_universe &&
        coverage.universe_size > 0 &&
        coverage.covered_products.size() == coverage.universe_size) {
        return true;
    }
    return coverage.has_symbolic_universe &&
           product_rect_is_covered_by(coverage.symbolic_universe, coverage.covered_symbolic_products);
}

std::string product_missing_case_hint(const IrType& match_type,
                                      const ProductMatchCoverage& coverage,
                                      const std::set<std::string>& tuple_struct_names) {
    if (!coverage.has_symbolic_universe) return "";
    ProductRect missing;
    if (!product_rect_first_gap(coverage.symbolic_universe, coverage.covered_symbolic_products, missing)) {
        return "";
    }
    std::size_t dimension = 0;
    return format_product_missing_case(match_type, missing, dimension, tuple_struct_names);
}

void note_integer_coverage(ScalarMatchCoverage& coverage,
                           const IrType& match_type,
                           std::uint64_t value,
                           bool negative) {
    std::uint64_t point = integer_pattern_order_value(value, negative, match_type);
    add_integer_coverage_interval(coverage, point, point);
}

void note_integer_range_coverage(ScalarMatchCoverage& coverage,
                                 const IrType& match_type,
                                 const Pattern& pattern) {
    std::uint64_t start = 0;
    std::uint64_t end = 0;
    if (!integer_range_coverage_interval(pattern, match_type, start, end)) return;
    add_integer_coverage_interval(coverage, start, end);
}

bool integer_range_coverage_interval(const Pattern& pattern,
                                     const IrType& match_type,
                                     std::uint64_t& start,
                                     std::uint64_t& end) {
    start = integer_pattern_order_value(pattern.int_value, pattern.int_negative, match_type);
    end = integer_pattern_order_value(pattern.range_end_value, pattern.range_end_negative, match_type);
    if (!pattern.range_inclusive) {
        if (start >= end) return false;
        --end;
    }
    return start <= end;
}

bool integer_interval_is_fully_covered(const ScalarMatchCoverage& coverage,
                                       std::uint64_t start,
                                       std::uint64_t end) {
    if (start > end) return false;
    for (const auto& interval : coverage.integer_intervals) {
        if (interval.first <= start && end <= interval.second) return true;
        if (start < interval.first) return false;
    }
    return false;
}

bool integer_coverage_is_exhaustive(
    const IrType& match_type,
    const std::vector<std::pair<std::uint64_t, std::uint64_t>>& intervals
) {
    if (!is_value_integer_type(match_type) || intervals.empty()) return false;
    return intervals.front().first == 0 &&
           intervals.front().second == integer_pattern_max_order_value(match_type);
}

std::string scalar_match_exhaustiveness_error(const IrType& match_type,
                                              const ScalarMatchCoverage& coverage) {
    if (coverage.has_wildcard) return "";
    if (match_type.qualifier == TypeQualifier::Value && match_type.primitive == IrPrimitiveKind::Bool) {
        if (coverage.covered_patterns.count("true") && coverage.covered_patterns.count("false")) return "";
        return "bool match must cover true and false or include a wildcard arm";
    }
    if (integer_coverage_is_exhaustive(match_type, coverage.integer_intervals)) return "";
    return "integer match must include a wildcard arm";
}

EnumCoverageResult note_enum_match_coverage(EnumMatchCoverage& coverage,
                                            const IrMatchArm& arm,
                                            bool covers_case,
                                            bool bool_payload_literal,
                                            bool bool_payload_value) {
    if (!arm.payload_literal_conditions.empty() ||
        !arm.payload_range_conditions.empty() ||
        !arm.payload_enum_conditions.empty()) {
        covers_case = false;
    }
    if (coverage.covered_tags.count(arm.enum_tag)) return EnumCoverageResult::DuplicateCase;
    if (covers_case) {
        coverage.covered_tags.insert(arm.enum_tag);
        return EnumCoverageResult::Added;
    }

    if (!coverage.covered_payload_literals.insert(enum_payload_pattern_coverage_key(arm)).second) {
        return EnumCoverageResult::DuplicatePayloadPattern;
    }
    if (bool_payload_literal) {
        unsigned bit = bool_payload_value ? 0b10 : 0b01;
        unsigned& mask = coverage.covered_bool_payloads[arm.enum_tag];
        mask |= bit;
        if ((mask & 0b11) == 0b11) {
            coverage.covered_tags.insert(arm.enum_tag);
        }
    }
    return EnumCoverageResult::Added;
}

bool enum_bool_payload_literal_value(const Pattern& pattern,
                                     const std::vector<IrType>& payloads,
                                     bool& out) {
    if (!pattern.payload_pattern || pattern.payload_pattern->kind != PatternKind::BoolLiteral) return false;
    if (payloads.empty()) return false;
    const IrType& payload_type = payloads[0];
    if (payload_type.qualifier != TypeQualifier::Value ||
        payload_type.primitive != IrPrimitiveKind::Bool) {
        return false;
    }
    out = pattern.payload_pattern->bool_value;
    return true;
}

std::string enum_match_exhaustiveness_error(const std::string& enum_name,
                                            std::size_t case_count,
                                            const EnumMatchCoverage& coverage) {
    if (coverage.has_wildcard || coverage.covered_tags.size() == case_count) return "";
    return "match must cover all cases of enum '" + enum_name + "'";
}

std::uint64_t integer_pattern_order_value(std::uint64_t value,
                                          bool negative,
                                          const IrType& match_type) {
    if (is_unsigned_integer_primitive(match_type.primitive)) return value;
    std::uint64_t bias = signed_negative_limit(match_type.primitive);
    return negative ? bias - value : bias + value;
}

std::uint64_t integer_pattern_max_order_value(const IrType& match_type) {
    if (is_unsigned_integer_primitive(match_type.primitive)) {
        return unsigned_max(match_type.primitive);
    }
    return signed_negative_limit(match_type.primitive) + signed_positive_max(match_type.primitive);
}

} // namespace ari
