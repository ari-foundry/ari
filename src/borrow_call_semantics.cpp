#include "borrow_call_semantics.hpp"

#include "common.hpp"
#include "type_semantics.hpp"

#include <set>
#include <utility>

namespace ari {

namespace {

[[noreturn]] void fail_borrow_call(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

} // namespace

std::optional<BorrowResultSource> expr_borrow_result_source(const IrExpr& value) {
    std::optional<BorrowResultSource> source = borrow_result_source(value);
    if (source) return source;
    if (value.kind == IrExprKind::Local && is_borrow_type(value.type)) {
        return BorrowResultSource{
            ir_expr_name(value),
            "",
            value.type.qualifier == TypeQualifier::MutRef
        };
    }
    return std::nullopt;
}

BorrowResultSource root_borrow_result_source(BorrowResultSource source,
                                             const BorrowCallLocalAdapter& locals) {
    std::set<std::string> seen;
    while (true) {
        LocalInfo* local = locals.find_local(source.name);
        if (!local) {
            throw CompileError("internal error: missing borrow result source '" + source.name + "'");
        }
        if (!is_borrow_type(local->type) || local->borrow_source.empty()) return source;
        if (!seen.insert(source.name).second) {
            throw CompileError("internal error: cyclic borrow source for '" + source.name + "'");
        }
        source.path = append_borrow_path(local->borrow_source_path, source.path);
        source.name = local->borrow_source;
        source.mutable_borrow = source.mutable_borrow || local->borrow_source_mutable;
    }
}

std::optional<BorrowResultSource> call_borrow_result_source(SourceLocation loc,
                                                           const std::string& display_name,
                                                           const BorrowCallContract& contract,
                                                           const std::vector<IrExprPtr>& args) {
    if (!is_borrow_type(contract.result)) return std::nullopt;
    if (contract.is_extern) {
        fail_borrow_call(
            loc,
            "extern borrow-returning function '" + display_name +
                "' cannot return tracked Ari borrow values yet");
    }
    if (!contract.source_param_index || *contract.source_param_index >= args.size()) {
        fail_borrow_call(
            loc,
            "borrow-returning function '" + display_name +
                "' currently requires exactly one borrow parameter so the result source can be tracked");
    }
    std::optional<BorrowResultSource> source =
        expr_borrow_result_source(*args[*contract.source_param_index]);
    if (!source) {
        fail_borrow_call(
            loc,
            "borrow-returning function '" + display_name +
                "' result source argument must be ref, ref mut, a borrow binding, or a compatible borrow control-flow result");
    }
    source->path = append_borrow_path(source->path, contract.return_path);
    source->mutable_borrow = contract.result.qualifier == TypeQualifier::MutRef;
    return source;
}

void activate_borrow_result(SourceLocation loc,
                            IrExpr& result,
                            const BorrowResultSource& source,
                            const BorrowCallLocalAdapter& locals) {
    LocalInfo& local = locals.require_live_local(loc, source.name);
    if (is_borrow_type(local.type)) {
        require_can_reborrow_path(loc, source.name, local, source.path, source.mutable_borrow);
    } else {
        require_can_borrow_path(loc, source.name, local, source.path, source.mutable_borrow);
    }
    locals.add_borrow_source(local, source.path, source.mutable_borrow);
    locals.push_temporary_borrow(source.name, source.path, source.mutable_borrow);
    set_borrow_result_source(result, source);
}

} // namespace ari
