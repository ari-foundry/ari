#include "borrow_return_semantics.hpp"

#include "type_semantics.hpp"

namespace ari {

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

} // namespace ari
