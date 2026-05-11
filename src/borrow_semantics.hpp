#pragma once

#include "local_state.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace ari {

struct BorrowResultSource {
    std::string name;
    std::string path;
    bool mutable_borrow = false;
};

struct TemporaryBorrow {
    std::string name;
    std::string path;
    bool mutable_borrow = false;
};

class BorrowContext {
public:
    explicit BorrowContext(LocalScopeStack& local_scopes);

    void clear();
    std::size_t mark() const;
    void push_temporary(std::string name, std::string path, bool mutable_borrow);
    void add_source(LocalInfo& source, const std::string& path, bool mutable_borrow);
    void release_source(const std::string& name, const std::string& path, bool mutable_borrow);
    void release_named(const LocalInfo& borrow);
    void promote_to_named(SourceLocation loc,
                          const IrExpr& init,
                          const std::string& binding_name,
                          LocalInfo& binding);
    void promote_to_aggregate(std::size_t mark, LocalInfo& binding);
    void release_to_mark(std::size_t mark);

private:
    LocalScopeStack& local_scopes_;
    std::vector<TemporaryBorrow> temporary_borrows_;
};

void require_not_borrowed(SourceLocation loc,
                          const std::string& name,
                          const LocalInfo& local,
                          const std::string& action);
void require_can_read_borrow_path(SourceLocation loc,
                                  const std::string& name,
                                  const LocalInfo& local,
                                  const std::string& path);
void require_can_assign_borrow_path(SourceLocation loc,
                                    const std::string& name,
                                    const LocalInfo& local,
                                    const std::string& path);
void require_can_borrow_path(SourceLocation loc,
                             const std::string& name,
                             const LocalInfo& local,
                             const std::string& path,
                             bool mutable_borrow);
void require_can_reborrow(SourceLocation loc,
                          const std::string& name,
                          const LocalInfo& borrow,
                          bool mutable_borrow);
std::optional<BorrowResultSource> borrow_result_source(const IrExpr& expr);
void set_borrow_result_source(IrExpr& expr, const BorrowResultSource& source);
bool same_borrow_result_source(const BorrowResultSource& left,
                               const BorrowResultSource& right);

} // namespace ari
