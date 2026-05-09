#pragma once

#include "module_cache.hpp"

namespace ari {

ModuleCacheAstSummary make_module_cache_ast_summary(const std::string& path,
                                                    const std::string& content_hash,
                                                    const std::vector<std::string>& module_path,
                                                    const Program& program,
                                                    bool is_root);
void require_valid_module_cache_ast_summary_payload(const ModuleCacheAstSummary& summary,
                                                    const std::string& display_path);
Program materialize_module_cache_ast_summary_declarations(const ModuleCacheAstSummary& summary,
                                                          const std::string& display_path);
bool can_load_module_cache_ast_summary_declarations(const Program& program);

} // namespace ari
