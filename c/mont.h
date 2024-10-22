#include "./bigint.h"
#include "./bigintf.h"
#include <stdint.h>

// TODO: 
// - https://github.com/ingonyama-zk/papers/blob/main/multi_precision_fast_mod_mul.pdf
// Done:
// - Non-SIMD CIOS (32-bit limbs in arrays of uint64_t) from Acar (see Mrabet et al)
// - Non-SIMD CIOS (51-bit limbs in arrays of doubles) from BM17
// - SIMD CIOS (51-bit limbs in arrays of doubles) from BM17
// - Niall's SIMD algo using f64s
// - Non-SIMD CIOS (30-bit limbs in arrays of uint64_t) from Mitscha-Baude

/// Returns the higher 30 bits.
static inline uint64_t hi_30(uint64_t v) {
    return v >> 30;
}

/// Returns the lower 30 bits.
static inline uint64_t lo_30(uint64_t v) {
    return v & 0x3FFFFFFF;
}

bool gt_270(
    uint64_t* s,
    BigInt270 *p
) {
    int i;
    for (int idx = 0; idx < 9; idx ++) {
        i = 8 - idx;
        if (s[i] < p->v[i]) {
            return false;
        } else if (s[i] > p->v[i]) {
            return true;
        }
    }
    return true;
}

void sub_270(
    uint64_t* res,
    uint64_t* s,
    BigInt270 *p
) {
    uint64_t borrow = 0;
    uint64_t mask = 0x3fffffff;

    uint64_t diff;
    for (int i = 0; i < 9; i ++) {
        diff = s[i] - p->v[i] - borrow;
        res[i] = diff & mask;
        borrow = (diff >> 30) & 1;
    }
}

/// See
/// https://github.com/mitschabaude/montgomery/blob/main/doc/zprize22.md#13-x-30-bit-multiplication
/// As a side note, there's no point trying to use FMAs in Gregor's method:
/// i.e. by representing a big integer as an array of f128s, where we store a
/// 30-bit limb in the lower 64-bit lane. First off, when we compute a product,
/// we need the higher and lower 30 bits, so we'll have to use Niall's
/// int_full_product method anyway. Furthermore, we might as well use all 51
/// bits available to us, so as to reduce the number of loop iterations.
BigInt270 mont_mul_9x30(
    BigInt270 *ar,
    BigInt270 *br,
    BigInt270 *p,
    uint64_t mu
) {
    const int nsafe = 8;
    const size_t NUM_LIMBS = 9;
    uint64_t s[9] = {0};

    uint64_t t, t_lo, qi, c;
    for (int i = 0; i < NUM_LIMBS; i ++) {
        t = s[0] + ar->v[i] * br->v[0];
        t_lo = lo_30(t);
        qi = lo_30(mu * t_lo);
        c = hi_30(t + qi * p->v[0]);

        // If we ignore nsafe:
        /*
        for (int j = 1; j < NUM_LIMBS; j ++) {
            c = s[j] + ar->v[i] * br->v[j] + qi * p->v[j] + c;
            s[j - 1] = lo_30(c);
            c = hi_30(c);
        }
        s[NUM_LIMBS - 1] = c;
        */

        // The full modified algo if we know nsafe:
        /*
        for (int j = 1; j < NUM_LIMBS - 1; j ++) {
            t = s[j] + ar->v[i] * br->v[j] + qi * p->v[j];

            if ((j-1) % nsafe == 0) {
                t += c;
            }
            if (j % nsafe == 0) {
                c = hi_30(t);
                s[j - 1] = lo_30(t);
            } else {
                s[j - 1] = t;
            }
        }

        if ((NUM_LIMBS - 2) % nsafe == 0) {
            c = s[NUM_LIMBS - 1] + ar->v[i] * br->v[NUM_LIMBS - 1] + qi * p->v[NUM_LIMBS - 1];
            s[NUM_LIMBS - 2] = lo_30(c);
            c = hi_30(c);
        } else {
            s[NUM_LIMBS - 2] = ar->v[i] * br->v[NUM_LIMBS - 1] + qi * p->v[NUM_LIMBS - 1];
        }
        */

        // When nsafe equals 8, we can further optimise the above:
        t = s[1] + ar->v[i] * br->v[1] + qi * p->v[1];
        t += c;
        s[0] = t;
        for (int j = 2; j < NUM_LIMBS - 1; j ++) {
            t = s[j] + ar->v[i] * br->v[j] + qi * p->v[j];
            s[j - 1] = t;
        }
        s[NUM_LIMBS - 2] = ar->v[i] * br->v[NUM_LIMBS - 1] + qi * p->v[NUM_LIMBS - 1];
    }

    c = 0;
    for (int i = 0; i < NUM_LIMBS; i ++) {
        c = s[i] + c;
        s[i] = lo_30(c);
        c = hi_30(c);
    }

    // Conditional reduction
    BigInt270 res = bigint270_new();
    if (gt_270(s, p)) {
        uint64_t r[9];
        sub_270(r, s, p);
        for (int i = 0; i < 9; i ++) {
            res.v[i] = r[i];
        }
    } else {
        for (int i = 0; i < 9; i ++) {
            res.v[i] = s[i];
        }
    }
    return res;
}

