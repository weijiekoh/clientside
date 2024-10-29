#include <stdio.h>
#include "../c/simd.h"

int main() {
    uint32_t cost;
    cost = 1 << 25;

    double a0, a1;
    double b0, b1;
    double c0, c1;

    a0 = 1.0;
    b0 = 2.0;
    c0 = 3.0;

    a1 = 0.0;
    b1 = 0.0;
    c1 = 0.0;

    f128 a, b, c, res;
    a = f64x2_make(a0, a1);
    b = f64x2_make(b0, b1);
    c = f64x2_make(c0, c1);

    for (int i = 0; i < cost; i ++) {
        a = f64x2_fma(a, b, c);
    }
}
