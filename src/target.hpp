#pragma once

#include <string>

namespace ari {

struct TargetInfo {
    std::string triple;
    std::string arch;
    bool linux = false;
    bool macos = false;
    bool windows = false;
    bool unix = false;
    unsigned pointer_bits = 64;
    unsigned long_bits = 64;
    bool plain_char_signed = true;
};

std::string default_target_triple();
TargetInfo resolve_target_info(const std::string& triple);
bool target_predicate_active(const TargetInfo& target, const std::string& name);

} // namespace ari
