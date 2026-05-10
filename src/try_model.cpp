#include "try_model.hpp"

#include <cstddef>

namespace ari {

namespace {

std::string basename_of_case(const std::string& name) {
    std::size_t split = name.rfind("::");
    if (split == std::string::npos) return name;
    return name.substr(split + 2);
}

bool is_success_case_name(const std::string& name) {
    std::string base = basename_of_case(name);
    return base == "Some" || base == "Ok" || base == "Success";
}

bool is_residual_case_name(const std::string& name) {
    std::string base = basename_of_case(name);
    return base == "None" || base == "Err" || base == "Error" || base == "Failure";
}

} // namespace

TryEnumShape analyze_try_enum_shape(
    const std::string& enum_name,
    const std::vector<TryEnumCaseShape>& cases,
    const std::string& operator_name
) {
    TryEnumShape shape;
    if (cases.size() != 2) {
        shape.diagnostic = operator_name + " expects a two-case Option/Result-style enum, got " + enum_name;
        return shape;
    }

    const TryEnumCaseShape* success = nullptr;
    const TryEnumCaseShape* residual = nullptr;
    for (const auto& item : cases) {
        if (is_success_case_name(item.name)) {
            if (success) {
                shape.diagnostic = operator_name + " found multiple success cases in enum '" + enum_name + "'";
                return shape;
            }
            success = &item;
        } else if (is_residual_case_name(item.name)) {
            if (residual) {
                shape.diagnostic = operator_name + " found multiple residual cases in enum '" + enum_name + "'";
                return shape;
            }
            residual = &item;
        }
    }

    if (!success || !residual) {
        shape.diagnostic =
            operator_name + " expects success case Some/Ok/Success and residual case None/Err/Error/Failure in enum '" +
            enum_name + "'";
        return shape;
    }
    if (success->payloads.size() != 1) {
        shape.diagnostic = operator_name + " success case '" + success->name + "' must have exactly one payload";
        return shape;
    }
    if (residual->payloads.size() > 1) {
        shape.diagnostic = operator_name + " residual case '" + residual->name + "' can carry at most one payload";
        return shape;
    }

    shape.supported = true;
    shape.success_tag = success->tag;
    shape.success_payload_type = success->payloads[0];
    shape.residual_tag = residual->tag;
    shape.residual_payloads = residual->payloads;
    return shape;
}

} // namespace ari
