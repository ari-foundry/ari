#pragma once

#include "ir.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ari {

struct RawVTableReference {
    std::size_t imm_offset = 0;
    std::string name;
};

struct RawVTableEntryPatch {
    std::size_t entry_offset = 0;
    std::string target_name;
};

class RawVTablePool {
public:
    void emit_tables(std::vector<std::uint8_t>& image,
                     const std::vector<IrTraitObjectVTable>& tables);
    void patch_references(std::vector<std::uint8_t>& image,
                          const std::vector<RawVTableReference>& references) const;

    const std::vector<RawVTableEntryPatch>& entry_patches() const { return entry_patches_; }

private:
    std::map<std::string, std::size_t> offsets_;
    std::vector<RawVTableEntryPatch> entry_patches_;
};

} // namespace ari
