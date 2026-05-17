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

struct ElfRelocation {
    std::uint64_t offset = 0;
    std::string symbol;
    std::uint32_t type = 0;
    std::int64_t addend = 0;
};

std::vector<std::uint8_t> write_elf_executable(const std::vector<std::uint8_t>& code,
                                               const std::vector<ElfSymbol>& symbols);
std::vector<std::uint8_t> write_elf_executable(const std::vector<std::uint8_t>& code);
std::vector<std::uint8_t> write_elf_relocatable_object(const std::vector<std::uint8_t>& code,
                                                       const std::vector<ElfSymbol>& symbols);
std::vector<std::uint8_t> write_elf_relocatable_object(const std::vector<std::uint8_t>& code,
                                                       const std::vector<ElfSymbol>& symbols,
                                                       const std::vector<ElfRelocation>& relocations);

} // namespace ari
