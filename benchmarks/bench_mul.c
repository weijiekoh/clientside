#include <stdio.h>
#include "../c/simd.h"

int main() {
    uint32_t cost;
    cost = 1 << 30;

    uint64_t a;
    uint64_t b;

    a = 123;
    b = 456;

    for (int i = 0; i < cost; i ++) {
        a = a * b;
    }
}
