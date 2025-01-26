#include "minunit.h"
#include "./test_mont_data.c"
#include "../c/mont.h"

const size_t NUM_TESTS = 1024;

char** get_mont_test_data();
char** get_mont_f64_test_data();

MU_TEST(test_mont_mul_9x29) {
    char** hex_strs = get_mont_9x29_test_data();
    char* p_hex = "12ab655e9a2ca55660b44d1e5c37b00159aa76fed00000010a11800000000001";
    uint64_t mu = 536870911;
    BigInt261 ar, br, p;
    int result;
    for (int i = 0; i < NUM_TESTS; i++) {
        char* ar_hex = hex_strs[i * 3];
        char* br_hex = hex_strs[i * 3 + 1];
        char* abr_hex = hex_strs[i * 3 + 2];

        result = hex_to_bigint261(p_hex, &p);
        mu_check(result == 0);
        result = hex_to_bigint261(ar_hex, &ar);
        mu_check(result == 0);
        result = hex_to_bigint261(br_hex, &br);
        mu_check(result == 0);

        BigInt261 res = mont_mul_9x29(&ar, &br, &p, mu);

        char* result_hex = bigint261_to_hex(&res);
        mu_check(strcmp(result_hex, abr_hex) == 0);
        free(result_hex);
    }
}

MU_TEST(test_mont_mul_9x30) {
    char** hex_strs = get_mont_9x30_test_data();
    char* p_hex = "12ab655e9a2ca55660b44d1e5c37b00159aa76fed00000010a11800000000001";
    // r = 0x40000000000000000000000000000000000000000000000000000000000000000000
    // rinv = 0x5d5191e6b7dac88d437608c7367240d24b96c56a35f53fbf4f22bff74686fbc
    uint64_t mu = 1073741823;

    // a = 0x0b626d61fa9249f1cdb1ed842fb0ce3683f172e5127d698fdcb3c98cba5a3dcb
    // b = 0x060746d8f3aa110102f1a1ab3d42df987110b2c030400f4c16da68ed2578bf10
    // abr = 0x0343242b5efb74206e6adbd66423a34c148866255f015136015dcf703f3baa19
    /*char* ar_hex = "04f3f6ad8ca41e576176363a623dda6b4f56d067ad20d29809280af92778432e";*/
    /*char* br_hex = "0d4494360069cc2ca61956c61bb03245639ff673a07bd30f4e62256761e02f39";*/
    /*char* abr_hex = "0343242b5efb74206e6adbd66423a34c148866255f015136015dcf703f3baa19";*/
    BigInt270 ar, br, p;
    int result;
    for (int i = 0; i < NUM_TESTS; i++) {
        char* ar_hex = hex_strs[i * 3];
        char* br_hex = hex_strs[i * 3 + 1];
        char* abr_hex = hex_strs[i * 3 + 2];

        result = hex_to_bigint270(p_hex, &p);
        mu_check(result == 0);
        result = hex_to_bigint270(ar_hex, &ar);
        mu_check(result == 0);
        result = hex_to_bigint270(br_hex, &br);
        mu_check(result == 0);

        BigInt270 res = mont_mul_9x30(&ar, &br, &p, mu);

        char* result_hex = bigint270_to_hex(&res);
        mu_check(strcmp(result_hex, abr_hex) == 0);
        free(result_hex);
    }
}

MU_TEST(test_mont_mul_cios_f64_simd) {
    char** hex_strs = get_mont_f64_test_data();
    char* p_hex = "12ab655e9a2ca55660b44d1e5c37b00159aa76fed00000010a11800000000001";
    uint64_t n0 = 422212465065983;

    BigIntF255 p, ar, br, abr, expected;

    ar = bigintf_new();
    br = bigintf_new();
    abr = bigintf_new();
    expected = bigintf_new();

    int result;

    // Convert p_hex, ar_hex, and br_hex to BigIntF255s
    result = hex_to_bigintf255(p_hex, &p);
    mu_check(result == 0);

    BigIntF255 p_for_redc = bigintf_new();
    uint64_t p0 = 0x1800000000001;
    uint64_t p1 = 0x7DA0000002142;
    uint64_t p2 = 0x0DEC00566A9DB;
    uint64_t p3 = 0x2AB305A268F2E;
    uint64_t p4 = 0x12AB655E9A2CA;
    memcpy(&(p_for_redc.v[0]), &p0, sizeof(uint64_t));
    memcpy(&(p_for_redc.v[1]), &p1, sizeof(uint64_t));
    memcpy(&(p_for_redc.v[2]), &p2, sizeof(uint64_t));
    memcpy(&(p_for_redc.v[3]), &p3, sizeof(uint64_t));
    memcpy(&(p_for_redc.v[4]), &p4, sizeof(uint64_t));

    for (int i = 0; i < NUM_TESTS; i++) {
        char* ar_hex = hex_strs[i * 3];
        char* br_hex = hex_strs[i * 3 + 1];
        char* abr_hex = hex_strs[i * 3 + 2];

        result = hex_to_bigintf255(ar_hex, &ar);
        mu_check(result == 0);

        result = hex_to_bigintf255(br_hex, &br);
        mu_check(result == 0);

        // Perform mont mul
        abr = mont_mul_cios_f64_simd(&ar, &br, &p, n0);

        abr = reduce_bigintf(&abr, &p_for_redc);
        abr = resolve_bigintf(&abr);

        uint64_t abrj = 0;
        for (int j = 0; j < 5; j++) {
            memcpy(&abrj, &(abr.v[j]), sizeof(uint64_t));
        }

        char* result_hex = malloc(65 * sizeof(char));
        bigintf255_to_hex(&abr, result_hex);
        mu_check(strcmp(result_hex, abr_hex) == 0);
        free(result_hex);
    }
}

