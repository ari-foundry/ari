#include "trait_semantics.hpp"

namespace ari {

std::string trait_application_display(const std::string& trait_name, const std::vector<IrType>& trait_args) {
    std::string out = trait_name;
    if (!trait_args.empty()) {
        out += "[";
        for (std::size_t i = 0; i < trait_args.size(); ++i) {
            if (i > 0) out += ", ";
            out += type_name(trait_args[i]);
        }
        out += "]";
    }
    return out;
}

std::string trait_method_display(const std::string& trait_name,
                                 const std::vector<IrType>& trait_args,
                                 const std::string& method_name) {
    return trait_application_display(trait_name, trait_args) + "::" + method_name;
}

std::string trait_impl_key(const std::string& trait_name,
                           const std::vector<IrType>& trait_args,
                           const IrType& self_type) {
    return trait_application_display(trait_name, trait_args) + " for " + type_name(self_type);
}

bool trait_method_param_is_self_receiver(const TypeRef& param) {
    return param.name == "Self" &&
           param.args.empty() &&
           param.array_size == 0 &&
           !param.is_dyn_object &&
           !param.nullable;
}

bool trait_method_has_self_receiver(const std::vector<TypeRef>& params) {
    return !params.empty() && trait_method_param_is_self_receiver(params.front());
}

bool trait_expected_type_can_select_associated_self(const IrType& expected) {
    return expected.qualifier == TypeQualifier::Value &&
           expected.primitive != IrPrimitiveKind::Unknown &&
           expected.primitive != IrPrimitiveKind::Void;
}

} // namespace ari
