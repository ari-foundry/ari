#pragma once

#include <optional>
#include <string>
#include <vector>

namespace ari {

struct AriBuiltinAlias {
    std::string source_name;
    std::string symbol;
};

const std::vector<AriBuiltinAlias>& ari_builtin_source_aliases();
bool is_ari_builtin_symbol(const std::string& symbol);
std::optional<std::string> ari_builtin_symbol_for_source_name(const std::string& source_name);
std::optional<std::string> ari_builtin_freestanding_blocked_feature(const std::string& source_name);

} // namespace ari
