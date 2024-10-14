// TODO: implement:
// - Random BigInt generation
//   - Use https://github.com/LekKit/sha256
// - bigint_to_hex()
// - eq, gt, sub

#include <stdint.h>
#include <stdbool.h>

// Multiprecision big-integers

// 8 x 32-bit limbs in little-endian form.
typedef struct {
    uint64_t v[8];
} BigInt_8_32;

typedef BigInt_8_32 BigInt256;

/*
 * Returns a new BigInt_8_32 initialized to zero.
 */
BigInt_8_32 bigint_new() {
    BigInt_8_32 result = {0}; // Initialize all elements to zero
    return result;
}

/*
 * Returns true if each limb of a and b are equal. Otherwise, returns false.
 */
bool bigint_eq(const BigInt_8_32 *a, const BigInt_8_32 *b) {
    for (int i = 0; i < 8; i++) {
        if (a->v[i] != b->v[i]) {
            return false;
        }
    }
    return true;
}

/*
 * Returns true if a > b. Otherwise, returns false.
 */
bool bigint_gt(const BigInt_8_32 *a, const BigInt_8_32 *b) {
    // Start from the most significant limb
    for (int i = 7; i >= 0; i--) {
        if (a->v[i] > b->v[i])
            return true;
        else if (a->v[i] < b->v[i])
            return false;
    }
    return false; // a is equal to b
}

/*
 * Stores a - b in result, and returns the borrow.
 */
// Function to compute result = a - b. Assumes that a > b.
// Returns true if the subtraction did not borrow (a >= b), false otherwise
uint64_t bigint_sub(BigInt_8_32 *result, const BigInt_8_32 *a, const BigInt_8_32 *b) {
    uint32_t num_limbs = 8; // Number of limbs in BigInt_8_32
    uint64_t borrow = 0;
    uint64_t limb_mask = 0xFFFFFFFF; // Mask for 32-bit limbs

    for (uint32_t i = 0; i < num_limbs; i++) {
        uint64_t lhs_limb = a->v[i];
        uint64_t rhs_limb = b->v[i];

        uint64_t diff = lhs_limb - rhs_limb - borrow;
        result->v[i] = (diff & limb_mask);

        borrow = (diff >> 32) & 1;
    }

    // Return the borrow
    return borrow;
}
