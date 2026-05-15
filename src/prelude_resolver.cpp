#include "prelude_resolver.hpp"

#include <initializer_list>
#include <string>

namespace ari {

namespace {

std::string unqualified_name(const std::string& name) {
    std::size_t split = name.rfind("::");
    if (split == std::string::npos) return name;
    return name.substr(split + 2);
}

bool is_name_in(const std::string& name, std::initializer_list<const char*> names) {
    for (const char* item : names) {
        if (name == item) return true;
    }
    return false;
}

} // namespace

bool is_format_print_name(const std::string& name) {
    return is_name_in(name, {
        "print",
        "println",
        "io::print",
        "io::println",
        "std::print",
        "std::println",
        "std::io::print",
        "std::io::println",
    });
}

bool is_println_name(const std::string& name) {
    return is_name_in(name, {
        "println",
        "io::println",
        "std::println",
        "std::io::println",
    });
}

bool is_planned_prelude_function_name(const std::string& name) {
    return is_prelude_range_function_name(name);
}

std::string planned_prelude_function_message(const std::string& name) {
    if (name == "range" || name == "iter::range" || name == "std::iter::range") {
        return "range(start, end) is a built-in range constructor";
    }
    if (name == "range_inclusive" ||
        name == "iter::range_inclusive" ||
        name == "std::iter::range_inclusive") {
        return "range_inclusive(start, end) is a built-in inclusive range constructor";
    }
    return "prelude function '" + name + "' is planned but not implemented yet";
}

bool is_prelude_range_function_name(const std::string& name) {
    return is_name_in(name, {
        "range",
        "iter::range",
        "std::range",
        "std::iter::range",
        "prelude::range",
        "prelude::iter::range",
        "range_inclusive",
        "iter::range_inclusive",
        "std::range_inclusive",
        "std::iter::range_inclusive",
        "prelude::range_inclusive",
        "prelude::iter::range_inclusive",
    });
}

bool is_prelude_inclusive_range_function_name(const std::string& name) {
    return is_name_in(name, {
        "range_inclusive",
        "iter::range_inclusive",
        "std::range_inclusive",
        "std::iter::range_inclusive",
        "prelude::range_inclusive",
        "prelude::iter::range_inclusive",
    });
}

bool is_prelude_vec_len_function_name(const std::string& name) {
    return is_name_in(name, {
        "len",
        "vec::len",
        "std::len",
        "std::vec::len",
        "prelude::len",
        "prelude::vec::len",
    });
}

bool is_prelude_pointer_offset_function_name(const std::string& name) {
    return is_name_in(name, {
        "ptr_offset",
        "mem::ptr_offset",
        "std::ptr_offset",
        "std::mem::ptr_offset",
        "prelude::ptr_offset",
        "prelude::mem::ptr_offset",
    });
}

bool is_prelude_pointer_add_function_name(const std::string& name) {
    return is_name_in(name, {
        "ptr_add",
        "mem::ptr_add",
        "std::ptr_add",
        "std::mem::ptr_add",
        "prelude::ptr_add",
        "prelude::mem::ptr_add",
    });
}

bool is_prelude_pointer_load_function_name(const std::string& name) {
    return is_name_in(name, {
        "ptr_load",
        "mem::ptr_load",
        "std::ptr_load",
        "std::mem::ptr_load",
        "prelude::ptr_load",
        "prelude::mem::ptr_load",
    });
}

bool is_prelude_pointer_store_function_name(const std::string& name) {
    return is_name_in(name, {
        "ptr_store",
        "mem::ptr_store",
        "std::ptr_store",
        "std::mem::ptr_store",
        "prelude::ptr_store",
        "prelude::mem::ptr_store",
    });
}

bool is_prelude_size_of_function_name(const std::string& name) {
    return is_name_in(name, {
        "size_of",
        "mem::size_of",
        "std::size_of",
        "std::mem::size_of",
        "prelude::size_of",
        "prelude::mem::size_of",
    });
}

bool is_prelude_align_of_function_name(const std::string& name) {
    return is_name_in(name, {
        "align_of",
        "mem::align_of",
        "std::align_of",
        "std::mem::align_of",
        "prelude::align_of",
        "prelude::mem::align_of",
    });
}

bool is_prelude_move_function_name(const std::string& name) {
    return is_name_in(name, {
        "move",
        "std::move",
        "prelude::move",
    });
}

bool is_prelude_take_function_name(const std::string& name) {
    return is_name_in(name, {
        "take",
        "std::take",
        "prelude::take",
    });
}

bool is_zone_alloc_function_name(const std::string& name) {
    return is_name_in(name, {
        "alloc",
        "zone::alloc",
        "std::alloc",
        "std::zone::alloc",
        "prelude::alloc",
        "prelude::zone::alloc",
    });
}

bool is_zone_new_function_name(const std::string& name) {
    return is_name_in(name, {
        "new",
        "zone::new",
        "std::new",
        "std::zone::new",
        "prelude::new",
        "prelude::zone::new",
    });
}

bool is_zone_promote_function_name(const std::string& name) {
    return is_name_in(name, {
        "promote",
        "zone::promote",
        "std::promote",
        "std::zone::promote",
        "prelude::promote",
        "prelude::zone::promote",
    });
}

bool is_zone_scratch_function_name(const std::string& name) {
    return is_name_in(name, {
        "zone::scratch",
        "std::zone::scratch",
        "prelude::zone::scratch",
    });
}

bool is_zone_temp_function_name(const std::string& name) {
    return is_name_in(name, {
        "temp",
        "zone::temp",
        "std::temp",
        "std::zone::temp",
        "prelude::temp",
        "prelude::zone::temp",
    });
}

bool is_source_declared_prelude_special_name(const std::string& name) {
    return is_prelude_pointer_offset_function_name(name) ||
           is_prelude_pointer_add_function_name(name) ||
           is_prelude_pointer_load_function_name(name) ||
           is_prelude_pointer_store_function_name(name) ||
           is_prelude_size_of_function_name(name) ||
           is_prelude_align_of_function_name(name) ||
           is_prelude_move_function_name(name) ||
           is_prelude_take_function_name(name) ||
           is_zone_alloc_function_name(name) ||
           is_zone_new_function_name(name) ||
           is_zone_promote_function_name(name) ||
           is_prelude_range_function_name(name);
}

bool planned_prelude_type_arity(const std::string& name, std::size_t& arity) {
    std::string base = unqualified_name(name);
    if (base == "Unique" || base == "Shared" || base == "Weak") {
        arity = 1;
        return true;
    }
    return false;
}

std::string planned_prelude_type_message(const std::string& name) {
    std::string base = unqualified_name(name);
    if (base == "Unique") {
        return "prelude type 'Unique' is reserved for smart-pointer policy; use Box[T] for today's explicit-zone handle";
    }
    if (base == "Shared") {
        return "prelude type 'Shared' is reserved for future reference-counted ownership";
    }
    if (base == "Weak") {
        return "prelude type 'Weak' is reserved for future non-owning shared-pointer handles";
    }
    return "prelude type '" + name + "' is planned but not implemented yet";
}

} // namespace ari
