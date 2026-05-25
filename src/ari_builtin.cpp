#include "ari_builtin.hpp"

#include <initializer_list>
#include <unordered_map>
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
        {"thread::spawn_configured", "ari_builtin_thread_spawn_configured"},
        {"std::thread::spawn_configured", "ari_builtin_thread_spawn_configured"},
        {"thread::join", "ari_builtin_thread_join"},
        {"std::thread::join", "ari_builtin_thread_join"},
        {"thread::is_finished", "ari_builtin_thread_is_finished"},
        {"std::thread::is_finished", "ari_builtin_thread_is_finished"},
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

namespace {

const std::unordered_map<std::string, std::string>& ari_builtin_source_alias_map() {
    static const std::unordered_map<std::string, std::string> aliases = [] {
        std::unordered_map<std::string, std::string> map;
        for (const auto& alias : ari_builtin_source_aliases()) {
            map.emplace(alias.source_name, alias.symbol);
        }
        return map;
    }();
    return aliases;
}

} // namespace

namespace {

const std::unordered_map<std::string, AriBuiltinSignatureExpectation>& ari_builtin_signature_map() {
    static const std::unordered_map<std::string, AriBuiltinSignatureExpectation> signatures = [] {
        std::unordered_map<std::string, AriBuiltinSignatureExpectation> map;
        auto add = [&map](std::string symbol, AriBuiltinSignatureExpectation signature) {
            map.emplace(std::move(symbol), std::move(signature));
        };
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
        const AriBuiltinTypeExpectation ptr_i64 = builtin_type("ptr i64");
        const AriBuiltinTypeExpectation ptr_u16 = builtin_type("ptr u16");
        const AriBuiltinTypeExpectation ptr_c_void = builtin_type("ptr c_void", {"ptr void"});
        const AriBuiltinTypeExpectation own_zone = builtin_type("own Zone");
        const AriBuiltinTypeExpectation ref_mut_zone = builtin_type("ref mut Zone");
        const AriBuiltinTypeExpectation raw_string = builtin_type("std::string::RawString");
        const AriBuiltinTypeExpectation std_string = builtin_type("std::string::String");
        const AriBuiltinTypeExpectation ref_std_string = builtin_type("ref std::string::String");
        const AriBuiltinTypeExpectation fs_file = builtin_type("std::fs::File");
        const AriBuiltinTypeExpectation fs_dir = builtin_type("std::fs::Dir");

        add("ari_builtin_context_argc", builtin_sig({}, i64));
        add("ari_builtin_context_arg", builtin_sig({i64}, source_string));
        add("ari_builtin_context_thread_id", builtin_sig({}, i64));
        add("ari_builtin_context_cwd", builtin_sig({}, source_string));
        add("ari_builtin_context_executable_path", builtin_sig({}, source_string));
        add("ari_builtin_target_triple", builtin_sig({}, source_string));
        add("ari_builtin_target_arch", builtin_sig({}, target_arch));
        add("ari_builtin_target_arch_name", builtin_sig({}, source_string));
        add("ari_builtin_target_os", builtin_sig({}, target_os));
        add("ari_builtin_target_os_name", builtin_sig({}, source_string));
        add("ari_builtin_target_env", builtin_sig({}, target_env));
        add("ari_builtin_target_env_name", builtin_sig({}, source_string));
        add("ari_builtin_target_object_format", builtin_sig({}, target_object_format));
        add("ari_builtin_target_debug_format", builtin_sig({}, target_debug_format));
        add("ari_builtin_target_errno_abi", builtin_sig({}, target_errno_abi));
        add("ari_builtin_target_pointer_bits", builtin_sig({}, i64));
        add("ari_builtin_target_long_bits", builtin_sig({}, i64));
        add("ari_builtin_env_get", builtin_sig({source_string}, source_string));
        add("ari_builtin_env_has", builtin_sig({source_string}, boolean));
        add("ari_builtin_env_set", builtin_sig({source_string, source_string}, boolean));
        add("ari_builtin_env_remove", builtin_sig({source_string}, boolean));
        add("ari_builtin_env_current_dir", builtin_sig({}, source_string));
        add("ari_builtin_env_set_current_dir", builtin_sig({source_string}, boolean));
        add("ari_builtin_env_executable_path", builtin_sig({}, source_string));
        add("ari_builtin_process_id", builtin_sig({}, i64));
        add("ari_builtin_process_uid", builtin_sig({}, i64));
        add("ari_builtin_process_gid", builtin_sig({}, i64));
        add("ari_builtin_process_exit", builtin_sig({i64}, void_type));
        add("ari_builtin_process_abort", builtin_sig({}, void_type));
        add("ari_builtin_process_fork", builtin_sig({}, i64));
        add("ari_builtin_process_wait", builtin_sig({i64}, i64));
        add("ari_builtin_mem_page_size", builtin_sig({}, i64));
        add("ari_builtin_thread_spawn", builtin_sig({thread_entry}, thread_handle));
        add("ari_builtin_thread_spawn_configured", builtin_sig({thread_entry, source_string, i64}, thread_handle));
        add("ari_builtin_thread_join", builtin_sig({thread_handle}, i64));
        add("ari_builtin_thread_is_finished", builtin_sig({thread_handle}, boolean));
        add("ari_builtin_thread_yield", builtin_sig({}, void_type));
        add("ari_builtin_thread_available_parallelism", builtin_sig({}, i64));
        add("ari_builtin_sync_atomic_i64_load", builtin_sig({ref_atomic_i64}, i64));
        add("ari_builtin_sync_atomic_i64_store", builtin_sig({ref_mut_atomic_i64, i64}, void_type));
        add("ari_builtin_sync_atomic_i64_swap", builtin_sig({ref_mut_atomic_i64, i64}, i64));
        add("ari_builtin_sync_atomic_i64_fetch_add", builtin_sig({ref_mut_atomic_i64, i64}, i64));
        add("ari_builtin_sync_atomic_i64_compare_exchange", builtin_sig({ref_mut_atomic_i64, i64, i64}, boolean));
        add("ari_builtin_sync_atomic_i64_load_order", builtin_sig({ref_atomic_i64, i64}, i64));
        add("ari_builtin_sync_atomic_i64_store_order", builtin_sig({ref_mut_atomic_i64, i64, i64}, void_type));
        add("ari_builtin_sync_atomic_i64_swap_order", builtin_sig({ref_mut_atomic_i64, i64, i64}, i64));
        add("ari_builtin_sync_atomic_i64_fetch_add_order", builtin_sig({ref_mut_atomic_i64, i64, i64}, i64));
        add("ari_builtin_sync_atomic_i64_compare_exchange_order", builtin_sig({ref_mut_atomic_i64, i64, i64, i64, i64}, boolean));
        add("ari_builtin_time_monotonic_nanos", builtin_sig({}, i64));
        add("ari_builtin_time_unix_nanos", builtin_sig({}, i64));
        add("ari_builtin_time_sleep_nanos", builtin_sig({i64}, void_type));
        add("ari_builtin_random_entropy", builtin_sig({}, u64));
        add("ari_builtin_random_fill", builtin_sig({ptr_u8, i64}, void_type));
        add("ari_builtin_random_fill_result", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_os_close", builtin_sig({i64}, boolean));
        add("ari_builtin_os_dup", builtin_sig({i64}, i64));
        add("ari_builtin_os_close_on_exec", builtin_sig({i64}, i64));
        add("ari_builtin_os_set_close_on_exec", builtin_sig({i64, boolean}, boolean));
        add("ari_builtin_os_nonblocking", builtin_sig({i64}, i64));
        add("ari_builtin_os_set_nonblocking", builtin_sig({i64, boolean}, boolean));
        add("ari_builtin_os_pipe", builtin_sig({}, i64));
        add("ari_builtin_os_read_byte", builtin_sig({i64}, i64));
        add("ari_builtin_os_write_byte", builtin_sig({i64, u8}, boolean));
        add("ari_builtin_net_tcp_listen_v4", builtin_sig({i64, i64, i64, i64, i64}, i64));
        add("ari_builtin_net_tcp_connect_v4", builtin_sig({i64, i64, i64, i64, i64}, i64));
        add("ari_builtin_net_tcp_listen_v6", builtin_sig({i64, i64, i64, i64, i64, i64, i64, i64, i64}, i64));
        add("ari_builtin_net_tcp_connect_v6", builtin_sig({i64, i64, i64, i64, i64, i64, i64, i64, i64}, i64));
        add("ari_builtin_net_tcp_accept", builtin_sig({i64}, i64));
        add("ari_builtin_net_local_port", builtin_sig({i64}, i64));
        add("ari_builtin_net_local_addr_family", builtin_sig({i64}, i64));
        add("ari_builtin_net_peer_addr_family", builtin_sig({i64}, i64));
        add("ari_builtin_net_local_addr_v4", builtin_sig({i64}, i64));
        add("ari_builtin_net_peer_addr_v4", builtin_sig({i64}, i64));
        add("ari_builtin_net_local_addr_v6", builtin_sig({i64, ptr_u16}, i64));
        add("ari_builtin_net_peer_addr_v6", builtin_sig({i64, ptr_u16}, i64));
        add("ari_builtin_net_udp_bind_v4", builtin_sig({i64, i64, i64, i64, i64}, i64));
        add("ari_builtin_net_udp_bind_v6", builtin_sig({i64, i64, i64, i64, i64, i64, i64, i64, i64}, i64));
        add("ari_builtin_net_udp_send_byte_to_v4", builtin_sig({i64, u8, i64, i64, i64, i64, i64}, boolean));
        add("ari_builtin_net_udp_send_byte_to_v6", builtin_sig({i64, u8, i64, i64, i64, i64, i64, i64, i64, i64, i64}, boolean));
        add("ari_builtin_net_udp_send_to_v4", builtin_sig({i64, ptr_u8, i64, i64, i64, i64, i64, i64}, i64));
        add("ari_builtin_net_udp_send_to_v6", builtin_sig({i64, ptr_u8, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64}, i64));
        add("ari_builtin_net_udp_connect_v4", builtin_sig({i64, i64, i64, i64, i64, i64}, boolean));
        add("ari_builtin_net_udp_connect_v6", builtin_sig({i64, i64, i64, i64, i64, i64, i64, i64, i64, i64}, boolean));
        add("ari_builtin_net_udp_send", builtin_sig({i64, ptr_u8, i64}, i64));
        add("ari_builtin_net_udp_recv", builtin_sig({i64, ptr_u8, i64}, i64));
        add("ari_builtin_net_udp_recv_from", builtin_sig({i64, ptr_u8, i64, i64, ptr_i64, ptr_i64, ptr_u16}, i64));
        add("ari_builtin_net_udp_recv_byte", builtin_sig({i64}, i64));
        add("ari_builtin_net_set_read_timeout_millis", builtin_sig({i64, i64}, boolean));
        add("ari_builtin_net_set_write_timeout_millis", builtin_sig({i64, i64}, boolean));
        add("ari_builtin_net_reuse_addr", builtin_sig({i64}, i64));
        add("ari_builtin_net_set_reuse_addr", builtin_sig({i64, boolean}, boolean));
        add("ari_builtin_net_reuse_port", builtin_sig({i64}, i64));
        add("ari_builtin_net_set_reuse_port", builtin_sig({i64, boolean}, boolean));
        add("ari_builtin_net_keepalive", builtin_sig({i64}, i64));
        add("ari_builtin_net_set_keepalive", builtin_sig({i64, boolean}, boolean));
        add("ari_builtin_net_broadcast", builtin_sig({i64}, i64));
        add("ari_builtin_net_set_broadcast", builtin_sig({i64, boolean}, boolean));
        add("ari_builtin_net_send_buffer_size", builtin_sig({i64}, i64));
        add("ari_builtin_net_set_send_buffer_size", builtin_sig({i64, i64}, boolean));
        add("ari_builtin_net_recv_buffer_size", builtin_sig({i64}, i64));
        add("ari_builtin_net_set_recv_buffer_size", builtin_sig({i64, i64}, boolean));
        add("ari_builtin_net_nodelay", builtin_sig({i64}, i64));
        add("ari_builtin_net_set_nodelay", builtin_sig({i64, boolean}, boolean));
        add("ari_builtin_net_shutdown", builtin_sig({i64, i64}, boolean));
        add("ari_builtin_net_unix_listen", builtin_sig({source_string}, i64));
        add("ari_builtin_net_unix_connect", builtin_sig({source_string}, i64));
        add("ari_builtin_net_lookup_v4", builtin_sig({source_string, i64}, i64));
        add("ari_builtin_net_lookup_v4_endpoint", builtin_sig({source_string}, i64));
        add("ari_builtin_net_lookup_v6", builtin_sig({source_string, i64, ptr_u16}, i64));
        add("ari_builtin_net_lookup_v6_endpoint", builtin_sig({source_string, ptr_u16}, i64));
        add("ari_builtin_fs_exists", builtin_sig({source_string}, boolean));
        add("ari_builtin_fs_can_read", builtin_sig({source_string}, boolean));
        add("ari_builtin_fs_can_write", builtin_sig({source_string}, boolean));
        add("ari_builtin_fs_can_execute", builtin_sig({source_string}, boolean));
        add("ari_builtin_fs_can_read_bytes", builtin_sig({ptr_u8, i64}, boolean));
        add("ari_builtin_fs_can_write_bytes", builtin_sig({ptr_u8, i64}, boolean));
        add("ari_builtin_fs_can_execute_bytes", builtin_sig({ptr_u8, i64}, boolean));
        add("ari_builtin_fs_remove", builtin_sig({source_string}, boolean));
        add("ari_builtin_fs_rename", builtin_sig({source_string, source_string}, boolean));
        add("ari_builtin_fs_hard_link", builtin_sig({source_string, source_string}, boolean));
        add("ari_builtin_fs_symbolic_link", builtin_sig({source_string, source_string}, boolean));
        add("ari_builtin_fs_read_link", builtin_sig({source_string}, source_string));
        add("ari_builtin_fs_create_dir", builtin_sig({source_string}, boolean));
        add("ari_builtin_fs_create_dir_all", builtin_sig({source_string}, boolean));
        add("ari_builtin_fs_remove_dir", builtin_sig({source_string}, boolean));
        add("ari_builtin_fs_open_dir", builtin_sig({source_string}, fs_dir));
        add("ari_builtin_fs_remove_bytes", builtin_sig({ptr_u8, i64}, boolean));
        add("ari_builtin_fs_remove_dir_bytes", builtin_sig({ptr_u8, i64}, boolean));
        add("ari_builtin_fs_open_dir_bytes", builtin_sig({ptr_u8, i64}, fs_dir));
        add("ari_builtin_fs_close_dir", builtin_sig({fs_dir}, boolean));
        add("ari_builtin_fs_read_dir_next", builtin_sig({fs_dir}, source_string));
        add("ari_builtin_fs_open", builtin_sig({source_string, source_string}, fs_file));
        add("ari_builtin_fs_open_options", builtin_sig({source_string, boolean, boolean, boolean, boolean, boolean, boolean}, fs_file));
        add("ari_builtin_fs_open_read", builtin_sig({source_string}, fs_file));
        add("ari_builtin_fs_open_write", builtin_sig({source_string}, fs_file));
        add("ari_builtin_fs_open_append", builtin_sig({source_string}, fs_file));
        add("ari_builtin_fs_close", builtin_sig({fs_file}, boolean));
        add("ari_builtin_fs_read_byte", builtin_sig({fs_file}, i64));
        add("ari_builtin_fs_write_byte", builtin_sig({fs_file, u8}, boolean));
        add("ari_builtin_fs_position", builtin_sig({fs_file}, i64));
        add("ari_builtin_fs_seek", builtin_sig({fs_file, i64}, boolean));
        add("ari_builtin_fs_metadata_size", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_metadata_kind", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_metadata_mode", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_metadata_accessed_nanos", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_metadata_modified_nanos", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_metadata_changed_nanos", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_symlink_metadata_size", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_symlink_metadata_kind", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_symlink_metadata_accessed_nanos", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_symlink_metadata_modified_nanos", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_symlink_metadata_changed_nanos", builtin_sig({source_string}, i64));
        add("ari_builtin_fs_metadata_size_bytes", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_fs_metadata_kind_bytes", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_fs_metadata_accessed_nanos_bytes", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_fs_metadata_modified_nanos_bytes", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_fs_metadata_changed_nanos_bytes", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_fs_symlink_metadata_size_bytes", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_fs_symlink_metadata_kind_bytes", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_fs_symlink_metadata_accessed_nanos_bytes", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_fs_symlink_metadata_modified_nanos_bytes", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_fs_symlink_metadata_changed_nanos_bytes", builtin_sig({ptr_u8, i64}, i64));
        add("ari_builtin_fs_set_mode", builtin_sig({source_string, i64}, boolean));
        add("ari_builtin_fs_canonicalize", builtin_sig({source_string}, source_string));
        add("ari_builtin_mem_copy_bytes", builtin_sig({ptr_u8, ptr_u8, i64}, void_type));
        add("ari_builtin_mem_move_bytes", builtin_sig({ptr_u8, ptr_u8, i64}, void_type));
        add("ari_builtin_mem_set_bytes", builtin_sig({ptr_u8, u8, i64}, void_type));
        add("ari_builtin_write_i64", builtin_sig({i64}, i64));
        add("ari_builtin_write_u64", builtin_sig({u64}, i64));
        add("ari_builtin_write_bool", builtin_sig({boolean}, i64));
        add("ari_builtin_write_byte", builtin_sig({u8}, i64));
        add("ari_builtin_write_error_byte", builtin_sig({u8}, i64));
        add("ari_builtin_newline", builtin_sig({}, i64));
        add("ari_builtin_read_byte", builtin_sig({}, i64));
        add("ari_builtin_read_line", builtin_sig({}, source_string));
        add("ari_builtin_read_line_owned", builtin_sig({ref_mut_zone}, std_string));
        add("ari_builtin_zone_create", builtin_sig({i64}, own_zone));
        add("ari_builtin_zone_alloc", builtin_sig({ref_mut_zone, i64, i64}, ptr_u8));
        add("ari_builtin_zone_alloc_handle", builtin_sig({ptr_c_void, i64, i64}, ptr_u8));
        add("ari_builtin_zone_handle", builtin_sig({ref_mut_zone}, ptr_c_void));
        add("ari_builtin_zone_allocation_zone", builtin_sig({ptr_u8}, ptr_c_void));
        add("ari_builtin_string_alloc_buffer", builtin_sig({ref_mut_zone, i64}, ptr_u8));
        add("ari_builtin_string_with_capacity", builtin_sig({ref_mut_zone, i64}, raw_string));
        add("ari_builtin_string_new", builtin_sig({ref_mut_zone, i64}, std_string));
        add("ari_builtin_string_from_string", builtin_sig({ref_mut_zone, source_string}, std_string));
        add("ari_builtin_string_copy_to", builtin_sig({ref_std_string, ref_mut_zone}, std_string));
        add("ari_builtin_zone_reset", builtin_sig({ref_mut_zone}, void_type));
        add("ari_builtin_zone_destroy", builtin_sig({own_zone}, void_type));
        add("ari_builtin_assert", builtin_sig({boolean}, i64));
        add("ari_builtin_assert_eq_i64", builtin_sig({i64, i64}, i64));
        add("ari_builtin_assert_ne_i64", builtin_sig({i64, i64}, i64));
        add("ari_builtin_assert_eq_bool", builtin_sig({boolean, boolean}, i64));
        add("ari_builtin_assert_ne_bool", builtin_sig({boolean, boolean}, i64));
        add("ari_builtin_panic", builtin_sig({}, void_type));
        return map;
    }();
    return signatures;
}

} // namespace

std::optional<AriBuiltinSignatureExpectation> ari_builtin_signature_for_symbol(const std::string& symbol) {
    const auto& signatures = ari_builtin_signature_map();
    auto found = signatures.find(symbol);
    if (found == signatures.end()) return std::nullopt;
    return found->second;
}

bool is_ari_builtin_symbol(const std::string& symbol) {
    const auto& signatures = ari_builtin_signature_map();
    return signatures.find(symbol) != signatures.end();
}

std::optional<std::string> ari_builtin_symbol_for_source_name(const std::string& source_name) {
    const auto& aliases = ari_builtin_source_alias_map();
    auto found = aliases.find(source_name);
    if (found != aliases.end()) return found->second;
    return std::nullopt;
}

} // namespace ari
