#pragma once

#include <optional>
#include <string>
#include <vector>

namespace ari {

struct AriBuiltinAlias {
    std::string source_name;
    std::string symbol;
};

struct AriBuiltinTypeExpectation {
    std::string display_name;
    std::vector<std::string> accepted_names;
};

struct AriBuiltinSignatureExpectation {
    std::vector<AriBuiltinTypeExpectation> params;
    AriBuiltinTypeExpectation result;
};

const std::vector<AriBuiltinAlias>& ari_builtin_source_aliases();
bool is_ari_builtin_symbol(const std::string& symbol);
std::optional<AriBuiltinSignatureExpectation> ari_builtin_signature_for_symbol(const std::string& symbol);
std::optional<std::string> ari_builtin_symbol_for_source_name(const std::string& source_name);

} // namespace ari
