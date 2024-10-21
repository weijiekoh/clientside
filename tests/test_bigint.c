#include "minunit.h"
#include "../c/bigint.h"


// Inline WASM using __asm__ in C code
int add(int a, int b) {
    int result;
    __asm__ (
        "local.get %1\n"   // Push 'a' onto the stack
        "local.get %2\n"   // Push 'b' onto the stack
        "i32.add\n"        // Perform the addition
        "local.set %0\n"   // Store result in 'result'
        : "=r" (result)    // Output operand
        : "r" (a), "r" (b) // Input operands
    );
    return result;
}

MU_TEST(test_asm) {
    int a = 5, b = 10;
    printf("Result of %d + %d = %d\n", a, b, add(a, b));
}


MU_TEST(test_bigint_rand) {
    BigInt256 rand0 = bigint_rand(0);
    BigInt256 rand1 = bigint_rand(1);
    BigInt256 rand2 = bigint_rand(0);

    char *hex_str0 = bigint_to_hex(&rand0);
    char *hex_str1 = bigint_to_hex(&rand1);
    char *hex_str2 = bigint_to_hex(&rand2);

    // A different seed should result in a different BigInt
    mu_check(strcmp(hex_str0, hex_str1) != 0);

    // The same seed should result in the same BigInt
    mu_check(strcmp(hex_str0, hex_str2) == 0);

    free(hex_str0);
    free(hex_str1);
    free(hex_str2);
}

MU_TEST(test_hex_to_bigint) {
    BigInt256 number;
    BigInt256 expected_number = {
        .v = {
            0x12345678ULL,
            0x9abcdef0ULL,
            0x0fedcba9ULL,
            0x87654321ULL,
            0x11223344ULL,
            0x55667788ULL,
            0x99aabbccULL,
            0xddeeff00ULL
        }
    };
    const char *hex_str = "ddeeff0099aabbcc5566778811223344876543210fedcba99abcdef012345678";

    int result = hex_to_bigint256(hex_str, &number);
    mu_check(result == 0);
    mu_check(bigint_eq(&number, &expected_number));
}

MU_TEST(test_bigint_to_hex) {
    BigInt256 number = {
        .v = {
            0x12345678ULL,
            0x9abcdef0ULL,
            0x0fedcba9ULL,
            0x87654321ULL,
            0x11223344ULL,
            0x55667788ULL,
            0x99aabbccULL,
            0xddeeff00ULL
        }
    };
    const char *expected_hex = "ddeeff0099aabbcc5566778811223344876543210fedcba99abcdef012345678";

    char *hex_str = bigint_to_hex(&number);
    if (hex_str) {
        mu_check(strcmp(hex_str, expected_hex) == 0);
        free(hex_str);
    } else {
        mu_fail("Memory allocation failed.\n");
    }
}

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

    a.v[0] = 5;
    a.v[1] = 1;

    b.v[0] = 5;
    b.v[1] = 1;

    bigint_sub(&c, &a, &b);
    mu_check(bigint_eq(&c, &zero));

    a = bigint_new();
    b = bigint_new();
    c = bigint_new();

    a.v[0] = 5;
    a.v[1] = 1;

    b.v[0] = 4;
    b.v[1] = 1;

    expected.v[0] = 1;
    expected.v[1] = 0;

    bigint_sub(&c, &a, &b);
    mu_check(bigint_eq(&c, &expected));

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

int do_test_bigint_sub(
    const char *lhs_hex,
    const char *rhs_hex,
    const char *expected
) {
    BigInt256 lhs, rhs, res;
    hex_to_bigint256(lhs_hex, &lhs);
    hex_to_bigint256(rhs_hex, &rhs);

    bigint_sub(&res, &lhs, &rhs);

    char *res_hex = bigint_to_hex(&res);

    int result = strcmp(res_hex, expected);
    free(res_hex);
    return result;
}

MU_TEST(test_bigint_sub_2) {
    BigInt256 lhs, rhs, res;
    const char *lhs_hex  = "0000000000000000000000000000000000000000000000000000000000000002";
    const char *rhs_hex  = "0000000000000000000000000000000000000000000000000000000000000001";
    const char *expected = "0000000000000000000000000000000000000000000000000000000000000001";
    mu_check(do_test_bigint_sub(lhs_hex, rhs_hex, expected) == 0);
}

MU_TEST(test_bigint_sub_3) {
    BigInt256 lhs, rhs, res;
    const char *lhs_hex  = "ddddddddddd1ddddddd3333333333dddddddddddddddddd88ddddddddddd1234";
    const char *rhs_hex  = "aeeeeeeeeeeeeeeeeeeeeeeeee222222eeeeeeeeeeeeeeee9eeee0eeeeeeabcd";
    const char *expected = "2eeeeeeeeee2eeeeeee4444445111bbaeeeeeeeeeeeeeee9eeeefceeeeee6667";
    mu_check(do_test_bigint_sub(lhs_hex, rhs_hex, expected) == 0);
}

MU_TEST_SUITE(test_suite) {
    MU_RUN_TEST(test_asm);
	MU_RUN_TEST(test_bigint_eq);
	MU_RUN_TEST(test_bigint_gt);
	MU_RUN_TEST(test_bigint_sub);
	MU_RUN_TEST(test_bigint_sub_2);
	MU_RUN_TEST(test_bigint_sub_3);
	MU_RUN_TEST(test_bigint_to_hex);
	MU_RUN_TEST(test_hex_to_bigint);
	MU_RUN_TEST(test_bigint_rand);
}

int main(int argc, char *argv[]) {
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return MU_EXIT_CODE;
}
