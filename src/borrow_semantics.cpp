#include "borrow_semantics.hpp"

#include "common.hpp"
#include "type_semantics.hpp"

#include <optional>
#include <utility>

namespace ari {

namespace {

[[noreturn]] void fail_borrow(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

} // namespace

BorrowContext::BorrowContext(LocalScopeStack& local_scopes) : local_scopes_(local_scopes) {}

void BorrowContext::clear() {
    temporary_borrows_.clear();
}

std::size_t BorrowContext::mark() const {
    return temporary_borrows_.size();
}

void BorrowContext::push_temporary(std::string name, std::string path, bool mutable_borrow) {
    temporary_borrows_.push_back(TemporaryBorrow{std::move(name), std::move(path), mutable_borrow});
}

void BorrowContext::add_source(LocalInfo& source, const std::string& path, bool mutable_borrow) {
    add_local_borrow_source(source, path, mutable_borrow);
}

void BorrowContext::release_source(const std::string& name, const std::string& path, bool mutable_borrow) {
    local_scopes_.release_borrow_source(name, path, mutable_borrow);
}

void BorrowContext::release_named(const LocalInfo& borrow) {
    local_scopes_.release_borrow_sources(borrow);
}

void BorrowContext::promote_to_named(SourceLocation loc,
                                     const IrExpr& init,
                                     const std::string& binding_name,
                                     LocalInfo& binding) {
    std::optional<BorrowResultSource> source = borrow_result_source(init);
    if (!source) {
        fail_borrow(loc, "borrow bindings must be initialized from ref, ref mut, or compatible borrow control-flow results");
    }
    if (temporary_borrows_.empty() ||
        temporary_borrows_.back().name != source->name ||
        temporary_borrows_.back().path != source->path ||
        temporary_borrows_.back().mutable_borrow != source->mutable_borrow) {
        throw CompileError("internal error: borrow binding '" + binding_name + "' did not match the active temporary borrow");
    }
    temporary_borrows_.pop_back();
    set_local_named_borrow_source(binding, source->name, source->path, source->mutable_borrow);
}

void BorrowContext::promote_to_aggregate(std::size_t mark, LocalInfo& binding) {
    if (temporary_borrows_.size() == mark) return;
    for (std::size_t i = mark; i < temporary_borrows_.size(); ++i) {
        const TemporaryBorrow& borrow = temporary_borrows_[i];
        add_local_aggregate_borrow_source(binding, borrow.name, borrow.path, borrow.mutable_borrow);
    }
    temporary_borrows_.resize(mark);
}

void BorrowContext::release_to_mark(std::size_t mark) {
    for (std::size_t i = temporary_borrows_.size(); i > mark; --i) {
        const TemporaryBorrow& borrow = temporary_borrows_[i - 1];
        release_source(borrow.name, borrow.path, borrow.mutable_borrow);
    }
    temporary_borrows_.resize(mark);
}

void require_not_borrowed(SourceLocation loc,
                          const std::string& name,
                          const LocalInfo& local,
                          const std::string& action) {
    if (local_has_active_borrows(local) || local_has_active_field_borrows(local)) {
        fail_borrow(loc, "cannot " + action + " borrowed binding '" + name + "'");
    }
}

void require_can_read_borrow_path(SourceLocation loc,
                                  const std::string& name,
                                  const LocalInfo& local,
                                  const std::string& path) {
    if (path.empty()) {
        if (local_has_mutable_borrows(local) || local_has_mutable_field_borrows(local)) {
            fail_borrow(loc, "cannot read mutably borrowed binding '" + name + "'");
        }
        return;
    }
    if (local_has_mutable_borrows(local) || local_has_overlapping_mutable_field_borrows(local, path)) {
        fail_borrow(loc, "cannot read mutably borrowed field '" + local_borrow_path_display(name, path) + "'");
    }
}

void require_can_assign_borrow_path(SourceLocation loc,
                                    const std::string& name,
                                    const LocalInfo& local,
                                    const std::string& path) {
    if (path.empty()) {
        require_not_borrowed(loc, name, local, "assign to");
        return;
    }
    if (local_has_active_borrows(local) || local_has_overlapping_field_borrows(local, path)) {
        fail_borrow(loc, "cannot assign to borrowed field '" + local_borrow_path_display(name, path) + "'");
    }
}

void require_can_borrow_path(SourceLocation loc,
                             const std::string& name,
                             const LocalInfo& local,
                             const std::string& path,
                             bool mutable_borrow) {
    if (path.empty()) {
        if (mutable_borrow) {
            if (local_has_active_borrows(local) || local_has_active_field_borrows(local)) {
                fail_borrow(loc, "cannot mutably borrow already borrowed binding '" + name + "'");
            }
            return;
        }
        if (local_has_mutable_borrows(local) || local_has_mutable_field_borrows(local)) {
            fail_borrow(loc, "cannot immutably borrow mutably borrowed binding '" + name + "'");
        }
        return;
    }
    if (mutable_borrow) {
        if (local_has_active_borrows(local) || local_has_overlapping_field_borrows(local, path)) {
            fail_borrow(loc, "cannot mutably borrow already borrowed field '" + local_borrow_path_display(name, path) + "'");
        }
        return;
    }
    if (local_has_mutable_borrows(local) || local_has_overlapping_mutable_field_borrows(local, path)) {
        fail_borrow(loc, "cannot immutably borrow mutably borrowed field '" + local_borrow_path_display(name, path) + "'");
    }
}

void require_can_reborrow(SourceLocation loc,
                          const std::string& name,
                          const LocalInfo& borrow,
                          bool mutable_borrow) {
    require_can_reborrow_path(loc, name, borrow, "", mutable_borrow);
}

void require_can_reborrow_path(SourceLocation loc,
                               const std::string& name,
                               const LocalInfo& borrow,
                               const std::string& path,
                               bool mutable_borrow) {
    if (!is_borrow_type(borrow.type)) {
        throw CompileError("internal error: reborrow source '" + name + "' is not a borrow binding");
    }
    if (mutable_borrow && borrow.type.qualifier != TypeQualifier::MutRef) {
        fail_borrow(loc, "cannot mutably reborrow immutable borrow binding '" + name + "'");
    }
    require_can_borrow_path(loc, name, borrow, path, mutable_borrow);
}

std::optional<BorrowResultSource> borrow_result_source(const IrExpr& expr) {
    if (!is_borrow_type(expr.type)) return std::nullopt;
    if (!ir_expr_borrow_source_name(expr).empty()) {
        return BorrowResultSource{
            ir_expr_borrow_source_name(expr),
            ir_expr_borrow_source_path(expr),
            expr.mutable_borrow
        };
    }
    switch (expr.kind) {
        case IrExprKind::Borrow:
        case IrExprKind::If:
        case IrExprKind::Match:
        case IrExprKind::Block:
            break;
        default:
            return std::nullopt;
    }
    const std::string& name = ir_expr_name(expr);
    if (name.empty()) return std::nullopt;
    return BorrowResultSource{name, ir_expr_label(expr), expr.mutable_borrow};
}

void set_borrow_result_source(IrExpr& expr, const BorrowResultSource& source) {
    set_ir_expr_borrow_source(expr, source.name, source.path);
    expr.mutable_borrow = source.mutable_borrow;
}

bool same_borrow_result_source(const BorrowResultSource& left,
                               const BorrowResultSource& right) {
    return left.name == right.name &&
           left.path == right.path &&
           left.mutable_borrow == right.mutable_borrow;
}

std::string append_borrow_path(const std::string& base, const std::string& suffix) {
    if (base.empty()) return suffix;
    if (suffix.empty()) return base;
    return base + "." + suffix;
}

} // namespace ari
