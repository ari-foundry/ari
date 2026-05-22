#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int64_t ApiStatus;
enum {
    ApiStatus_Ready = 0,
    ApiStatus_Busy = 1,
    ApiStatus_Done = 2,
};

typedef int64_t WireStatus;
enum {
    WireStatus_Offline = 0,
    WireStatus_Online = 1,
};

typedef struct ApiSlots ApiSlots;
typedef struct ApiPair ApiPair;
typedef struct GenericHandle GenericHandle;
typedef struct GenericHandle_i64 GenericHandle_i64;

struct ApiSlots {
    int64_t value;
    bool flag;
    ApiStatus status;
    void* raw;
    const int64_t* shared;
    int64_t* unique;
};

struct ApiPair {
    int64_t left;
    int64_t right;
};

struct GenericHandle_i64 {
    int64_t* raw;
};

#ifdef __cplusplus
extern "C" {
#endif

int64_t ari_ping(int64_t arg0);
int64_t ari_observe(ApiSlots* arg0);
int64_t ari_observe_ref(const int64_t* arg0);
int64_t ari_observe_generic(GenericHandle* arg0);
int64_t ari_take_generic(GenericHandle_i64 arg0);
int64_t ari_sum_pair(ApiPair arg0);
ApiPair ari_make_pair(int64_t arg0);
int64_t ari_status_code(ApiStatus arg0);
int64_t ari_wire_status_code(WireStatus arg0);

#ifdef __cplusplus
}
#endif
