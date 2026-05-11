#include "borrow_return_semantics.hpp"

#include "attribute_semantics.hpp"
#include "common.hpp"
#include "type_semantics.hpp"

#include <string>
#include <utility>

namespace ari {

namespace {

[[noreturn]] void fail_borrow_return_attribute(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

} // namespace

std::optional<std::size_t> borrow_return_param_index(const std::vector<IrType>& params,
                                                     const IrType& result) {
    if (!is_borrow_type(result)) return std::nullopt;

    std::optional<std::size_t> index;
    for (std::size_t i = 0; i < params.size(); ++i) {
        if (!is_borrow_type(params[i])) continue;
        if (index) return std::nullopt;
        index = i;
    }
    return index;
}

std::optional<ExplicitBorrowReturnContract> explicit_borrow_return_contract(
    const std::vector<Attribute>& attributes) {
    const Attribute* attr = find_attribute(attributes, "borrow_return");
    if (!attr) return std::nullopt;
    if (!attr->has_args || attr->args.empty()) {
        fail_borrow_return_attribute(
            attr->loc,
            "attribute '@borrow_return' expects a source parameter path");
    }
    if (attr->args.front().kind != TokenKind::Identifier) {
        fail_borrow_return_attribute(
            attr->loc,
            "attribute '@borrow_return' expects a source parameter path");
    }

    ExplicitBorrowReturnContract contract;
    contract.param_name = attr->args.front().text;
    contract.loc = attr->loc;

    std::size_t index = 1;
    while (index < attr->args.size()) {
        const Token& dot = attr->args[index];
        if (dot.kind != TokenKind::Dot || index + 1 >= attr->args.size()) {
            fail_borrow_return_attribute(
                dot.loc,
                "attribute '@borrow_return' expects a dotted source parameter path");
        }
        const Token& part = attr->args[index + 1];
        BorrowReturnPathComponent component;
        component.loc = part.loc;
        if (part.kind == TokenKind::Identifier) {
            component.kind = BorrowReturnPathComponent::Kind::Field;
            component.text = part.text;
        } else if (part.kind == TokenKind::Integer) {
            component.kind = BorrowReturnPathComponent::Kind::Index;
            component.index = part.int_value;
        } else {
            fail_borrow_return_attribute(
                part.loc,
                "attribute '@borrow_return' path components must be field names or non-negative indexes");
        }
        contract.path.push_back(std::move(component));
        index += 2;
    }

    return contract;
}

} // namespace ari
