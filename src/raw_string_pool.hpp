#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ari {

struct RawStringReference {
    std::size_t imm_offset = 0;
    std::string value;
};

class RawStringPool {
public:
    void patch_references(std::vector<std::uint8_t>& image,
                          const std::vector<RawStringReference>& references);

private:
    std::size_t string_offset(std::vector<std::uint8_t>& image, const std::string& value);

    std::map<std::string, std::size_t> offsets_;
};

} // namespace ari
