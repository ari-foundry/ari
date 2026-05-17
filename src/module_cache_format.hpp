#pragma once

namespace ari {

inline constexpr int kModuleCacheFormatVersion = 0;
inline constexpr const char* kModuleMetadataHeader = "ari-module-metadata-v0";
inline constexpr const char* kModuleCacheHeader = "ari-module-cache-v0";
inline constexpr const char* kModuleAstDeclsPayloadHeader = "ari-ast-decls-v1;";
inline constexpr const char* kModuleIrSummaryPayloadHeader = "ari-ir-summary-v0;";

} // namespace ari
