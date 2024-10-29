// SIMD functions
#if !defined(WASM)
#error Please compile with -DWASM.
#endif

#if defined(WASM)
#include <wasm_simd128.h>

typedef v128_t f128;
typedef v128_t i128;

/*
 * Convert an i128 to a f128
 */
inline f128 i2f(i128 x) { return x; }

/*
 * Convert a f128 to an i128
 */
inline i128 f2i(f128 x) { return x; }

f128 f64x2_make(double a, double b);
inline f128 f64x2_make(double a, double b) {
    return wasm_f64x2_make(a, b);
}

i128 i64x2_splat(uint64_t a) {
    return wasm_i64x2_splat(a);
}

i128 f64x2_splat(uint64_t a) {
    return wasm_f64x2_splat(a);
}

i128 i64x2_make(uint64_t a, uint64_t b) {
    return wasm_u64x2_make(a, b);
}

i128 i64x2_mul(i128 a, i128 b) {
    return wasm_i64x2_mul(a, b);
}

i128 u64x2_mul(i128 a, i128 b) {
    return wasm_i64x2_mul(a, b);
}

i128 i64x2_add(i128 a, i128 b) {
    return wasm_i64x2_add(a, b);
}

i128 i64x2_and(i128 a, i128 b) {
    return wasm_v128_and(a, b);
}

i128 u64x2_shr(i128 a, uint32_t b) {
    return wasm_u64x2_shr(a, b);
}

i128 i64x2_shr(i128 a, uint32_t b) {
    return wasm_i64x2_shr(a, b);
}

i128 i128_and(i128 a, i128 b) {
    return wasm_v128_and(a, b);
}

/*
 * Fused-multiply-add with relaxed SIMD. Returns a * b + c where the
 * multiplication is performed with infinite precision, and rounding occurs
 * only during the addition step.
 */
f128 f64x2_fma(f128 a, f128 b, f128 c);
inline f128 f64x2_fma(f128 a, f128 b, f128 c) {
    return wasm_f64x2_relaxed_madd(a, b, c);
}

f128 f64x2_sub(f128 a, f128 b);
inline f128 f64x2_sub(f128 a, f128 b) {
    return wasm_f64x2_sub(a, b);
}

/*
 * Returns the first (index 0) 64-bit value in the given i128.
 */
uint64_t i64x2_extract_l(i128 x);
inline uint64_t i64x2_extract_l(i128 x) {
    return wasm_i64x2_extract_lane(x, 0);
}

/*
 * Returns the second (index 1) 64-bit value in the given i128.
 */
uint64_t i64x2_extract_h(i128 x);
inline uint64_t i64x2_extract_h(i128 x) {
    return wasm_i64x2_extract_lane(x, 1);
}

/*
 * Returns the first (index 0) 64-bit value in the given f128.
 */
double f64x2_extract_l(f128 x);
inline double f64x2_extract_l(f128 x) {
    return wasm_f64x2_extract_lane(x, 0);
}

/*
 * Returns the second (index 1) 64-bit value in the given f128.
 */
double f64x2_extract_h(f128 x);
inline double f64x2_extract_h(f128 x) {
    return wasm_f64x2_extract_lane(x, 1);
}

/*
 * Print the binary representation of an f128 (f0f1) as such:
 * "(f0: <sign> <exp minus 1023 in decimal> <mantissa in hex>, f1: <sign> <exp
 * minus 1023 in decimal> <mantissa in hex>)"
 */
void print_f128(f128 in) {
    unsigned long long f0, f1;
    uint8_t sign0, sign1;
    uint32_t exp0, exp1;
    uint64_t mantissa0, mantissa1;

    f0 = i64x2_extract_l(in);
    f1 = i64x2_extract_h(in);

    sign0 = f0 >> 63;
    sign1 = f1 >> 63;

    uint64_t mantissa_mask = 0xfffffffffffff;
    exp0 = ((f0 >> 52) & 0x7ff) - 1023;
    exp1 = ((f1 >> 52) & 0x7ff) - 1023;

    mantissa0 = f0 & mantissa_mask;
    mantissa1 = f1 & mantissa_mask;

    char sign0_char, sign1_char;

    sign0_char = sign0 == 0 ? '+' : '-';
    sign1_char = sign1 == 0 ? '+' : '-';

    printf(
        "(%c %d %013llx, %c %d %013llx)",
        sign0_char, exp0, mantissa0, sign1_char, exp1, mantissa1
    );
}

/*
 * Print the binary representation of an i128 (i0i1) as two 64-bit hexadecimal
 * values: "(i0, i1)"
 */
void print_i128(i128 in) {
    unsigned long long v[2];
    wasm_v128_store((i128*) v, in);
    printf("(%016llx %016llx)", v[0], v[1]);
}

/*
 * Print the binary representation of an f128 (i0i1) as two 64-bit hexadecimal
 * values: "(i0, i1)"
 */
void print_f128_hex(f128 in) {
    print_i128(f2i(in));
}
#endif
