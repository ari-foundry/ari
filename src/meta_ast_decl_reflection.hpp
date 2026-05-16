#pragma once

#include "ast.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace ari {

struct MetaAstNameTypeSummary {
    std::string name;
    std::string type;
};

struct MetaAstCallableSummary {
    std::string name;
    std::string return_type;
    std::size_t generic_count = 0;
    std::vector<MetaAstNameTypeSummary> params;
};

struct MetaAstEnumCaseSummary {
    std::string name;
    std::vector<std::string> payload_types;
};

struct MetaAstDeclInput {
    std::vector<Token> tokens;
    std::size_t count = 0;
    std::string kind = "empty";
    std::string name;
    std::string return_type;
    std::string trait_type;
    bool is_public = false;
    std::size_t generic_count = 0;
    std::size_t param_count = 0;
    std::size_t field_count = 0;
    std::size_t case_count = 0;
    std::size_t method_count = 0;
    std::size_t associated_type_count = 0;
    std::vector<std::string> generics;
    std::vector<MetaAstNameTypeSummary> params;
    std::vector<MetaAstNameTypeSummary> fields;
    std::vector<MetaAstEnumCaseSummary> cases;
    std::vector<MetaAstCallableSummary> methods;
    std::vector<MetaAstNameTypeSummary> associated_types;
};

MetaAstDeclInput summarize_meta_ast_decl_input(const std::vector<Token>& input_tokens,
                                               const Program& parsed_input);

} // namespace ari
