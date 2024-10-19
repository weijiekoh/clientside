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

int main() {
    uint64_t cost = 1 << 20;

    // Only check the result against the hardcoded expected constants if cost == 1024
    bool do_assert = cost == 1 << 10;

    double start, end;
    int result;
    BigInt256 p, ar, br, res;
    BigIntF255 p_f, ar_f, br_f, res_f;

    char* p_hex =  "12ab655e9a2ca55660b44d1e5c37b00159aa76fed00000010a11800000000001";
    char* ar_hex = "05552c9522974fd00772ef23d45519d4d3a486218b3a0d238a25560beda10ce9";
    char* br_hex = "094c0e4dc5769c3bcc4c984fa08b0ceaf437545d83d259471a983cf05e97f19b";
    // These results are hardcoded for cost == 1024
    char* expected_hex = "116a8d07bb676b153699f5744d2eace047a0646f4cfe06012a0bbd53720543b1";
    char* expected_for_cios_f64_hex = "120af9332aca0835cba7214954b32e70d0a56db70e7f03011a0e9ea9b902a1d9";

    result = hex_to_bigint256(p_hex, &p);
    assert(result == 0);
    result = hex_to_bigint256(ar_hex, &ar);
    assert(result == 0);
    result = hex_to_bigint256(br_hex, &br);
    assert(result == 0);

    // Benchmark bm17_non_simd_mont_mul
    start = emscripten_get_now();
    res = reference_func_bm17_non_simd(&ar, &br, &p, 1, cost);
    end = emscripten_get_now();
    char *res_hex = bigint_to_hex(&res);
    if (do_assert)
        assert(strcmp(res_hex, expected_hex) == 0);
    printf("%llu Montgomery multiplications with BM17 (non-SIMD) took %fms\n", cost, end - start);

    // Benchmark bm17_simd_mont_mul. It is slower in WASM because it uses SIMD opcodes that are not
    // natively executed by the CPU.
    start = emscripten_get_now();
    res = reference_func_bm17_simd(&ar, &br, &p, 1, cost);
    end = emscripten_get_now();
    res_hex = bigint_to_hex(&res);
    if (do_assert)
        assert(strcmp(res_hex, expected_hex) == 0);
    printf("%llu Montgomery multiplications with BM17 (SIMD) took %fms\n", cost, end - start);

    // Benchmark mont_mul_cios
    uint64_t p_wide[9] = {0};
    for (int i = 0; i < 8; i ++) {
        p_wide[i] = p.v[i];
    }
    start = emscripten_get_now();
    res = reference_func_mont_mul_cios(&ar, &br, &p, p_wide, 4294967295, cost);
    end = emscripten_get_now();
    res_hex = bigint_to_hex(&res);
    if (do_assert)
        assert(strcmp(res_hex, expected_hex) == 0);
    printf("%llu Montgomery multiplications with CIOS (non-SIMD, without gnark optimisation) took %fms\n", cost, end - start);

    // Benchmark mont_mul_cios_f64_simd
    ar_hex = "0c0048f9de61fa9334139e21184664eb16a77e902d9d06924a1b6b05f6d08675";
    br_hex = "0dfbb9d62fd1a0c9168072b6fe615e7626f0e5ae29e92ca41254de782f4bf8ce";
    // Convert to BigIntF255
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

    start = emscripten_get_now();
    res_f = reference_func_mont_mul_cios_f64_simd(&ar_f, &br_f, &p_f, &p_for_redc, n0, cost);
    end = emscripten_get_now();

    char res_f_hex[65];
    bigintf255_to_hex(&res_f, res_f_hex);

    if (do_assert)
        assert(strcmp(res_f_hex, expected_for_cios_f64_hex) == 0);
    printf("%llu Montgomery multiplications with f64s and CIOS (SIMD) took %fms\n", cost, end - start);
}
