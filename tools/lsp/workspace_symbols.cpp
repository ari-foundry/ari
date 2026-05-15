#include "workspace_symbols.hpp"

#include "documents.hpp"
#include "symbols.hpp"

#include "../ari_tooling/diagnostic.hpp"

#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace ari::lsp {
namespace {

struct WorkspaceSymbol {
    Symbol symbol;
    std::string path;
};

bool is_directory(const std::string& path) {
    struct stat info {};
    return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

bool is_regular_file(const std::string& path) {
    struct stat info {};
    return stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode);
}

bool should_skip_dir(const std::string& name) {
    return name == "." ||
           name == ".." ||
           name == ".git" ||
           name == ".cache" ||
           name == "build" ||
           name == "node_modules";
}

bool has_ari_extension(const std::string& path) {
    return path.size() >= 4 &&
           (path.rfind(".ari") == path.size() - 4 ||
            (path.size() >= 5 && path.rfind(".arih") == path.size() - 5));
}

std::string join_path(const std::string& dir, const std::string& name) {
    if (dir.empty() || dir == ".") return name;
    if (dir.back() == '/') return dir + name;
    return dir + "/" + name;
}

std::string read_file_text(const std::string& path) {
    std::ifstream in(path);
    if (!in) return "";
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

std::string lower_ascii(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return text;
}

bool matches_query(const Symbol& symbol, const std::string& query) {
    if (query.empty()) return true;
    std::string needle = lower_ascii(query);
    std::string name = lower_ascii(symbol.name);
    std::string label = lower_ascii(symbol.label);
    return name.find(needle) != std::string::npos || label.find(needle) != std::string::npos;
}

void collect_ari_files(const std::string& root, std::vector<std::string>& files) {
    DIR* dir = opendir(root.c_str());
    if (!dir) return;
    while (dirent* entry = readdir(dir)) {
        std::string name = entry->d_name;
        std::string path = join_path(root, name);
        if (is_directory(path)) {
            if (!should_skip_dir(name)) collect_ari_files(path, files);
            continue;
        }
        if (is_regular_file(path) && has_ari_extension(path)) {
            files.push_back(path);
        }
    }
    closedir(dir);
}

std::vector<WorkspaceSymbol> collect_workspace_symbols(const std::string& root_path, const std::string& query) {
    std::vector<std::string> files;
    collect_ari_files(root_path.empty() ? "." : root_path, files);

    std::vector<WorkspaceSymbol> symbols;
    for (const std::string& file : files) {
        for (const Symbol& symbol : collect_symbols(read_file_text(file))) {
            if (!matches_query(symbol, query)) continue;
            symbols.push_back(WorkspaceSymbol{symbol, file});
        }
    }
    return symbols;
}

std::string workspace_symbol_json(const WorkspaceSymbol& symbol) {
    std::ostringstream out;
    out << "{";
    out << "\"name\":\"" << tooling::json_escape(symbol.symbol.name) << "\",";
    out << "\"kind\":" << symbol.symbol.kind << ",";
    out << "\"location\":" << symbol_location_json(path_to_uri(symbol.path), symbol.symbol) << ",";
    out << "\"containerName\":\"" << tooling::json_escape(symbol.path) << "\"";
    out << "}";
    return out.str();
}

} // namespace

std::string workspace_symbols_response(const std::string& id,
                                       const std::string& root_path,
                                       const std::string& query) {
    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"id\":" << (id.empty() ? "null" : id) << ",";
    out << "\"result\":[";
    bool first = true;
    for (const WorkspaceSymbol& symbol : collect_workspace_symbols(root_path, query)) {
        if (!first) out << ",";
        first = false;
        out << workspace_symbol_json(symbol);
    }
    out << "]}";
    return out.str();
}

} // namespace ari::lsp
