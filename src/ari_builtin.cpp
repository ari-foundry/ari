#include "ari_builtin.hpp"

namespace ari {

const std::vector<AriBuiltinAlias>& ari_builtin_source_aliases() {
    static const std::vector<AriBuiltinAlias> aliases = {
        {"write_i64", "ari_builtin_write_i64"},
        {"std::write_i64", "ari_builtin_write_i64"},
        {"io::write_i64", "ari_builtin_write_i64"},
        {"std::io::write_i64", "ari_builtin_write_i64"},
        {"write_u64", "ari_builtin_write_u64"},
        {"std::write_u64", "ari_builtin_write_u64"},
        {"io::write_u64", "ari_builtin_write_u64"},
        {"std::io::write_u64", "ari_builtin_write_u64"},
        {"write_bool", "ari_builtin_write_bool"},
        {"std::write_bool", "ari_builtin_write_bool"},
        {"io::write_bool", "ari_builtin_write_bool"},
        {"std::io::write_bool", "ari_builtin_write_bool"},
        {"write_byte", "ari_builtin_write_byte"},
        {"std::write_byte", "ari_builtin_write_byte"},
        {"io::write_byte", "ari_builtin_write_byte"},
        {"std::io::write_byte", "ari_builtin_write_byte"},
        {"newline", "ari_builtin_newline"},
        {"std::newline", "ari_builtin_newline"},
        {"io::newline", "ari_builtin_newline"},
        {"std::io::newline", "ari_builtin_newline"},
        {"read_byte", "ari_builtin_read_byte"},
        {"std::read_byte", "ari_builtin_read_byte"},
        {"io::read_byte", "ari_builtin_read_byte"},
        {"std::io::read_byte", "ari_builtin_read_byte"},
        {"input::read_byte", "ari_builtin_read_byte"},
        {"std::input::read_byte", "ari_builtin_read_byte"},
        {"read_line", "ari_builtin_read_line"},
        {"std::read_line", "ari_builtin_read_line"},
        {"io::read_line", "ari_builtin_read_line"},
        {"std::io::read_line", "ari_builtin_read_line"},
        {"input", "ari_builtin_read_line"},
        {"std::input", "ari_builtin_read_line"},
        {"input::line", "ari_builtin_read_line"},
        {"std::input::line", "ari_builtin_read_line"},
        {"read_line_owned", "ari_builtin_read_line_owned"},
        {"std::read_line_owned", "ari_builtin_read_line_owned"},
        {"io::read_line_owned", "ari_builtin_read_line_owned"},
        {"std::io::read_line_owned", "ari_builtin_read_line_owned"},
        {"input_owned", "ari_builtin_read_line_owned"},
        {"std::input_owned", "ari_builtin_read_line_owned"},
        {"input::owned_line", "ari_builtin_read_line_owned"},
        {"std::input::owned_line", "ari_builtin_read_line_owned"},
        {"arg_count", "ari_builtin_context_argc"},
        {"std::arg_count", "ari_builtin_context_argc"},
        {"context::argc", "ari_builtin_context_argc"},
        {"std::context::argc", "ari_builtin_context_argc"},
        {"arg", "ari_builtin_context_arg"},
        {"std::arg", "ari_builtin_context_arg"},
        {"context::arg", "ari_builtin_context_arg"},
        {"std::context::arg", "ari_builtin_context_arg"},
        {"create", "ari_builtin_zone_create"},
        {"std::create", "ari_builtin_zone_create"},
        {"zone::create", "ari_builtin_zone_create"},
        {"std::zone::create", "ari_builtin_zone_create"},
        {"temp", "ari_builtin_zone_create"},
        {"std::temp", "ari_builtin_zone_create"},
        {"zone::temp", "ari_builtin_zone_create"},
        {"std::zone::temp", "ari_builtin_zone_create"},
        {"alloc", "ari_builtin_zone_alloc"},
        {"std::alloc", "ari_builtin_zone_alloc"},
        {"zone::alloc", "ari_builtin_zone_alloc"},
        {"std::zone::alloc", "ari_builtin_zone_alloc"},
        {"allocation_zone", "ari_builtin_zone_allocation_zone"},
        {"std::allocation_zone", "ari_builtin_zone_allocation_zone"},
        {"zone::allocation_zone", "ari_builtin_zone_allocation_zone"},
        {"std::zone::allocation_zone", "ari_builtin_zone_allocation_zone"},
        {"string::alloc_buffer", "ari_builtin_string_alloc_buffer"},
        {"std::string::alloc_buffer", "ari_builtin_string_alloc_buffer"},
        {"string::with_capacity", "ari_builtin_string_with_capacity"},
        {"std::string::with_capacity", "ari_builtin_string_with_capacity"},
        {"string::new", "ari_builtin_string_new"},
        {"std::string::new", "ari_builtin_string_new"},
        {"string::from_string", "ari_builtin_string_from_string"},
        {"std::string::from_string", "ari_builtin_string_from_string"},
        {"string::copy_to", "ari_builtin_string_copy_to"},
        {"std::string::copy_to", "ari_builtin_string_copy_to"},
        {"reset", "ari_builtin_zone_reset"},
        {"std::reset", "ari_builtin_zone_reset"},
        {"zone::reset", "ari_builtin_zone_reset"},
        {"std::zone::reset", "ari_builtin_zone_reset"},
        {"destroy", "ari_builtin_zone_destroy"},
        {"std::destroy", "ari_builtin_zone_destroy"},
        {"zone::destroy", "ari_builtin_zone_destroy"},
        {"std::zone::destroy", "ari_builtin_zone_destroy"},
        {"assert", "ari_builtin_assert"},
        {"std::assert", "ari_builtin_assert"},
        {"debug_assert", "ari_builtin_assert"},
        {"std::debug_assert", "ari_builtin_assert"},
        {"assert_eq_i64", "ari_builtin_assert_eq_i64"},
        {"std::assert_eq_i64", "ari_builtin_assert_eq_i64"},
        {"assert_ne_i64", "ari_builtin_assert_ne_i64"},
        {"std::assert_ne_i64", "ari_builtin_assert_ne_i64"},
        {"assert_eq_bool", "ari_builtin_assert_eq_bool"},
        {"std::assert_eq_bool", "ari_builtin_assert_eq_bool"},
        {"assert_ne_bool", "ari_builtin_assert_ne_bool"},
        {"std::assert_ne_bool", "ari_builtin_assert_ne_bool"},
        {"panic", "ari_builtin_panic"},
        {"std::panic", "ari_builtin_panic"},
        {"todo", "ari_builtin_panic"},
        {"std::todo", "ari_builtin_panic"},
        {"unreachable", "ari_builtin_panic"},
        {"std::unreachable", "ari_builtin_panic"},
    };
    return aliases;
}

bool is_ari_builtin_symbol(const std::string& symbol) {
    return symbol == "ari_builtin_context_argc" ||
           symbol == "ari_builtin_context_arg" ||
           symbol == "ari_builtin_write_i64" ||
           symbol == "ari_builtin_write_u64" ||
           symbol == "ari_builtin_write_bool" ||
           symbol == "ari_builtin_write_byte" ||
           symbol == "ari_builtin_newline" ||
           symbol == "ari_builtin_read_byte" ||
           symbol == "ari_builtin_read_line" ||
           symbol == "ari_builtin_read_line_owned" ||
           symbol == "ari_builtin_zone_create" ||
           symbol == "ari_builtin_zone_alloc" ||
           symbol == "ari_builtin_zone_allocation_zone" ||
           symbol == "ari_builtin_string_alloc_buffer" ||
           symbol == "ari_builtin_string_with_capacity" ||
           symbol == "ari_builtin_string_new" ||
           symbol == "ari_builtin_string_from_string" ||
           symbol == "ari_builtin_string_copy_to" ||
           symbol == "ari_builtin_zone_reset" ||
           symbol == "ari_builtin_zone_destroy" ||
           symbol == "ari_builtin_assert" ||
           symbol == "ari_builtin_assert_eq_i64" ||
           symbol == "ari_builtin_assert_ne_i64" ||
           symbol == "ari_builtin_assert_eq_bool" ||
           symbol == "ari_builtin_assert_ne_bool" ||
           symbol == "ari_builtin_panic";
}

std::optional<std::string> ari_builtin_symbol_for_source_name(const std::string& source_name) {
    for (const auto& alias : ari_builtin_source_aliases()) {
        if (alias.source_name == source_name) return alias.symbol;
    }
    return std::nullopt;
}

std::optional<std::string> ari_builtin_freestanding_blocked_feature(const std::string& source_name) {
    if (source_name == "new" ||
        source_name == "std::new" ||
        source_name == "zone::new" ||
        source_name == "std::zone::new" ||
        source_name == "promote" ||
        source_name == "std::promote" ||
        source_name == "zone::promote" ||
        source_name == "std::zone::promote") {
        return "zone allocation";
    }

    std::optional<std::string> symbol = ari_builtin_symbol_for_source_name(source_name);
    if (!symbol) return std::nullopt;
    if (*symbol == "ari_builtin_read_line" ||
        *symbol == "ari_builtin_read_line_owned") {
        return "line input helpers";
    }
    if (*symbol == "ari_builtin_context_argc" || *symbol == "ari_builtin_context_arg") {
        return "process argument helpers";
    }
    if (*symbol == "ari_builtin_zone_create" ||
        *symbol == "ari_builtin_zone_alloc" ||
        *symbol == "ari_builtin_zone_allocation_zone" ||
        *symbol == "ari_builtin_string_alloc_buffer" ||
        *symbol == "ari_builtin_string_with_capacity" ||
        *symbol == "ari_builtin_string_new" ||
        *symbol == "ari_builtin_string_from_string" ||
        *symbol == "ari_builtin_string_copy_to" ||
        *symbol == "ari_builtin_zone_reset" ||
        *symbol == "ari_builtin_zone_destroy") {
        return "zone allocation";
    }
    return std::nullopt;
}

} // namespace ari
