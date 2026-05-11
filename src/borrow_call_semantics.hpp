#pragma once

#include "borrow_semantics.hpp"
#include "ir.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace ari {

struct BorrowCallContract {
    IrType result;
    std::optional<std::size_t> source_param_index;
    std::string return_path;
    bool is_extern = false;
};

struct BorrowCallLocalAdapter {
    std::function<LocalInfo*(const std::string&)> find_local;
    std::function<LocalInfo&(SourceLocation, const std::string&)> require_live_local;
    std::function<void(LocalInfo&, const std::string&, bool)> add_borrow_source;
    std::function<void(std::string, std::string, bool)> push_temporary_borrow;
};

std::optional<BorrowResultSource> expr_borrow_result_source(const IrExpr& value);

BorrowResultSource root_borrow_result_source(BorrowResultSource source,
                                             const BorrowCallLocalAdapter& locals);

std::optional<BorrowResultSource> call_borrow_result_source(SourceLocation loc,
                                                           const std::string& display_name,
                                                           const BorrowCallContract& contract,
                                                           const std::vector<IrExprPtr>& args);

void activate_borrow_result(SourceLocation loc,
                            IrExpr& result,
                            const BorrowResultSource& source,
                            const BorrowCallLocalAdapter& locals);

} // namespace ari