/// Returns the higher 32 bits.
static inline uint64_t hi(uint64_t v) {
    return v >> 32;
}

/// Returns the lower 32 bits.
static inline uint64_t lo(uint64_t v) {
    return v & 0xFFFFFFFF;
}

/// Compares the most significant limb of val agaist that of p.
bool msl_is_greater(
    BigIntF255 *val,
    BigIntF255 *p
) {
    return f64x2_extract_l(val->v[4]) > f64x2_extract_l(p->v[4]);
}

/// Conditional subtraction. p should not have the exponent or sign bits set.
BigIntF255 reduce_bigintf(
    BigIntF255 *val,
    BigIntF255 *p
) {
    if (msl_is_greater(val, p)) {
        return bigintf_sub(val, p);
    }

    return *val;
}

/// TODO: document what this does; it appears to perform carries.
BigIntF255 resolve_bigintf(BigIntF255 *val) {
    uint64_t B = 51;
    uint64_t mask = 0x7ffffffffffff;

    int64_t local[3] = {0};
    BigIntF255 r = bigintf_new();

    // Copy bits from val to v0 - v4
    int64_t v0, v1, v2, v3, v4;
    memcpy(&v0, &(val->v[0]), sizeof(uint64_t));
    memcpy(&v1, &(val->v[1]), sizeof(uint64_t));
    memcpy(&v2, &(val->v[2]), sizeof(uint64_t));
    memcpy(&v3, &(val->v[3]), sizeof(uint64_t));
    memcpy(&v4, &(val->v[4]), sizeof(uint64_t));

    local[0] = v1 + (v0 >> B);
    local[1] = v2 + (local[0] >> B);
    local[2] = v3 + (local[1] >> B);
    uint64_t r4 = v4 + (local[2] >> B);

    r.v[4] = f64x2_make((double) r4, 0);
    local[0] = local[0] & mask;
    local[1] = local[1] & mask;
    local[2] = local[2] & mask;
    r.v[0] = f64x2_make((double) (v0 & mask), 0);
    r.v[1] = f64x2_make((double) local[0], 0);
    r.v[2] = f64x2_make((double) local[1], 0);
    r.v[3] = f64x2_make((double) local[2], 0);

    return r;
}

