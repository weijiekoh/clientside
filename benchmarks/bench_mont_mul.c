#include <stdio.h>
#include <assert.h>
#include <emscripten.h>
#include "../c/mont.h"

BigIntF255 reference_func_mont_mul_cios_f64_simd(
    BigIntF255 *a,
    BigIntF255 *b,
    BigIntF255 *p,
    BigIntF255 *p_for_redc,
    uint64_t n0,
    uint64_t cost
) {
    BigIntF255 x = *a;
    BigIntF255 y = *b;
    BigIntF255 z;

    uint64_t xx;

    for (uint64_t i = 0; i < cost; i ++) {
        BigIntF255 z = mont_mul_cios_f64_simd(&x, &y, p, n0);
        z = reduce_bigintf(&z, p_for_redc);
        z = resolve_bigintf(&z);
        x = y;
        y = z;
    }
    return y;
}

BigInt261 reference_func_mont_mul_9x29(
    BigInt261 *a,
    BigInt261 *b,
    BigInt261 *p,
    uint64_t n0,
    uint64_t cost
) {
    BigInt261 x = *a;
    BigInt261 y = *b;
    BigInt261 z;

    for (uint64_t i = 0; i < cost; i ++) {
        z = mont_mul_9x29(&x, &y, p, n0);
        x = y;
        y = z;
    }
    return y;
}

BigInt270 reference_func_mont_mul_9x30(
    BigInt270 *a,
    BigInt270 *b,
    BigInt270 *p,
    uint64_t n0,
    uint64_t cost
) {
    BigInt270 x = *a;
    BigInt270 y = *b;
    BigInt270 z;

    for (uint64_t i = 0; i < cost; i ++) {
        z = mont_mul_9x30(&x, &y, p, n0);
        x = y;
        y = z;
    }
    return y;
}

BigInt256 reference_func_mont_mul_cios(
    BigInt256 *a,
    BigInt256 *b,
    BigInt256 *p,
    uint64_t *p_for_redc,
    uint64_t mu,
    uint64_t cost
) {
    BigInt256 x = *a;
    BigInt256 y = *b;
    BigInt256 z;

    for (uint64_t i = 0; i < cost; i ++) {
        z = mont_mul_cios(&x, &y, p, p_for_redc, mu);
        x = y;
        y = z;
    }
    return y;
}

BigInt256 reference_func_bm17_non_simd(
    BigInt256 *a,
    BigInt256 *b,
    BigInt256 *p,
    uint64_t mu,
    uint64_t cost
) {
    BigInt256 x = *a;
    BigInt256 y = *b;
    BigInt256 z;

    for (uint64_t i = 0; i < cost; i ++) {
        z = bm17_non_simd_mont_mul(&x, &y, p, mu);
        x = y;
        y = z;
    }
    return y;
}

BigInt256 reference_func_bm17_simd(
    BigInt256 *a,
    BigInt256 *b,
    BigInt256 *p,
    uint64_t mu,
    uint64_t cost
) {
    BigInt256 x = *a;
    BigInt256 y = *b;
    BigInt256 z;

    for (uint64_t i = 0; i < cost; i ++) {
        z = bm17_simd_mont_mul(&x, &y, p, mu);
        x = y;
        y = z;
    }
    return y;
}

