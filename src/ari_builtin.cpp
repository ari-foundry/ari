#include "ari_builtin.hpp"

#include <initializer_list>
#include <utility>

namespace ari {
namespace {

AriBuiltinTypeExpectation builtin_type(std::string display_name,
                                       std::initializer_list<const char*> aliases = {}) {
    AriBuiltinTypeExpectation type;
    type.display_name = std::move(display_name);
    type.accepted_names.push_back(type.display_name);
    for (const char* alias : aliases) {
        type.accepted_names.emplace_back(alias);
    }
    return type;
}

AriBuiltinSignatureExpectation builtin_sig(std::initializer_list<AriBuiltinTypeExpectation> params,
                                           AriBuiltinTypeExpectation result) {
    AriBuiltinSignatureExpectation signature;
    signature.params.assign(params.begin(), params.end());
    signature.result = std::move(result);
    return signature;
}

} // namespace

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
        {"env::get", "ari_builtin_env_get"},
        {"std::env::get", "ari_builtin_env_get"},
        {"env::has", "ari_builtin_env_has"},
        {"std::env::has", "ari_builtin_env_has"},
        {"env::set", "ari_builtin_env_set"},
        {"std::env::set", "ari_builtin_env_set"},
        {"env::remove", "ari_builtin_env_remove"},
        {"std::env::remove", "ari_builtin_env_remove"},
        {"process::id", "ari_builtin_process_id"},
        {"std::process::id", "ari_builtin_process_id"},
        {"process::exit", "ari_builtin_process_exit"},
        {"std::process::exit", "ari_builtin_process_exit"},
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
    return ari_builtin_signature_for_symbol(symbol).has_value();
}

std::optional<AriBuiltinSignatureExpectation> ari_builtin_signature_for_symbol(const std::string& symbol) {
    const AriBuiltinTypeExpectation i64 = builtin_type("i64");
    const AriBuiltinTypeExpectation u64 = builtin_type("u64");
    const AriBuiltinTypeExpectation u8 = builtin_type("u8");
    const AriBuiltinTypeExpectation boolean = builtin_type("bool");
    const AriBuiltinTypeExpectation void_type = builtin_type("void");
    const AriBuiltinTypeExpectation source_string = builtin_type("string");
    const AriBuiltinTypeExpectation ptr_u8 = builtin_type("ptr u8");
    const AriBuiltinTypeExpectation ptr_c_void = builtin_type("ptr c_void", {"ptr void"});
    const AriBuiltinTypeExpectation own_zone = builtin_type("own Zone");
    const AriBuiltinTypeExpectation ref_mut_zone = builtin_type("ref mut Zone");
    const AriBuiltinTypeExpectation raw_string = builtin_type("std::string::RawString");
    const AriBuiltinTypeExpectation std_string = builtin_type("std::string::String");
    const AriBuiltinTypeExpectation ref_std_string = builtin_type("ref std::string::String");

    if (symbol == "ari_builtin_context_argc") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_context_arg") return builtin_sig({i64}, source_string);
    if (symbol == "ari_builtin_env_get") return builtin_sig({source_string}, source_string);
    if (symbol == "ari_builtin_env_has") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_env_set") return builtin_sig({source_string, source_string}, boolean);
    if (symbol == "ari_builtin_env_remove") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_process_id") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_process_exit") return builtin_sig({i64}, void_type);
    if (symbol == "ari_builtin_write_i64") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_write_u64") return builtin_sig({u64}, i64);
    if (symbol == "ari_builtin_write_bool") return builtin_sig({boolean}, i64);
    if (symbol == "ari_builtin_write_byte") return builtin_sig({u8}, i64);
    if (symbol == "ari_builtin_newline") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_read_byte") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_read_line") return builtin_sig({}, source_string);
    if (symbol == "ari_builtin_read_line_owned") return builtin_sig({ref_mut_zone}, std_string);
    if (symbol == "ari_builtin_zone_create") return builtin_sig({i64}, own_zone);
    if (symbol == "ari_builtin_zone_alloc") return builtin_sig({ref_mut_zone, i64, i64}, ptr_u8);
    if (symbol == "ari_builtin_zone_allocation_zone") return builtin_sig({ptr_u8}, ptr_c_void);
    if (symbol == "ari_builtin_string_alloc_buffer") return builtin_sig({ref_mut_zone, i64}, ptr_u8);
    if (symbol == "ari_builtin_string_with_capacity") return builtin_sig({ref_mut_zone, i64}, raw_string);
    if (symbol == "ari_builtin_string_new") return builtin_sig({ref_mut_zone, i64}, std_string);
    if (symbol == "ari_builtin_string_from_string") return builtin_sig({ref_mut_zone, source_string}, std_string);
    if (symbol == "ari_builtin_string_copy_to") return builtin_sig({ref_std_string, ref_mut_zone}, std_string);
    if (symbol == "ari_builtin_zone_reset") return builtin_sig({ref_mut_zone}, void_type);
    if (symbol == "ari_builtin_zone_destroy") return builtin_sig({own_zone}, void_type);
    if (symbol == "ari_builtin_assert") return builtin_sig({boolean}, i64);
    if (symbol == "ari_builtin_assert_eq_i64") return builtin_sig({i64, i64}, i64);
    if (symbol == "ari_builtin_assert_ne_i64") return builtin_sig({i64, i64}, i64);
    if (symbol == "ari_builtin_assert_eq_bool") return builtin_sig({boolean, boolean}, i64);
    if (symbol == "ari_builtin_assert_ne_bool") return builtin_sig({boolean, boolean}, i64);
    if (symbol == "ari_builtin_panic") return builtin_sig({}, void_type);
    return std::nullopt;
}

std::optional<std::string> ari_builtin_symbol_for_source_name(const std::string& source_name) {
    for (const auto& alias : ari_builtin_source_aliases()) {
        if (alias.source_name == source_name) return alias.symbol;
    }
    return std::nullopt;
}

} // namespace ari