/// Adapted from https://github.com/z-prize/2023-entries/tree/main/prize-2-msm-wasm/prize-2b-twisted-edwards/yrrid-snarkify
BigIntF255 mont_mul_cios_f64_simd(
    BigIntF255 *ar,
    BigIntF255 *br,
    BigIntF255 *p,
    uint64_t n0
) {
    f128 term, c2, c3, c4, bd[5], lh[5];
    i128 sum[11], c0, c1;
    uint64_t q0, q1;
    const size_t NUM_LIMBS = 5;

    for (int i = 0; i < NUM_LIMBS; i ++) {
        bd[i] = br->v[i];
    }

    // Refer to make_initial() in Niall's paper and also
    // https://github.com/mitschabaude/montgomery/blob/main/src/51x5/fma.ts#L58
    // to understand how these magic numbers are generated.
    sum[0] = i64x2_splat(0x7990000000000000);
    sum[1] = i64x2_splat(0x6660000000000000);
    sum[2] = i64x2_splat(0x5330000000000000);
    sum[3] = i64x2_splat(0x4000000000000000);
    sum[4] = i64x2_splat(0x2CD0000000000000);
    sum[5] = i64x2_splat(0x2680000000000000);
    sum[6] = i64x2_splat(0x39B0000000000000);
    sum[7] = i64x2_splat(0x4CE0000000000000);
    sum[8] = i64x2_splat(0x6010000000000000);
    sum[9] = i64x2_splat(0x7340000000000000);
    sum[10] = i64x2_splat(0x0);

    c0 = i64x2_splat(0x7FFFFFFFFFFFFL);
    c1 = i64x2_splat(0x4330000000000000L);
    c2 = i2f(c1);
    c3 = i2f(i64x2_splat(0x4660000000000000L));
    c4 = i2f(i64x2_splat(0x4660000000000003L));

    for (int i = 0; i < NUM_LIMBS; i ++) {
        term = ar->v[i];

        for (int j = 0; j < NUM_LIMBS; j ++) {
            lh[j] = f64x2_fma(term, bd[j], c3);
        }

        for (int j = 0; j < NUM_LIMBS; j ++) {
            sum[j + 1] = i64x2_add(sum[j + 1], f2i(lh[j]));
        }

        for (int j = 0; j < NUM_LIMBS; j ++) {
            lh[j] = f64x2_sub(c4, lh[j]);
        }

        for (int j = 0; j < NUM_LIMBS; j ++) {
            lh[j] = f64x2_fma(term, bd[j], lh[j]);
        }

        for (int j = 0; j < NUM_LIMBS; j ++) {
            sum[j] = i64x2_add(sum[j], f2i(lh[j]));
        }

        q0 = i64x2_extract_l(sum[0]) * n0;
        q1 = i64x2_extract_h(sum[0]) * n0;
        term = f64x2_sub(
                i2f(
                    i64x2_add(
                        i64x2_and(
                            i64x2_make(q0, q1),
                            c0
                            ),
                        c1
                        )
                   ),
                c2
            );

        for (int j = 0; j < NUM_LIMBS; j ++) {
            lh[j] = f64x2_fma(term, p->v[j], c3);
        }

        for (int j = 0; j < NUM_LIMBS; j ++) {
            sum[j + 1] = i64x2_add(sum[j + 1], f2i(lh[j]));
        }

        for (int j = 0; j < NUM_LIMBS; j ++) {
            lh[j] = f64x2_sub(c4, lh[j]);
        }

        for (int j = 0; j < NUM_LIMBS; j ++) {
            lh[j] = f64x2_fma(term, p->v[j], lh[j]);
        }

        sum[0] = i64x2_add(sum[0], f2i(lh[0]));
        sum[1] = i64x2_add(sum[1], f2i(lh[1]));
        sum[0] = i64x2_add(sum[1], i64x2_shr(sum[0], 51));

        for(int j = 1; j < 4; j ++) {
            sum[j] = i64x2_add(sum[j + 1], f2i(lh[j + 1]));
        }

        sum[4] = sum[5];
        sum[5] = sum[i+6];
    }

    BigIntF255 res = bigintf_new();
    for (int i = 0; i < NUM_LIMBS; i ++) {
        res.v[i] = sum[i];
    }
    return res;
}

