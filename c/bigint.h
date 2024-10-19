// TODO: implement:
// - functions using inline assembly?: https://webassembly.github.io/wabt/demo/wat2wasm/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sha256/sha256.c"

typedef struct {
    uint64_t v[9];
} BigInt_9_30;
typedef BigInt_9_30 BigInt270;

// 8 x 32-bit limbs in little-endian form.
// We use 64-bit variables to store 32-bit limbs because the algorithm we use
// to perform multiplication requires us to compute 64-bit limb products.
typedef struct {
    uint64_t v[8];
} BigInt_8_32;

typedef BigInt_8_32 BigInt256;

/*
 * Returns a pseudorandomly generated BigInt_8_32.
 */
BigInt_8_32 bigint_rand(uint64_t seed) {
    BigInt_8_32 result = {0}; // Initialize all elements to zero
    char data[8];
    uint8_t* hash;
    for (int i = 0; i < 8; i++) {
        data[i] = (seed >> (56 - i * 8)) & 0xFF;
    }

    // The 32-bit hash
    hash = (uint8_t*) malloc(32);
 
    // Perform the hash
    sha256_easy_hash(data, 8, hash);

    // hash now contains 32 uint8_t values.

    // Populate the limbs of result with the data in hash.
    for (int i = 0; i < 8; i++) {
        int base = 28 - (i * 4); // Calculate the starting index for each limb
        result.v[i] = 
            ((uint64_t)hash[base]) |
            ((uint64_t)hash[base + 1] << 8) |
            ((uint64_t)hash[base + 2] << 16) |
            ((uint64_t)hash[base + 3] << 24);
    }

    free(hash);

    return result;
}

/*
 * Converts a Bigint_8_32 into a big-endian hexadecimal string.
 * Remember to free() it after use.
 */
char* bigint_to_hex(const BigInt_8_32 *val) {
    // Allocate memory for 64 characters plus the null terminator
    char *hex_str;
    hex_str = (char*) malloc(65);
    if (!hex_str) return NULL; // Check for allocation failure

    // Process each limb in reverse order (big-endian output)
    for (int i = 0; i < 8; i++) {
        // Get the limb from most significant to least significant
        uint32_t limb = (uint32_t)(val->v[7 - i] & 0xFFFFFFFFULL);
        // Write 8 hex digits per limb into the string
        sprintf(hex_str + i * 8, "%08x", limb);
    }

    hex_str[64] = '\0'; // Null-terminate the string
    return hex_str;
}

// Helper function to convert 8 hex characters to a uint32_t
int hex_to_uint32(const char *hex_str, uint32_t *value) {
    uint32_t result = 0;
    for (int i = 0; i < 8; i++) {
        char c = hex_str[i];
        uint8_t digit;

        if (c >= '0' && c <= '9') {
            digit = (uint8_t)(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            digit = (uint8_t)(c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            digit = (uint8_t)(c - 'A' + 10);
        } else {
            return -1; // Invalid character
        }

        result = (result << 4) | digit;
    }
    *value = result;
    return 0;
}

/*
 * Convert a 64-character big-endian hexadecimal string to a Bigint256.
 */
// Function to convert hex string to BigInt256
int hex_to_bigint256(const char *hex_str, BigInt256 *val) {
    if (!hex_str || !val) return -1; // Null pointer error

    // Check that the string length is exactly 64 characters
    if (strlen(hex_str) != 64) {
        printf("strlen: %lu\n", strlen(hex_str));

        return -2; // Invalid length
    }

    // Process each limb
    for (int i = 0; i < 8; i++) {
        // Get the substring for this limb
        const char *limb_str = hex_str + i * 8;
        uint32_t limb_value;

        // Convert the 8-character hex substring to a uint32_t
        if (hex_to_uint32(limb_str, &limb_value) != 0) {
            return -3; // Invalid hex character encountered
        }

        // Since limbs are stored in little-endian order, assign in reverse
        val->v[7 - i] = (uint64_t)limb_value;
    }

    return 0; // Success
}

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
