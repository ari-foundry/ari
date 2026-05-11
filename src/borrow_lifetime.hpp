#pragma once

#include "ast.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace ari {

using NameUseCounts = std::map<std::string, std::size_t>;

struct StatementNameUses {
    std::vector<NameUseCounts> per_statement;
    NameUseCounts remaining;
};

StatementNameUses collect_statement_name_uses(const std::vector<StmtPtr>& statements);
void subtract_name_uses(NameUseCounts& remaining, const NameUseCounts& used);
bool has_remaining_name_use(const NameUseCounts& remaining, const std::string& name);

} // namespace ari
