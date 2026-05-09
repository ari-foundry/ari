#pragma once

#include "common.hpp"
#include "ir.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ari {

struct TryEnumCaseShape {
    std::string name;
    std::uint32_t tag = 0;
    std::vector<IrType> payloads;
    SourceLocation loc;
};

struct TryEnumShape {
    bool supported = false;
    std::string diagnostic;
    std::uint32_t success_tag = 0;
    IrType success_payload_type;
    std::uint32_t residual_tag = 0;
    std::vector<IrType> residual_payloads;
};

TryEnumShape analyze_try_enum_shape(
    const std::string& enum_name,
    const std::vector<TryEnumCaseShape>& cases,
    const std::string& operator_name = "postfix ?"
);

} // namespace ari
