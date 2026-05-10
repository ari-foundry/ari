#pragma once

#include "ir.hpp"

#include <string>
#include <vector>

namespace ari {

enum class ForIteratorTraitKind {
    Iterator,
    IntoIterator
};

struct ForIteratorTraitMatch {
    ForIteratorTraitKind kind = ForIteratorTraitKind::Iterator;
    std::string trait_name;
    IrType item_type;
};

bool is_std_iterator_trait_name(const std::string& name);
bool is_std_into_iterator_trait_name(const std::string& name);
bool is_into_iterator_result_contract(const std::string& trait_name, const std::string& method_name);
bool is_iterator_next_receiver_contract(const std::string& trait_name, const std::string& method_name);
bool is_into_iterator_receiver_contract(const std::string& trait_name, const std::string& method_name);
bool iterator_receiver_compatible(const IrType& expected, const IrType& actual);

std::vector<std::string> for_iterator_trait_candidates(ForIteratorTraitKind kind);
std::string for_iterator_trait_display(const std::string& trait_name, const IrType& item_type);

} // namespace ari
