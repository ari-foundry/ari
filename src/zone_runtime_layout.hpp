#pragma once

#include <cstdint>
#include <limits>

namespace ari {

inline constexpr std::int64_t kZoneAllocationHeaderBytes = 8;
inline constexpr std::int64_t kZoneAllocationHeaderZoneOffset = -8;
inline constexpr std::int64_t kZoneRuntimeMinimumPayloadAlign = 8;
inline constexpr std::int64_t kZoneRuntimeArenaReserveScale = 64;
inline constexpr std::int64_t kZoneRuntimeArenaReserveSlack = 64;
inline constexpr std::int64_t kZoneRuntimeZoneStructBytes = 40;
inline constexpr std::int64_t kZoneRuntimeMaxCreateCapacity =
    (std::numeric_limits<std::int64_t>::max() - kZoneRuntimeArenaReserveSlack) /
    kZoneRuntimeArenaReserveScale;

} // namespace ari
