#include "iterator_semantics.hpp"

#include "type_semantics.hpp"

namespace ari {
namespace {

IrType value_qualified_type(IrType type) {
    type.qualifier = TypeQualifier::Value;
    return type;
}

bool is_receiver_borrow_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Ref || type.qualifier == TypeQualifier::MutRef;
}

bool same_receiver_base_type(const IrType& left, const IrType& right) {
    return same_type(value_qualified_type(left), value_qualified_type(right));
}

} // namespace

bool is_std_iterator_trait_name(const std::string& name) {
    return name == "std::Iterator" || name == "std::iter::Iterator";
}

bool is_std_into_iterator_trait_name(const std::string& name) {
    return name == "std::IntoIterator" || name == "std::iter::IntoIterator";
}

bool is_into_iterator_result_contract(const std::string& trait_name, const std::string& method_name) {
    return method_name == "into_iter" && is_std_into_iterator_trait_name(trait_name);
}

bool is_iterator_next_receiver_contract(const std::string& trait_name, const std::string& method_name) {
    return method_name == "next" && is_std_iterator_trait_name(trait_name);
}

bool is_into_iterator_receiver_contract(const std::string& trait_name, const std::string& method_name) {
    return method_name == "into_iter" && is_std_into_iterator_trait_name(trait_name);
}

bool iterator_receiver_compatible(const IrType& expected, const IrType& actual) {
    if (same_type(expected, actual)) return true;
    return (expected.qualifier == TypeQualifier::Value || is_receiver_borrow_type(expected)) &&
           (actual.qualifier == TypeQualifier::Value || is_receiver_borrow_type(actual)) &&
           same_receiver_base_type(expected, actual);
}

std::vector<std::string> for_iterator_trait_candidates(ForIteratorTraitKind kind) {
    if (kind == ForIteratorTraitKind::Iterator) {
        return {"std::Iterator", "std::iter::Iterator"};
    }
    return {"std::IntoIterator", "std::iter::IntoIterator"};
}

std::string for_iterator_trait_display(const std::string& trait_name, const IrType& item_type) {
    std::string name = trait_name;
    if (name.rfind("std::iter::", 0) == 0) {
        name = "iter::" + name.substr(std::string("std::iter::").size());
    } else if (name.rfind("std::", 0) == 0) {
        name = name.substr(std::string("std::").size());
    }
    return name + "[" + type_name(item_type) + "]";
}

} // namespace ari