typedef BigInt256 (*MontMulFunc)(BigInt256 *, BigInt256 *, BigInt256 *, uint64_t);
void do_mont_mul_test(
    MontMulFunc func_ptr,
    uint64_t n0 // aka mu
) {
    char* p_hex = "12ab655e9a2ca55660b44d1e5c37b00159aa76fed00000010a11800000000001";

    char** hex_strs = get_mont_test_data();
    
    BigInt256 p, ar, br, abr, expected;

    // Convert p_hex to a Bigint256
    int result;
    result = hex_to_bigint256(p_hex, &p);
    mu_check(result == 0);

    for (int i = 0; i < NUM_TESTS; i++) {
        char* ar_hex = hex_strs[i * 3];
        char* br_hex = hex_strs[i * 3 + 1];
        char* c_hex = hex_strs[i * 3 + 2];

        result = hex_to_bigint256(ar_hex, &ar);
        mu_check(result == 0);
        result = hex_to_bigint256(br_hex, &br);
        mu_check(result == 0);
        result = hex_to_bigint256(c_hex, &expected);
        mu_check(result == 0);

        // Perform mont mul
        abr = func_ptr(&ar, &br, &p, n0);

        char *abr_hex = bigint_to_hex(&abr);

        /*printf("%s\n", ar_hex);*/
        /*printf("%s\n", br_hex);*/
        /*printf("%s\n", c_hex);*/
        /*printf("%s\n", abr_hex);*/
        mu_check(strcmp(abr_hex, c_hex) == 0);
    }
}

MU_TEST(test_mont_mul_cios) {
    uint64_t n0 = 4294967295;
    char* p_hex = "12ab655e9a2ca55660b44d1e5c37b00159aa76fed00000010a11800000000001";

    char** hex_strs = get_mont_test_data();
    
    BigInt256 p, ar, br, abr, expected;

    // Convert p_hex to a Bigint256
    int result;
    result = hex_to_bigint256(p_hex, &p);
    mu_check(result == 0);

    uint64_t p_wide[9] = {0};
    for (int i = 0; i < 8; i ++) {
        p_wide[i] = p.v[i];
    }

    for (int i = 0; i < NUM_TESTS; i++) {
        char* ar_hex = hex_strs[i * 3];
        char* br_hex = hex_strs[i * 3 + 1];
        char* c_hex = hex_strs[i * 3 + 2];

        result = hex_to_bigint256(ar_hex, &ar);
        mu_check(result == 0);
        result = hex_to_bigint256(br_hex, &br);
        mu_check(result == 0);
        result = hex_to_bigint256(c_hex, &expected);
        mu_check(result == 0);

        // Perform mont mul
        abr = mont_mul_cios(&ar, &br, &p, p_wide, n0);

        char *abr_hex = bigint_to_hex(&abr);

        /*printf("%s\n", ar_hex);*/
        /*printf("%s\n", br_hex);*/
        /*printf("%s\n", c_hex);*/
        /*printf("%s\n", abr_hex);*/
        mu_check(strcmp(abr_hex, c_hex) == 0);
    }
}

MU_TEST(test_bm17_simd_mont_mul) {
    uint64_t mu = 1;
    MontMulFunc func_ptr = bm17_simd_mont_mul;
    do_mont_mul_test(func_ptr, mu);
}

MU_TEST(test_bm17_non_simd_mont_mul) {
    uint64_t mu = 1;
    MontMulFunc func_ptr = bm17_non_simd_mont_mul;
    do_mont_mul_test(func_ptr, mu);
}

MU_TEST_SUITE(test_suite) {
    MU_RUN_TEST(test_bm17_simd_mont_mul);
    MU_RUN_TEST(test_bm17_non_simd_mont_mul);
    MU_RUN_TEST(test_mont_mul_cios);
    MU_RUN_TEST(test_mont_mul_cios_f64_simd);
    MU_RUN_TEST(test_mont_mul_9x30);
    MU_RUN_TEST(test_mont_mul_9x29);
}

int main(int argc, char *argv[]) {
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return MU_EXIT_CODE;
}
