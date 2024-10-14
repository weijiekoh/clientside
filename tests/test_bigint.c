#include "minunit.h"
#include "../c/bigint.h"

MU_TEST(test_bigint_eq) {
    BigInt256 a, b;
    a = bigint_new();
    b = bigint_new();

    mu_check(bigint_eq(&a, &b));

    a.v[0] = 5;
    a.v[1] = 1;

    b.v[0] = 5;
    b.v[1] = 1;

    mu_check(bigint_eq(&a, &b));
}

MU_TEST(test_bigint_gt) {
    BigInt256 a, b;
    a = bigint_new();
    b = bigint_new();

    a.v[0] = 5;
    a.v[1] = 1;

    b.v[0] = 5;
    b.v[1] = 1;

    mu_check(!bigint_gt(&a, &b));
}

MU_TEST(test_bigint_sub) {
    BigInt256 a, b, c, zero, expected;

    a = bigint_new();
    b = bigint_new();
    c = bigint_new();
    zero = bigint_new();
    expected = bigint_new();

    /*a.v[0] = 5;*/
    /*a.v[1] = 1;*/

    /*b.v[0] = 5;*/
    /*b.v[1] = 1;*/

    /*bigint_sub(&c, &a, &b);*/
    /*mu_check(bigint_eq(&c, &zero));*/

    /*a = bigint_new();*/
    /*b = bigint_new();*/
    /*c = bigint_new();*/

    /*a.v[0] = 5;*/
    /*a.v[1] = 1;*/

    /*b.v[0] = 4;*/
    /*b.v[1] = 1;*/

    /*expected.v[0] = 1;*/
    /*expected.v[1] = 0;*/

    /*bigint_sub(&c, &a, &b);*/
    /*mu_check(bigint_eq(&c, &expected));*/

    a = bigint_new();
    b = bigint_new();
    c = bigint_new();
    expected = bigint_new();

    a.v[0] = 5;
    a.v[1] = 1;

    b.v[0] = 6;
    b.v[1] = 0;

    expected.v[0] = 4294967295;
    expected.v[1] = 0;

    bigint_sub(&c, &a, &b);
    mu_check(bigint_eq(&c, &expected));
}

MU_TEST_SUITE(test_suite) {
	MU_RUN_TEST(test_bigint_eq);
	MU_RUN_TEST(test_bigint_gt);
	MU_RUN_TEST(test_bigint_sub);
}

int main(int argc, char *argv[]) {
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return MU_EXIT_CODE;
}
