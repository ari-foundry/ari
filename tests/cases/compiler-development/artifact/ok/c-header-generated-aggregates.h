#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct AriTuple_i64_i64 AriTuple_i64_i64;
typedef struct AriEnum_MaybeWide AriEnum_MaybeWide;
typedef struct AriArray_i64_1 AriArray_i64_1;
typedef struct AriVec_Vec_i64_1 AriVec_Vec_i64_1;

struct AriTuple_i64_i64 {
    int64_t field0;
    int64_t field1;
};

struct AriEnum_MaybeWide {
    int32_t tag;
    uint64_t payload0;
};

struct AriArray_i64_1 {
    int64_t elements[1];
};

struct AriVec_Vec_i64_1 {
    int64_t len;
    int64_t data[1];
};

#ifdef __cplusplus
extern "C" {
#endif

int64_t ari_take_tuple(AriTuple_i64_i64 arg0);
AriTuple_i64_i64 ari_make_tuple(int64_t arg0);
int64_t ari_tuple_ptr(AriTuple_i64_i64* arg0);
int64_t ari_take_maybe(AriEnum_MaybeWide arg0);
AriEnum_MaybeWide ari_make_maybe(int64_t arg0);
int64_t ari_force_vec(void);
AriVec_Vec_i64_1 _ARNv23echo__G_Vec_i64__cap_1_(AriVec_Vec_i64_1 arg0);

#ifdef __cplusplus
}
#endif
