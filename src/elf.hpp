#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ari {

struct ElfSymbol {
    std::string name;
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
};

std::vector<std::uint8_t> write_elf_executable(const std::vector<std::uint8_t>& code,
                                               const std::vector<ElfSymbol>& symbols);
std::vector<std::uint8_t> write_elf_executable(const std::vector<std::uint8_t>& code);
std::vector<std::uint8_t> write_elf_relocatable_object(const std::vector<std::uint8_t>& code,
                                                       const std::vector<ElfSymbol>& symbols);

} // namespace ari
