#include "minunit.h"
#include "../c/simd.h"

MU_TEST(test_fma) {
    double a0, a1;
    double b0, b1;
    double c0, c1;
    double exp0, exp1;
    double res0, res1;

    a0 = 1.0;
    b0 = 2.0;
    c0 = 3.0;
    exp0 = a0 * b0 + c0;

    a1 = 4.0;
    b1 = 5.0;
    c1 = 6.0;
    exp1 = a1 * b1 + c1;

    f128 a, b, c, res;
    a = f64x2_make(a0, a1);
    b = f64x2_make(b0, b1);
    c = f64x2_make(c0, c1);
    res = f64x2_fma(a, b, c);

    res0 = f64x2_extract_l(res);
    res1 = f64x2_extract_h(res);

    mu_check(res0 == exp0);
    mu_check(res1 == exp1);
}

MU_TEST_SUITE(test_suite) {
	MU_RUN_TEST(test_fma);
}

int main(int argc, char *argv[]) {
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return MU_EXIT_CODE;
}