/// Amine Mrabet, Nadia El-Mrabet, Ronan Lashermes, Jean-Baptiste Rigaud, Belgacem Bouallegue, et
/// al.. High-performance Elliptic Curve Cryptography by Using the CIOS Method for Modular
/// Multiplication. CRiSIS 2016, Sep 2016, Roscoff, France. hal-01383162
/// https://inria.hal.science/hal-01383162/document, page 4
/// Also see Acar, 1996.
/// This is the "classic" CIOS algorithm.
/// Does not implement the gnark optimisation (https://hackmd.io/@gnark/modular_multiplication),
/// but that should be useful.
/// Does not use SIMD instructions.
BigInt256 mont_mul_cios(
    BigInt256 *ar,
    BigInt256 *br,
    BigInt256 *p,
    uint64_t* p_for_redc,
    uint64_t n0
) {
    size_t NUM_LIMBS = 8;
    size_t B = 32;
    uint64_t mask = 0xffffffff;

    uint64_t t[10] = {0};

    for (int i = 0; i < NUM_LIMBS; i ++) {
        uint64_t c = 0;
        uint64_t cs;
        for (int j = 0; j < NUM_LIMBS; j ++) {
            cs = t[j] + ar->v[i] * br->v[j] + c;
            c = hi(cs);
            t[j] = lo(cs);
        }
        cs = t[NUM_LIMBS] + c;
        c = hi(cs);
        t[NUM_LIMBS] = lo(cs);
        t[NUM_LIMBS + 1] = c;

        uint64_t m = (t[0] * n0) & mask;
        cs = t[0] + m * p->v[0];
        c = hi(cs);

        for (int j = 1; j < NUM_LIMBS; j ++) {
            cs = t[j] + m * p->v[j] + c;
            c = hi(cs);
            t[j - 1] = lo(cs);
        }

        cs = t[NUM_LIMBS] + c;
        c = hi(cs);
        t[NUM_LIMBS - 1] = lo(cs);
        t[NUM_LIMBS] = t[NUM_LIMBS + 1] + c;
    }

    bool t_gt_p = false;
    for (int idx = 0; idx < NUM_LIMBS + 1; idx ++) {
        int i = NUM_LIMBS - idx;
        uint64_t pi = 0;
        if (i < NUM_LIMBS) {
            pi = p->v[i];
        };

        if (t[i] < pi) {
            break;
        } else if (t[i] > pi) {
            t_gt_p = true;
            break;
        }
    }

    if (!t_gt_p) {
        BigInt256 res;
        res = bigint_new();
        for (int i = 0; i < NUM_LIMBS; i ++) {
            res.v[i] = t[i];
        }
        return res;
    }

    uint64_t t_wide[9] = {0};
    for (int i = 0; i < NUM_LIMBS + 1; i ++) {
        t_wide[i] = t[i];
    }

    uint64_t result[9] = {0};
    uint64_t borrow = 0;
    const uint64_t limb_mask = 4294967295;

    for (int i = 0; i < NUM_LIMBS + 1; i ++) {
        uint64_t lhs_limb = t_wide[i];
        uint64_t rhs_limb = p_for_redc[i];
        uint64_t diff = lhs_limb - rhs_limb - borrow;
        result[i] = diff & limb_mask;
        borrow = (diff >> B) & 1;
    }

    BigInt256 res;
    res = bigint_new();
    for (int i = 0; i < NUM_LIMBS; i ++) {
        res.v[i] = result[i];
    }

    return res;
}

/// Algorithm 4 of "Montgomery Arithmetic from a Software Perspective" by Bos and Montgomery
/// without SIMD opcodes.
BigInt256 bm17_non_simd_mont_mul(
    BigInt256 *ar,
    BigInt256 *br,
    BigInt256 *p,
    uint64_t mu
) {
    size_t NUM_LIMBS = 8;
    size_t B = 32;
    uint64_t mask = 0xffffffff;
    uint64_t d[8] = {0};
    uint64_t e[8] = {0};
    uint64_t mu_b0 = mu * br->v[0];
    uint64_t q, t0, t1, d0_minus_e0;

    for (int j = 0; j < NUM_LIMBS; j ++) {
        d0_minus_e0 = d[0] - e[0];
        q = (mu_b0 * ar->v[j] + mu * d0_minus_e0) & mask;
        t0 = (ar->v[j] * br->v[0] + d[0]) >> B;
        t1 = (q * p->v[0] + e[0]) >> B;

        uint64_t p0, p1;
        for (int i = 1; i < NUM_LIMBS; i ++) {
            p0 = ar->v[j] * br->v[i] + t0 + d[i];
            t0 = p0 >> B;
            d[i - 1] = p0 & mask;
            p1 = q * p->v[i] + t1 + e[i];
            t1 = p1 >> B;
            e[i - 1] = p1 & mask;
        }
        d[NUM_LIMBS - 1] = t0;
        e[NUM_LIMBS - 1] = t1;
    }

    BigInt256 d_bigint = bigint_new();
    BigInt256 e_bigint = bigint_new();

    for (int i = 0; i < NUM_LIMBS; i ++) {
        d_bigint.v[i] = d[i];
        e_bigint.v[i] = e[i];
    }

    BigInt256 res;
    res = bigint_new();
    if (bigint_gt(&e_bigint, &d_bigint)) {
        BigInt256 e_minus_d;
        bigint_sub(&e_minus_d, &e_bigint, &d_bigint);
        bigint_sub(&res, p, &e_minus_d);
    } else {
        bigint_sub(&res, &d_bigint, &e_bigint);
    }

    return res;
}

