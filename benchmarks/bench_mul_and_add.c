#include <stdio.h>
#include "../c/simd.h"

int main() {
    uint32_t cost;
    cost = 1 << 25;

    double a, b, c;

    a = 1.0;
    b = 2.0;
    c = 3.0;

    double temp;
    for (int i = 0; i < cost; i ++) {
        temp = a * b;
        a = temp + c;
    }
}
