#include <stdint.h>

struct AriTestPair {
    int64_t left;
    int64_t right;
};

int64_t ari_test_add_i64(int64_t left, int64_t right) {
    return left + right;
}

struct AriTestPair ari_test_make_pair(int64_t seed) {
    struct AriTestPair pair = {seed, seed + 4};
    return pair;
}

int64_t ari_test_sum_pair(struct AriTestPair pair) {
    return pair.left + pair.right;
}

int64_t ari_test_strlen(const char* text) {
    int64_t count = 0;
    while (text[count] != '\0') {
        ++count;
    }
    return count;
}

void ari_test_inc_i64(int64_t* value) {
    *value += 1;
}

int64_t* ari_test_identity_i64_ref(int64_t* value) {
    return value;
}

int64_t* ari_test_identity_i64_mut_ref(int64_t* value) {
    return value;
}

int64_t ari_test_is_null(const void* value) {
    return value == 0 ? 1 : 0;
}

int64_t ari_test_apply_i64(int64_t (*op)(int64_t), int64_t value) {
    return op(value);
}
