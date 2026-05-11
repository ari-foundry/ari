#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace ari {

struct BorrowReturnPathComponent {
    enum class Kind {
        Field,
        Index
    };

    Kind kind = Kind::Field;
    std::string text;
    std::uint64_t index = 0;
    SourceLocation loc;
};

struct ExplicitBorrowReturnContract {
    std::string param_name;
    std::vector<BorrowReturnPathComponent> path;
    SourceLocation loc;
};

std::optional<std::size_t> borrow_return_param_index(const std::vector<IrType>& params,
                                                     const IrType& result);
std::optional<ExplicitBorrowReturnContract> explicit_borrow_return_contract(
    const std::vector<Attribute>& attributes);

} // namespace ari