int main(int argc, char *argv[]) {
    uint64_t log_cost = 10;
    if (argc > 1) {
        char* endptr;
        log_cost = strtoull(argv[1], &endptr, 0);
        
        if (*endptr != '\0') {
            printf("Error: Invalid argument format; should be an integer.\n");
            exit(1);
        }
    }

    uint64_t cost = 1 << log_cost;

    // Only check the result against the hardcoded expected constants if cost == 1024
    bool do_assert = cost == 1 << 10;

    int result;
    BigInt256 p, ar, br, res;
    BigIntF255 p_f, ar_f, br_f, res_f;

    char* p_hex =  "12ab655e9a2ca55660b44d1e5c37b00159aa76fed00000010a11800000000001";
    char* ar_hex = "05552c9522974fd00772ef23d45519d4d3a486218b3a0d238a25560beda10ce9";
    char* br_hex = "094c0e4dc5769c3bcc4c984fa08b0ceaf437545d83d259471a983cf05e97f19b";
    // These results are hardcoded for cost == 1024
    char* expected_hex = "116a8d07bb676b153699f5744d2eace047a0646f4cfe06012a0bbd53720543b1";

    result = hex_to_bigint256(p_hex, &p);
    assert(result == 0);
    result = hex_to_bigint256(ar_hex, &ar);
    assert(result == 0);
    result = hex_to_bigint256(br_hex, &br);
    assert(result == 0);

    int num_runs = 5;
    double start_a, end_a;
    double start_b, end_b;
    double start_c, end_c;
    double start_d, end_d;
    double start_e, end_e;
    double start_f, end_f;

    double avg_a = 0;
    double avg_b = 0;
    double avg_c = 0;
    double avg_d = 0;
    double avg_e = 0;
    double avg_f = 0;

    // Benchmark bm17_non_simd_mont_mul
    for (int i = 0; i < num_runs; i ++) {
        start_a = emscripten_get_now();
        res = reference_func_bm17_non_simd(&ar, &br, &p, 1, cost);
        end_a = emscripten_get_now();
        avg_a += end_a - start_a;
        char* res_hex = bigint_to_hex(&res);
        if (do_assert)
            assert(strcmp(res_hex, expected_hex) == 0);
    }

    // Benchmark bm17_simd_mont_mul. It is slower in WASM because it uses SIMD opcodes that are not
    // natively executed by the CPU.
    for (int i = 0; i < num_runs; i ++) {
        start_b = emscripten_get_now();
        res = reference_func_bm17_simd(&ar, &br, &p, 1, cost);
        end_b = emscripten_get_now();
        avg_b += end_b - start_b;
        char* res_hex = bigint_to_hex(&res);
        if (do_assert)
            assert(strcmp(res_hex, expected_hex) == 0);
    }

    // Benchmark mont_mul_cios
    uint64_t p_wide[9] = {0};
    for (int i = 0; i < 8; i ++) {
        p_wide[i] = p.v[i];
    }

    for (int i = 0; i < num_runs; i ++) {
        start_c = emscripten_get_now();
        res = reference_func_mont_mul_cios(&ar, &br, &p, p_wide, 4294967295, cost);
        end_c = emscripten_get_now();
        avg_c += end_c - start_c;
        char* res_hex = bigint_to_hex(&res);
        if (do_assert)
            assert(strcmp(res_hex, expected_hex) == 0);
    }

    // Benchmark mont_mul_cios_f64_simd
    ar_hex = "0c0048f9de61fa9334139e21184664eb16a77e902d9d06924a1b6b05f6d08675";
    br_hex = "0dfbb9d62fd1a0c9168072b6fe615e7626f0e5ae29e92ca41254de782f4bf8ce";
    char* expected_for_cios_f64_hex = "120af9332aca0835cba7214954b32e70d0a56db70e7f03011a0e9ea9b902a1d9";

    // Benchmark mont_mul_cios_f64_simd
    result = hex_to_bigintf255(p_hex, &p_f);
    assert(result == 0);
    result = hex_to_bigintf255(ar_hex, &ar_f);
    assert(result == 0);
    result = hex_to_bigintf255(br_hex, &br_f);
    assert(result == 0);

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

    uint64_t n0 = 422212465065983;

    for (int i = 0; i < num_runs; i ++) {
        start_d = emscripten_get_now();
        res_f = reference_func_mont_mul_cios_f64_simd(&ar_f, &br_f, &p_f, &p_for_redc, n0, cost);
        end_d = emscripten_get_now();
        avg_d = end_d - start_d;
        char res_f_hex[65];
        bigintf255_to_hex(&res_f, res_f_hex);
        if (do_assert)
            assert(strcmp(res_f_hex, expected_for_cios_f64_hex) == 0);
    }

    // Benchmark mont_mul_9x30
    ar_hex = "107b8491edf2141b3c5b6f2dc8a34c3b46e37782d348cf8a4fa87b68433a2db9";
    br_hex = "11ab14a88c521c7574625aa40c9ff2066edb54cc3651a587358f17a5fc66a022";
    char* expected_for_9x30_hex = "0261dacd294624a0f0d0dab1fe80014ac3d36e2541800c75d286dc8150ec044c";

    BigInt270 ar_270, br_270, p_270, res_270;
    result = hex_to_bigint270(p_hex, &p_270);
    assert(result == 0);
    result = hex_to_bigint270(ar_hex, &ar_270);
    assert(result == 0);
    result = hex_to_bigint270(br_hex, &br_270);
    assert(result == 0);

    for (int i = 0; i < num_runs; i ++) {
        start_e = emscripten_get_now();
        res_270 = reference_func_mont_mul_9x30(&ar_270, &br_270, &p_270, 1073741823, cost);
        end_e = emscripten_get_now();
        avg_e = end_e - start_e;
        char* res_hex = bigint270_to_hex(&res_270);
        if (do_assert)
            assert(strcmp(res_hex, expected_for_9x30_hex) == 0);
    }

    // Benchmark mont_mul_9x29
    /*ar_hex = "029f0250e75829f788072e694cae0a8e4d92953c1741a467ea0d417db4219d17";*/
    /*br_hex = "1176d92da635d769df02852caa1e4d4a45ed92c24a4b28d3bc011e0bd2fe3351";*/
    /*char* expected_for_9x29 = "0fe7253ff5dea7ddded1f2193386abe1cbbd130c0fc0c0071d7c2a6e40a87603";*/
    ar_hex = "0d3a69657d06317d0736bc97a156f051155991dec7e65881b25d3858d9f7affe";
    br_hex = "02194df48190eb9fa29e870e0d2f6ab762a851c94383427d8f68d90d9cc0fcf4";
    char* expected_for_9x29 = "076d42656adcf183ebe06a2fb8840c71b1018e61f7d62192d487edf04c2dcfdd";

    BigInt261 ar_261, br_261, p_261, res_261;
    result = hex_to_bigint261(p_hex, &p_261);
    assert(result == 0);
    result = hex_to_bigint261(ar_hex, &ar_261);
    assert(result == 0);
    result = hex_to_bigint261(br_hex, &br_261);
    assert(result == 0);

    for (int i = 0; i < num_runs; i ++) {
        start_f = emscripten_get_now();
        res_261 = reference_func_mont_mul_9x29(&ar_261, &br_261, &p_261, 536870911, cost);
        end_f = emscripten_get_now();
        avg_f = end_f - start_f;
        char* res_hex = bigint261_to_hex(&res_261);
        if (do_assert)
            assert(strcmp(res_hex, expected_for_9x29) == 0);
    }

    avg_a /= num_runs;
    avg_b /= num_runs;
    avg_c /= num_runs;
    avg_d /= num_runs;
    avg_e /= num_runs;
    avg_f /= num_runs;

    printf("%llu Montgomery multiplications with BM17 (non-SIMD) took                             %f ms\n", cost, avg_a);
    printf("%llu Montgomery multiplications with BM17 (SIMD) took                                 %f ms\n", cost, avg_b);
    printf("%llu Montgomery multiplications with CIOS (non-SIMD, without gnark optimisation) took %f ms\n", cost, avg_c);
    printf("%llu Montgomery multiplications with f64s and CIOS (SIMD) took                        %f ms\n", cost, avg_d);
    printf("%llu Montgomery multiplications with 30-bit limbs took                                %f ms\n", cost, avg_e);
    printf("%llu Montgomery multiplications with 29-bit limbs took                                %f ms\n", cost, avg_f);
}
