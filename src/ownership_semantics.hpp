#pragma once

#include "local_state.hpp"

#include <map>
#include <string>

namespace ari {

void collect_owned_field_states(const IrType& type,
                                const std::string& path,
                                std::map<std::string, LocalState>& states);

void initialize_owned_field_states(LocalInfo& local);
bool initialize_owned_field_states_from_direct_enum_constructor(LocalInfo& local,
                                                               const IrExpr& init);

}
