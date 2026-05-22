#pragma once

#include <cstddef>
#include <string>

namespace ari {

bool is_format_print_name(const std::string& name);
bool is_println_name(const std::string& name);
bool is_eprintln_name(const std::string& name);

bool is_planned_prelude_function_name(const std::string& name);
std::string planned_prelude_function_message(const std::string& name);

bool is_prelude_range_function_name(const std::string& name);
bool is_prelude_inclusive_range_function_name(const std::string& name);
bool is_prelude_vec_len_function_name(const std::string& name);

bool is_prelude_pointer_offset_function_name(const std::string& name);
bool is_prelude_pointer_add_function_name(const std::string& name);
bool is_prelude_pointer_load_function_name(const std::string& name);
bool is_prelude_pointer_store_function_name(const std::string& name);
bool is_prelude_mem_replace_function_name(const std::string& name);
bool is_prelude_mem_swap_function_name(const std::string& name);
bool is_prelude_size_of_function_name(const std::string& name);
bool is_prelude_align_of_function_name(const std::string& name);
bool is_prelude_move_function_name(const std::string& name);
bool is_prelude_take_function_name(const std::string& name);

bool is_zone_alloc_function_name(const std::string& name);
bool is_zone_new_function_name(const std::string& name);
bool is_zone_promote_function_name(const std::string& name);
bool is_zone_scratch_function_name(const std::string& name);
bool is_zone_temp_function_name(const std::string& name);

bool is_source_declared_prelude_special_name(const std::string& name);

bool planned_prelude_type_arity(const std::string& name, std::size_t& arity);
std::string planned_prelude_type_message(const std::string& name);

} // namespace ari
