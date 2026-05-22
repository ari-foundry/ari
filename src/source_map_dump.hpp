#pragma once

#include "common.hpp"

#include <string>
#include <vector>

namespace ari {

struct SourceMapDumpFile {
    SourceId source_id;
    std::string module_name;
    bool is_root = false;
};

std::string dump_source_map(const std::string& source_name, std::vector<SourceMapDumpFile> files);

} // namespace ari
