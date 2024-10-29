#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include "./simd.h"

typedef struct {
    f128 v[5];
} BigInt_5_51;

typedef BigInt_5_51 BigIntF255;

/*
 * Returns a new BigInt_5_51 initialized to zero.
 */
BigInt_5_51 bigintf_new() {
    f128 zero = f64x2_splat(0);
    BigInt_5_51 result = {zero}; // Initialize all elements to zero
    return result;
}

//TODO
BigInt_5_51 bigintf_sub(
    BigInt_5_51 *a,
    BigInt_5_51 *b
) {
    BigInt_5_51 res = bigintf_new();
    uint64_t a_limb;
    uint64_t b_limb;
    uint64_t s;
    for (int i = 0; i < 5; i ++) {
        memcpy(&a_limb, &(a->v[i]), sizeof(uint64_t));
        memcpy(&b_limb, &(b->v[i]), sizeof(uint64_t));
        s = a_limb - b_limb;
        memcpy(&(res.v[i]), &s, sizeof(uint64_t));
    }
    return res;
}

uint64_t extract_bits(uint64_t *num, int start_bit, int num_bits) {
    int start_idx = start_bit / 64;
    int start_offset = start_bit % 64;
    int end_bit = start_bit + num_bits - 1;
    int end_idx = end_bit / 64;

    uint64_t result;

    if (start_idx == end_idx) {
        result = (num[start_idx] >> start_offset) & ((1ULL << num_bits) - 1);
    } else if (end_idx == start_idx + 1) {
        uint64_t low_part = num[start_idx] >> start_offset;
        uint64_t high_part = num[end_idx] << (64 - start_offset);
        result = (low_part | high_part) & ((1ULL << num_bits) - 1);
    } else {
        // Should not occur for 51-bit limbs
        return 0;
    }

    return result;
}

int hex_to_bigintf255(const char *hex_str, BigIntF255 *bigint) {
    // Step 1: Check for null pointers
    if (!hex_str || !bigint) return -1; // Null pointer error

    // Step 2: Check that the string length is exactly 64 characters
    if (strlen(hex_str) != 64) {
        return -2; // Invalid length
    }

    // Step 3: Convert hex string to bytes with error checking
    uint8_t bytes[32];
    for (int i = 0; i < 32; i++) {
        char c1 = hex_str[2 * i];
        char c2 = hex_str[2 * i + 1];

        int b1, b2;

        if (isdigit((unsigned char)c1))
            b1 = c1 - '0';
        else if (c1 >= 'a' && c1 <= 'f')
            b1 = c1 - 'a' + 10;
        else if (c1 >= 'A' && c1 <= 'F')
            b1 = c1 - 'A' + 10;
        else
            return -3; // Invalid hex character

        if (isdigit((unsigned char)c2))
            b2 = c2 - '0';
        else if (c2 >= 'a' && c2 <= 'f')
            b2 = c2 - 'a' + 10;
        else if (c2 >= 'A' && c2 <= 'F')
            b2 = c2 - 'A' + 10;
        else
            return -3; // Invalid hex character

        bytes[i] = (uint8_t)((b1 << 4) | b2);
    }

    // Step 4: Read bytes into num[4] in little-endian order
    uint64_t num[4] = {0};
    for (int i = 0; i < 4; i++) {
        num[i] = 0;
        for (int j = 0; j < 8; j++) {
            num[i] |= ((uint64_t)bytes[31 - (i * 8 + j)]) << (j * 8);
        }
    }

    // Step 5: Extract limbs (51 bits each)
    uint64_t limb[5];
    for (int i = 0; i < 5; i++) {
        limb[i] = extract_bits(num, i * 51, 51);
    }

    // Step 6: Store limbs in BigIntF255
    for (int i = 0; i < 5; i++) {
        //memcpy(&(bigint->v[i]), &(limb[i]), sizeof(uint64_t));
        bigint->v[i] = f64x2_make((double) limb[i], 0);
    }

    return 0; // Success
}

void bigintf255_to_hex(const BigIntF255 *num, char* hexString) {
    uint64_t limbs[5];

    // Extract the integer values from the double limbs
    for (int i = 0; i < 5; i++) {
        limbs[i] = (uint64_t) f64x2_extract_l(num->v[i]);
    }

    // Now, assemble the limbs into bytes
    uint8_t bigInt[32] = {0};

    uint64_t acc = 0;
    int acc_bits = 0;
    int byte_index = 0;

    for (int i = 0; i < 5; i++) {
        acc |= limbs[i] << acc_bits;
        acc_bits += 51;

        while (acc_bits >= 8 && byte_index < 32) {
            bigInt[byte_index++] = acc & 0xFF;
            acc >>=8;
            acc_bits -=8;
        }
    }

    // Handle remaining bits
    if (byte_index < 32 && acc_bits > 0) {
        bigInt[byte_index++] = acc & 0xFF;
    }

    // Now, bigInt[0..31] contains the 256-bit integer in little-endian order

    // Convert to big-endian hex string by reversing the byte array
    for (int i = 0; i < 32; i++) {
        sprintf(hexString + i * 2, "%02x", bigInt[31 - i]);
    }

    hexString[64] = '\0';  // Null-terminate the string
}
