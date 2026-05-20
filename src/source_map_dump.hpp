#pragma once

#include <string>
#include <vector>

namespace ari {

struct SourceMapDumpFile {
    std::string module_name;
    std::string path;
    std::string text;
    bool is_root = false;
};

std::string dump_source_map(const std::string& source_name, std::vector<SourceMapDumpFile> files);

} // namespace ari
