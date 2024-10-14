#include <stdio.h>
#include "../c/simd.h"

int main() {
    uint32_t cost;
    cost = 1 << 30;

    uint64_t a0, a1;
    uint64_t b0, b1;

    a0 = 123;
    b0 = 456;

    a1 = 0;
    b1 = 0;

    i128 a, b, c, res;
    a = i64x2_make(a0, a1);
    b = i64x2_make(b0, b1);

    for (int i = 0; i < cost; i ++) {
        a = i64x2_mul(a, b);
    }
}

