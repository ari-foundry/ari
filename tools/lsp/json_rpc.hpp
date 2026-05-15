#pragma once

#include <iosfwd>
#include <optional>
#include <string>

namespace ari::lsp {

struct RpcMessage {
    std::string body;
};

std::optional<RpcMessage> read_message(std::istream& in);
void write_message(std::ostream& out, const std::string& body);

std::string json_string_field(const std::string& json, const std::string& key);
std::string json_raw_field(const std::string& json, const std::string& key);

} // namespace ari::lsp
