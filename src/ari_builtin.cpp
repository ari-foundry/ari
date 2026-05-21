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
        {"io::write_error_byte", "ari_builtin_write_error_byte"},
        {"std::io::write_error_byte", "ari_builtin_write_error_byte"},
        {"io::read_fd_byte", "ari_builtin_os_read_byte"},
        {"std::io::read_fd_byte", "ari_builtin_os_read_byte"},
        {"io::write_fd_byte", "ari_builtin_os_write_byte"},
        {"std::io::write_fd_byte", "ari_builtin_os_write_byte"},
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
        {"context::thread_id", "ari_builtin_context_thread_id"},
        {"std::context::thread_id", "ari_builtin_context_thread_id"},
        {"context::cwd", "ari_builtin_context_cwd"},
        {"std::context::cwd", "ari_builtin_context_cwd"},
        {"context::executable_path", "ari_builtin_context_executable_path"},
        {"std::context::executable_path", "ari_builtin_context_executable_path"},
        {"target::triple", "ari_builtin_target_triple"},
        {"std::target::triple", "ari_builtin_target_triple"},
        {"target::arch", "ari_builtin_target_arch"},
        {"std::target::arch", "ari_builtin_target_arch"},
        {"target::arch_name", "ari_builtin_target_arch_name"},
        {"std::target::arch_name", "ari_builtin_target_arch_name"},
        {"target::os", "ari_builtin_target_os"},
        {"std::target::os", "ari_builtin_target_os"},
        {"target::os_name", "ari_builtin_target_os_name"},
        {"std::target::os_name", "ari_builtin_target_os_name"},
        {"target::env", "ari_builtin_target_env"},
        {"std::target::env", "ari_builtin_target_env"},
        {"target::env_name", "ari_builtin_target_env_name"},
        {"std::target::env_name", "ari_builtin_target_env_name"},
        {"target::object_format", "ari_builtin_target_object_format"},
        {"std::target::object_format", "ari_builtin_target_object_format"},
        {"target::debug_format", "ari_builtin_target_debug_format"},
        {"std::target::debug_format", "ari_builtin_target_debug_format"},
        {"target::errno_abi", "ari_builtin_target_errno_abi"},
        {"std::target::errno_abi", "ari_builtin_target_errno_abi"},
        {"target::pointer_bits", "ari_builtin_target_pointer_bits"},
        {"std::target::pointer_bits", "ari_builtin_target_pointer_bits"},
        {"target::long_bits", "ari_builtin_target_long_bits"},
        {"std::target::long_bits", "ari_builtin_target_long_bits"},
        {"env::get", "ari_builtin_env_get"},
        {"std::env::get", "ari_builtin_env_get"},
        {"env::has", "ari_builtin_env_has"},
        {"std::env::has", "ari_builtin_env_has"},
        {"env::set", "ari_builtin_env_set"},
        {"std::env::set", "ari_builtin_env_set"},
        {"env::remove", "ari_builtin_env_remove"},
        {"std::env::remove", "ari_builtin_env_remove"},
        {"env::current_dir", "ari_builtin_env_current_dir"},
        {"std::env::current_dir", "ari_builtin_env_current_dir"},
        {"env::set_current_dir", "ari_builtin_env_set_current_dir"},
        {"std::env::set_current_dir", "ari_builtin_env_set_current_dir"},
        {"env::executable_path", "ari_builtin_env_executable_path"},
        {"std::env::executable_path", "ari_builtin_env_executable_path"},
        {"process::id", "ari_builtin_process_id"},
        {"std::process::id", "ari_builtin_process_id"},
        {"process::uid", "ari_builtin_process_uid"},
        {"std::process::uid", "ari_builtin_process_uid"},
        {"process::gid", "ari_builtin_process_gid"},
        {"std::process::gid", "ari_builtin_process_gid"},
        {"process::exit", "ari_builtin_process_exit"},
        {"std::process::exit", "ari_builtin_process_exit"},
        {"process::abort", "ari_builtin_process_abort"},
        {"std::process::abort", "ari_builtin_process_abort"},
        {"process::fork", "ari_builtin_process_fork"},
        {"std::process::fork", "ari_builtin_process_fork"},
        {"process::wait", "ari_builtin_process_wait"},
        {"std::process::wait", "ari_builtin_process_wait"},
        {"thread::spawn", "ari_builtin_thread_spawn"},
        {"std::thread::spawn", "ari_builtin_thread_spawn"},
        {"thread::join", "ari_builtin_thread_join"},
        {"std::thread::join", "ari_builtin_thread_join"},
        {"thread::yield_now", "ari_builtin_thread_yield"},
        {"std::thread::yield_now", "ari_builtin_thread_yield"},
        {"thread::available_parallelism", "ari_builtin_thread_available_parallelism"},
        {"std::thread::available_parallelism", "ari_builtin_thread_available_parallelism"},
        {"sync::load", "ari_builtin_sync_atomic_i64_load"},
        {"std::sync::load", "ari_builtin_sync_atomic_i64_load"},
        {"sync::store", "ari_builtin_sync_atomic_i64_store"},
        {"std::sync::store", "ari_builtin_sync_atomic_i64_store"},
        {"sync::swap", "ari_builtin_sync_atomic_i64_swap"},
        {"std::sync::swap", "ari_builtin_sync_atomic_i64_swap"},
        {"sync::fetch_add", "ari_builtin_sync_atomic_i64_fetch_add"},
        {"std::sync::fetch_add", "ari_builtin_sync_atomic_i64_fetch_add"},
        {"sync::compare_exchange", "ari_builtin_sync_atomic_i64_compare_exchange"},
        {"std::sync::compare_exchange", "ari_builtin_sync_atomic_i64_compare_exchange"},
        {"time::monotonic_nanos", "ari_builtin_time_monotonic_nanos"},
        {"std::time::monotonic_nanos", "ari_builtin_time_monotonic_nanos"},
        {"time::unix_nanos", "ari_builtin_time_unix_nanos"},
        {"std::time::unix_nanos", "ari_builtin_time_unix_nanos"},
        {"time::sleep_nanos", "ari_builtin_time_sleep_nanos"},
        {"std::time::sleep_nanos", "ari_builtin_time_sleep_nanos"},
        {"random::entropy", "ari_builtin_random_entropy"},
        {"std::random::entropy", "ari_builtin_random_entropy"},
        {"os::close_raw", "ari_builtin_os_close"},
        {"std::os::close_raw", "ari_builtin_os_close"},
        {"os::duplicate_raw", "ari_builtin_os_dup"},
        {"std::os::duplicate_raw", "ari_builtin_os_dup"},
        {"os::close_on_exec_raw", "ari_builtin_os_close_on_exec"},
        {"std::os::close_on_exec_raw", "ari_builtin_os_close_on_exec"},
        {"os::set_close_on_exec_raw", "ari_builtin_os_set_close_on_exec"},
        {"std::os::set_close_on_exec_raw", "ari_builtin_os_set_close_on_exec"},
        {"os::nonblocking_raw", "ari_builtin_os_nonblocking"},
        {"std::os::nonblocking_raw", "ari_builtin_os_nonblocking"},
        {"os::set_nonblocking_raw", "ari_builtin_os_set_nonblocking"},
        {"std::os::set_nonblocking_raw", "ari_builtin_os_set_nonblocking"},
        {"os::pipe_raw", "ari_builtin_os_pipe"},
        {"std::os::pipe_raw", "ari_builtin_os_pipe"},
        {"fs::exists", "ari_builtin_fs_exists"},
        {"std::fs::exists", "ari_builtin_fs_exists"},
        {"fs::can_read", "ari_builtin_fs_can_read"},
        {"std::fs::can_read", "ari_builtin_fs_can_read"},
        {"fs::can_write", "ari_builtin_fs_can_write"},
        {"std::fs::can_write", "ari_builtin_fs_can_write"},
        {"fs::can_execute", "ari_builtin_fs_can_execute"},
        {"std::fs::can_execute", "ari_builtin_fs_can_execute"},
        {"fs::remove", "ari_builtin_fs_remove"},
        {"std::fs::remove", "ari_builtin_fs_remove"},
        {"fs::rename", "ari_builtin_fs_rename"},
        {"std::fs::rename", "ari_builtin_fs_rename"},
        {"fs::hard_link", "ari_builtin_fs_hard_link"},
        {"std::fs::hard_link", "ari_builtin_fs_hard_link"},
        {"fs::symbolic_link", "ari_builtin_fs_symbolic_link"},
        {"std::fs::symbolic_link", "ari_builtin_fs_symbolic_link"},
        {"fs::create_dir", "ari_builtin_fs_create_dir"},
        {"std::fs::create_dir", "ari_builtin_fs_create_dir"},
        {"fs::create_dir_all", "ari_builtin_fs_create_dir_all"},
        {"std::fs::create_dir_all", "ari_builtin_fs_create_dir_all"},
        {"fs::remove_dir", "ari_builtin_fs_remove_dir"},
        {"std::fs::remove_dir", "ari_builtin_fs_remove_dir"},
        {"fs::open_dir", "ari_builtin_fs_open_dir"},
        {"std::fs::open_dir", "ari_builtin_fs_open_dir"},
        {"fs::close_dir", "ari_builtin_fs_close_dir"},
        {"std::fs::close_dir", "ari_builtin_fs_close_dir"},
        {"fs::open", "ari_builtin_fs_open"},
        {"std::fs::open", "ari_builtin_fs_open"},
        {"fs::open_read", "ari_builtin_fs_open_read"},
        {"std::fs::open_read", "ari_builtin_fs_open_read"},
        {"fs::open_write", "ari_builtin_fs_open_write"},
        {"std::fs::open_write", "ari_builtin_fs_open_write"},
        {"fs::open_append", "ari_builtin_fs_open_append"},
        {"std::fs::open_append", "ari_builtin_fs_open_append"},
        {"fs::close", "ari_builtin_fs_close"},
        {"std::fs::close", "ari_builtin_fs_close"},
        {"fs::read_byte", "ari_builtin_fs_read_byte"},
        {"std::fs::read_byte", "ari_builtin_fs_read_byte"},
        {"fs::write_byte", "ari_builtin_fs_write_byte"},
        {"std::fs::write_byte", "ari_builtin_fs_write_byte"},
        {"fs::position", "ari_builtin_fs_position"},
        {"std::fs::position", "ari_builtin_fs_position"},
        {"fs::seek", "ari_builtin_fs_seek"},
        {"std::fs::seek", "ari_builtin_fs_seek"},
        {"mem::copy_bytes", "ari_builtin_mem_copy_bytes"},
        {"std::mem::copy_bytes", "ari_builtin_mem_copy_bytes"},
        {"mem::move_bytes", "ari_builtin_mem_move_bytes"},
        {"std::mem::move_bytes", "ari_builtin_mem_move_bytes"},
        {"mem::set_bytes", "ari_builtin_mem_set_bytes"},
        {"std::mem::set_bytes", "ari_builtin_mem_set_bytes"},
        {"mem::page_size", "ari_builtin_mem_page_size"},
        {"std::mem::page_size", "ari_builtin_mem_page_size"},
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
        {"zone::alloc_handle", "ari_builtin_zone_alloc_handle"},
        {"std::zone::alloc_handle", "ari_builtin_zone_alloc_handle"},
        {"zone::zone_handle", "ari_builtin_zone_handle"},
        {"std::zone::zone_handle", "ari_builtin_zone_handle"},
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
    const AriBuiltinTypeExpectation thread_entry = builtin_type("fn() -> i64");
    const AriBuiltinTypeExpectation thread_handle = builtin_type("std::thread::Thread");
    const AriBuiltinTypeExpectation target_arch = builtin_type("std::target::Arch");
    const AriBuiltinTypeExpectation target_os = builtin_type("std::target::Os");
    const AriBuiltinTypeExpectation target_env = builtin_type("std::target::Env");
    const AriBuiltinTypeExpectation target_object_format = builtin_type("std::target::ObjectFormat");
    const AriBuiltinTypeExpectation target_debug_format = builtin_type("std::target::DebugFormat");
    const AriBuiltinTypeExpectation target_errno_abi = builtin_type("std::target::ErrnoAbi");
    const AriBuiltinTypeExpectation ref_atomic_i64 = builtin_type("ref std::sync::AtomicI64");
    const AriBuiltinTypeExpectation ref_mut_atomic_i64 = builtin_type("ref mut std::sync::AtomicI64");
    const AriBuiltinTypeExpectation ptr_u8 = builtin_type("ptr u8");
    const AriBuiltinTypeExpectation ptr_c_void = builtin_type("ptr c_void", {"ptr void"});
    const AriBuiltinTypeExpectation own_zone = builtin_type("own Zone");
    const AriBuiltinTypeExpectation ref_mut_zone = builtin_type("ref mut Zone");
    const AriBuiltinTypeExpectation raw_string = builtin_type("std::string::RawString");
    const AriBuiltinTypeExpectation std_string = builtin_type("std::string::String");
    const AriBuiltinTypeExpectation ref_std_string = builtin_type("ref std::string::String");
    const AriBuiltinTypeExpectation fs_file = builtin_type("std::fs::File");
    const AriBuiltinTypeExpectation fs_dir = builtin_type("std::fs::Dir");

    if (symbol == "ari_builtin_context_argc") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_context_arg") return builtin_sig({i64}, source_string);
    if (symbol == "ari_builtin_context_thread_id") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_context_cwd") return builtin_sig({}, source_string);
    if (symbol == "ari_builtin_context_executable_path") return builtin_sig({}, source_string);
    if (symbol == "ari_builtin_target_triple") return builtin_sig({}, source_string);
    if (symbol == "ari_builtin_target_arch") return builtin_sig({}, target_arch);
    if (symbol == "ari_builtin_target_arch_name") return builtin_sig({}, source_string);
    if (symbol == "ari_builtin_target_os") return builtin_sig({}, target_os);
    if (symbol == "ari_builtin_target_os_name") return builtin_sig({}, source_string);
    if (symbol == "ari_builtin_target_env") return builtin_sig({}, target_env);
    if (symbol == "ari_builtin_target_env_name") return builtin_sig({}, source_string);
    if (symbol == "ari_builtin_target_object_format") return builtin_sig({}, target_object_format);
    if (symbol == "ari_builtin_target_debug_format") return builtin_sig({}, target_debug_format);
    if (symbol == "ari_builtin_target_errno_abi") return builtin_sig({}, target_errno_abi);
    if (symbol == "ari_builtin_target_pointer_bits") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_target_long_bits") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_env_get") return builtin_sig({source_string}, source_string);
    if (symbol == "ari_builtin_env_has") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_env_set") return builtin_sig({source_string, source_string}, boolean);
    if (symbol == "ari_builtin_env_remove") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_env_current_dir") return builtin_sig({}, source_string);
    if (symbol == "ari_builtin_env_set_current_dir") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_env_executable_path") return builtin_sig({}, source_string);
    if (symbol == "ari_builtin_process_id") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_process_uid") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_process_gid") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_process_exit") return builtin_sig({i64}, void_type);
    if (symbol == "ari_builtin_process_abort") return builtin_sig({}, void_type);
    if (symbol == "ari_builtin_process_fork") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_process_wait") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_mem_page_size") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_thread_spawn") return builtin_sig({thread_entry}, thread_handle);
    if (symbol == "ari_builtin_thread_join") return builtin_sig({thread_handle}, i64);
    if (symbol == "ari_builtin_thread_yield") return builtin_sig({}, void_type);
    if (symbol == "ari_builtin_thread_available_parallelism") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_sync_atomic_i64_load") return builtin_sig({ref_atomic_i64}, i64);
    if (symbol == "ari_builtin_sync_atomic_i64_store") return builtin_sig({ref_mut_atomic_i64, i64}, void_type);
    if (symbol == "ari_builtin_sync_atomic_i64_swap") return builtin_sig({ref_mut_atomic_i64, i64}, i64);
    if (symbol == "ari_builtin_sync_atomic_i64_fetch_add") return builtin_sig({ref_mut_atomic_i64, i64}, i64);
    if (symbol == "ari_builtin_sync_atomic_i64_compare_exchange") return builtin_sig({ref_mut_atomic_i64, i64, i64}, boolean);
    if (symbol == "ari_builtin_time_monotonic_nanos") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_time_unix_nanos") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_time_sleep_nanos") return builtin_sig({i64}, void_type);
    if (symbol == "ari_builtin_random_entropy") return builtin_sig({}, u64);
    if (symbol == "ari_builtin_random_fill") return builtin_sig({ptr_u8, i64}, void_type);
    if (symbol == "ari_builtin_os_close") return builtin_sig({i64}, boolean);
    if (symbol == "ari_builtin_os_dup") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_os_close_on_exec") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_os_set_close_on_exec") return builtin_sig({i64, boolean}, boolean);
    if (symbol == "ari_builtin_os_nonblocking") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_os_set_nonblocking") return builtin_sig({i64, boolean}, boolean);
    if (symbol == "ari_builtin_os_pipe") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_os_read_byte") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_os_write_byte") return builtin_sig({i64, u8}, boolean);
    if (symbol == "ari_builtin_net_tcp_listen_v4") return builtin_sig({i64, i64, i64, i64, i64}, i64);
    if (symbol == "ari_builtin_net_tcp_connect_v4") return builtin_sig({i64, i64, i64, i64, i64}, i64);
    if (symbol == "ari_builtin_net_tcp_accept") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_net_local_port") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_net_local_addr_v4") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_net_udp_bind_v4") return builtin_sig({i64, i64, i64, i64, i64}, i64);
    if (symbol == "ari_builtin_net_udp_send_byte_to_v4") {
        return builtin_sig({i64, u8, i64, i64, i64, i64, i64}, boolean);
    }
    if (symbol == "ari_builtin_net_udp_recv_byte") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_net_set_read_timeout_millis") return builtin_sig({i64, i64}, boolean);
    if (symbol == "ari_builtin_net_set_write_timeout_millis") return builtin_sig({i64, i64}, boolean);
    if (symbol == "ari_builtin_net_shutdown") return builtin_sig({i64, i64}, boolean);
    if (symbol == "ari_builtin_net_unix_listen") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_net_unix_connect") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_net_lookup_v4") return builtin_sig({source_string, i64}, i64);
    if (symbol == "ari_builtin_fs_exists") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_fs_can_read") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_fs_can_write") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_fs_can_execute") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_fs_can_read_bytes") return builtin_sig({ptr_u8, i64}, boolean);
    if (symbol == "ari_builtin_fs_can_write_bytes") return builtin_sig({ptr_u8, i64}, boolean);
    if (symbol == "ari_builtin_fs_can_execute_bytes") return builtin_sig({ptr_u8, i64}, boolean);
    if (symbol == "ari_builtin_fs_remove") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_fs_rename") return builtin_sig({source_string, source_string}, boolean);
    if (symbol == "ari_builtin_fs_hard_link") return builtin_sig({source_string, source_string}, boolean);
    if (symbol == "ari_builtin_fs_symbolic_link") return builtin_sig({source_string, source_string}, boolean);
    if (symbol == "ari_builtin_fs_read_link") return builtin_sig({source_string}, source_string);
    if (symbol == "ari_builtin_fs_create_dir") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_fs_create_dir_all") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_fs_remove_dir") return builtin_sig({source_string}, boolean);
    if (symbol == "ari_builtin_fs_open_dir") return builtin_sig({source_string}, fs_dir);
    if (symbol == "ari_builtin_fs_remove_bytes") return builtin_sig({ptr_u8, i64}, boolean);
    if (symbol == "ari_builtin_fs_remove_dir_bytes") return builtin_sig({ptr_u8, i64}, boolean);
    if (symbol == "ari_builtin_fs_open_dir_bytes") return builtin_sig({ptr_u8, i64}, fs_dir);
    if (symbol == "ari_builtin_fs_close_dir") return builtin_sig({fs_dir}, boolean);
    if (symbol == "ari_builtin_fs_read_dir_next") return builtin_sig({fs_dir}, source_string);
    if (symbol == "ari_builtin_fs_open") return builtin_sig({source_string, source_string}, fs_file);
    if (symbol == "ari_builtin_fs_open_options") {
        return builtin_sig({source_string, boolean, boolean, boolean, boolean, boolean, boolean}, fs_file);
    }
    if (symbol == "ari_builtin_fs_open_read") return builtin_sig({source_string}, fs_file);
    if (symbol == "ari_builtin_fs_open_write") return builtin_sig({source_string}, fs_file);
    if (symbol == "ari_builtin_fs_open_append") return builtin_sig({source_string}, fs_file);
    if (symbol == "ari_builtin_fs_close") return builtin_sig({fs_file}, boolean);
    if (symbol == "ari_builtin_fs_read_byte") return builtin_sig({fs_file}, i64);
    if (symbol == "ari_builtin_fs_write_byte") return builtin_sig({fs_file, u8}, boolean);
    if (symbol == "ari_builtin_fs_position") return builtin_sig({fs_file}, i64);
    if (symbol == "ari_builtin_fs_seek") return builtin_sig({fs_file, i64}, boolean);
    if (symbol == "ari_builtin_fs_metadata_size") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_metadata_kind") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_metadata_mode") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_metadata_accessed_nanos") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_metadata_modified_nanos") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_metadata_changed_nanos") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_symlink_metadata_size") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_symlink_metadata_kind") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_symlink_metadata_accessed_nanos") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_symlink_metadata_modified_nanos") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_symlink_metadata_changed_nanos") return builtin_sig({source_string}, i64);
    if (symbol == "ari_builtin_fs_metadata_size_bytes") return builtin_sig({ptr_u8, i64}, i64);
    if (symbol == "ari_builtin_fs_metadata_kind_bytes") return builtin_sig({ptr_u8, i64}, i64);
    if (symbol == "ari_builtin_fs_metadata_accessed_nanos_bytes") return builtin_sig({ptr_u8, i64}, i64);
    if (symbol == "ari_builtin_fs_metadata_modified_nanos_bytes") return builtin_sig({ptr_u8, i64}, i64);
    if (symbol == "ari_builtin_fs_metadata_changed_nanos_bytes") return builtin_sig({ptr_u8, i64}, i64);
    if (symbol == "ari_builtin_fs_symlink_metadata_size_bytes") return builtin_sig({ptr_u8, i64}, i64);
    if (symbol == "ari_builtin_fs_symlink_metadata_kind_bytes") return builtin_sig({ptr_u8, i64}, i64);
    if (symbol == "ari_builtin_fs_symlink_metadata_accessed_nanos_bytes") return builtin_sig({ptr_u8, i64}, i64);
    if (symbol == "ari_builtin_fs_symlink_metadata_modified_nanos_bytes") return builtin_sig({ptr_u8, i64}, i64);
    if (symbol == "ari_builtin_fs_symlink_metadata_changed_nanos_bytes") return builtin_sig({ptr_u8, i64}, i64);
    if (symbol == "ari_builtin_fs_set_mode") return builtin_sig({source_string, i64}, boolean);
    if (symbol == "ari_builtin_fs_canonicalize") return builtin_sig({source_string}, source_string);
    if (symbol == "ari_builtin_mem_copy_bytes") return builtin_sig({ptr_u8, ptr_u8, i64}, void_type);
    if (symbol == "ari_builtin_mem_move_bytes") return builtin_sig({ptr_u8, ptr_u8, i64}, void_type);
    if (symbol == "ari_builtin_mem_set_bytes") return builtin_sig({ptr_u8, u8, i64}, void_type);
    if (symbol == "ari_builtin_write_i64") return builtin_sig({i64}, i64);
    if (symbol == "ari_builtin_write_u64") return builtin_sig({u64}, i64);
    if (symbol == "ari_builtin_write_bool") return builtin_sig({boolean}, i64);
    if (symbol == "ari_builtin_write_byte") return builtin_sig({u8}, i64);
    if (symbol == "ari_builtin_write_error_byte") return builtin_sig({u8}, i64);
    if (symbol == "ari_builtin_newline") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_read_byte") return builtin_sig({}, i64);
    if (symbol == "ari_builtin_read_line") return builtin_sig({}, source_string);
    if (symbol == "ari_builtin_read_line_owned") return builtin_sig({ref_mut_zone}, std_string);
    if (symbol == "ari_builtin_zone_create") return builtin_sig({i64}, own_zone);
    if (symbol == "ari_builtin_zone_alloc") return builtin_sig({ref_mut_zone, i64, i64}, ptr_u8);
    if (symbol == "ari_builtin_zone_alloc_handle") return builtin_sig({ptr_c_void, i64, i64}, ptr_u8);
    if (symbol == "ari_builtin_zone_handle") return builtin_sig({ref_mut_zone}, ptr_c_void);
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