/// Algorithm 4 of "Montgomery Arithmetic from a Software Perspective" by Bos and Montgomery
/// Uses WASM SIMD opcodes.
/// Counterintuitively, in browsers, this runs slower than the non-SIMD version, likely because the
/// SIMD opcodes are emulated rather than executed using the native processor's SIMD instructions. 
/// The performance difference can be seen in benchmarks.
/// See https://emscripten.org/docs/porting/simd.html#optimization-considerations for a list of
/// *some* WASM SIMD instructions which do not have equivalent x86 semantics; those which this
/// function uses probably suffer from the same issue.
/// Also see:
/// https://github.com/coreboot/vboot/blob/060efa0cf64d4b7ccbe3e88140c9da5f747355ee/firmware/2lib/2modpow_sse2.c#L113
/// Note: overflow-checks = false should be set in Cargo.toml under [profile.dev], so the Wrapping
/// trait does not have to be used.
BigInt256 bm17_simd_mont_mul(
    BigInt256 *ar,
    BigInt256 *br,
    BigInt256 *p,
    uint64_t mu
) {
    const size_t NUM_LIMBS = 8;
    uint64_t mask_64 = 0xffffffff;
    i128 mask = i64x2_make(mask_64, mask_64);

    i128 de[8] = {
        i64x2_make(0, 0),
        i64x2_make(0, 0),
        i64x2_make(0, 0),
        i64x2_make(0, 0),
        i64x2_make(0, 0),
        i64x2_make(0, 0),
        i64x2_make(0, 0),
        i64x2_make(0, 0),
    };

    uint64_t mu_b0 = mu * br->v[0];

    i128 bp[8];
    for (int i = 0; i < NUM_LIMBS; i ++) {
        bp[i] = i64x2_make(br->v[i], p->v[i]);
    }

    uint64_t d0, e0, d0_minus_e0, q;
    i128 aq, t01;

    for (int j = 0; j < NUM_LIMBS; j ++) {
        // Compute q
        d0 = i64x2_extract_l(de[0]);
        e0 = i64x2_extract_h(de[0]);
        d0_minus_e0 = d0 - e0;

        // q = (mub0)aj + mu(d0 - e0) mod 2^32
        q = (mu_b0 * ar->v[j] + mu * d0_minus_e0) & mask_64;

        // t0 = ajb0 + d0
        // t1 = qp0 + e0

        // aq = ar[j], q
        aq = i64x2_make(ar->v[j], q);

        t01 = i64x2_add(
            // ajb0, qp0
            u64x2_mul(aq, bp[0]),
            de[0]
        );

        t01 = u64x2_shr(t01, 32);

        i128 p01;
        for (int i = 1; i < NUM_LIMBS; i ++) {
            // p0 = ajbi + t0 + di
            // p1 = qpi + t1 + ei
            p01 = i64x2_add(
                i64x2_add(t01, de[i]),
                u64x2_mul(aq, bp[i])
            );

            // t0 = p0 / 2^32
            // t1 = p1 / 2^32
            t01 = u64x2_shr(p01, 32);

            // d[i-1] = p0 mod 2^32
            // e[i-1] = p1 mod 2^32
            de[i - 1] = i128_and(p01, mask);
        }
        de[NUM_LIMBS - 1] = t01;
    }

    BigInt256 d = bigint_new();
    BigInt256 e = bigint_new();

    for (int i = 0; i < NUM_LIMBS; i ++) {
        d.v[i] = i64x2_extract_l(de[i]);
        e.v[i] = i64x2_extract_h(de[i]);
    }

    BigInt256 res;
    res = bigint_new();
    if (bigint_gt(&e, &d)) {
        BigInt256 e_minus_d;
        bigint_sub(&e_minus_d, &e, &d);
        bigint_sub(&res, p, &e_minus_d);
    } else {
        bigint_sub(&res, &d, &e);
    }

    return res;
}
