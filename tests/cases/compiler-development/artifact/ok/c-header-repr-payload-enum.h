#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct WireValue WireValue;
typedef struct WirePair WirePair;
typedef struct WireSmall WireSmall;
typedef struct WireHolder WireHolder;

struct WireValue {
    int32_t tag;
    uint64_t payload0;
};

enum {
    WireValue_ValueEmpty = 0,
    WireValue_Number = 1,
};

struct WirePair {
    int32_t tag;
    uint64_t payload0;
    uint64_t payload1;
};

enum {
    WirePair_PairEmpty = 0,
    WirePair_Pair = 1,
};

struct WireSmall {
    int32_t tag;
    uint64_t payload0;
};

enum {
    WireSmall_SmallEmpty = 0,
    WireSmall_Small = 1,
};

struct WireHolder {
    WireValue value;
    WirePair* pair;
    WireSmall small;
};

#ifdef __cplusplus
extern "C" {
#endif

int64_t ari_wire_value_tag(WireValue arg0);
WireValue ari_make_wire_value(int64_t arg0);
int64_t ari_wire_pair_ptr(WirePair* arg0);
int64_t ari_wire_holder_ptr(WireHolder* arg0);
WireSmall ari_make_wire_small(int32_t arg0);

#ifdef __cplusplus
}
#endif
