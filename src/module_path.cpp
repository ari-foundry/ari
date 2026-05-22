#include "module_path.hpp"

#include <cstddef>

namespace ari {

bool is_qualified_path(const std::string& path) {
    return path.find("::") != std::string::npos;
}

std::vector<std::string> split_qualified_path(const std::string& path) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    for (;;) {
        std::size_t split = path.find("::", start);
        if (split == std::string::npos) {
            parts.push_back(path.substr(start));
            break;
        }
        parts.push_back(path.substr(start, split - start));
        start = split + 2;
    }
    return parts;
}

std::string join_qualified_path(const std::vector<std::string>& parts) {
    return join_qualified_path(parts, 0, parts.size());
}

std::string join_qualified_path(const std::vector<std::string>& parts,
                                std::size_t begin,
                                std::size_t end) {
    std::string result;
    for (std::size_t i = begin; i < end; ++i) {
        if (!result.empty()) result += "::";
        result += parts[i];
    }
    return result;
}

std::string qualified_basename(const std::string& path) {
    std::size_t split = path.rfind("::");
    if (split == std::string::npos) return path;
    return path.substr(split + 2);
}

std::string qualified_parent(const std::string& path) {
    std::size_t split = path.rfind("::");
    if (split == std::string::npos) return "";
    return path.substr(0, split);
}

std::string resolve_relative_path(SourceLocation loc,
                                  const std::string& current_module,
                                  const std::string& path) {
    std::vector<std::string> parts = split_qualified_path(path);
    if (parts.empty()) return path;
    if (parts[0] != "self" && parts[0] != "super") return path;

    std::vector<std::string> resolved;
    if (!current_module.empty()) resolved = split_qualified_path(current_module);

    std::size_t i = 0;
    if (parts[0] == "self") {
        i = 1;
    } else {
        while (i < parts.size() && parts[i] == "super") {
            if (resolved.empty()) {
                throw CompileError(loc, "super:: path cannot escape the root module");
            }
            resolved.pop_back();
            ++i;
        }
    }

    for (; i < parts.size(); ++i) resolved.push_back(parts[i]);
    return join_qualified_path(resolved);
}

} // namespace ari
