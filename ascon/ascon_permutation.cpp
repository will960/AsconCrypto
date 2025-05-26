#include <iostream>
#include <cstdint>
#include "ascon_permutation.h"

// 5-bit S-box implementation as a lookup table
uint8_t SBOX[32] = {
    0x4, 0xb, 0x1f, 0x14, 0x1a, 0x15, 0x9, 0x2, 0x1b, 0x5, 0x8, 0x12, 0x1d, 0x3, 0x6, 0x1c,
    0x1e, 0x13, 0x7, 0xe, 0x0, 0xd, 0x11, 0x18, 0x10, 0xc, 0x1, 0x19, 0x16, 0xa, 0xf, 0x17
};

// Function to apply the S-box to a 5-bit value
uint8_t applySBOX(uint8_t x)
{
    return SBOX[x & 0x1f]; // Ensure x is within 5-bit range
}

// Function to apply the Substitution Layer
void substitutionLayer(uint64_t state[5])
{
    uint64_t temp[5]; // Temporary state to store results

    for (int i = 0; i < 5; ++i)
    {
        temp[i] = 0;
    }

    for (int bit = 0; bit < 64; ++bit)
    {
        // Extract 5 bits from the state (one from each 64-bit word)
        uint8_t sboxInput = 0;
        sboxInput |= ((state[0] >> bit) & 0x1) << 4;
        sboxInput |= ((state[1] >> bit) & 0x1) << 3;
        sboxInput |= ((state[2] >> bit) & 0x1) << 2;
        sboxInput |= ((state[3] >> bit) & 0x1) << 1;
        sboxInput |= ((state[4] >> bit) & 0x1) << 0;

        uint8_t sboxOutput = applySBOX(sboxInput);

        // Set the output bits in the temp state
        for (int i = 0; i < 5; ++i)
        {
            temp[i] |= ((uint64_t)((sboxOutput >> (4 - i)) & 0x1)) << bit;
        }
    }

    for (int i = 0; i < 5; ++i)
    {
        state[i] = temp[i];
    }
}

// Right rotation function
uint64_t rotateRight(uint64_t x, int bits)
{
    return (x >> bits) | (x << (64 - bits));
}

// Linear diffusion functions
uint64_t sigma0(uint64_t x)
{
    return x ^ rotateRight(x, 19) ^ rotateRight(x, 28);
}

uint64_t sigma1(uint64_t x)
{
    return x ^ rotateRight(x, 61) ^ rotateRight(x, 39);
}

uint64_t sigma2(uint64_t x)
{
    return x ^ rotateRight(x, 1) ^ rotateRight(x, 6);
}

uint64_t sigma3(uint64_t x)
{
    return x ^ rotateRight(x, 10) ^ rotateRight(x, 17);
}

uint64_t sigma4(uint64_t x)
{
    return x ^ rotateRight(x, 7) ^ rotateRight(x, 41);
}

// Function to apply the Linear Diffusion Layer
void linearDiffusionLayer(uint64_t state[5])
{
    state[0] = sigma0(state[0]);
    state[1] = sigma1(state[1]);
    state[2] = sigma2(state[2]);
    state[3] = sigma3(state[3]);
    state[4] = sigma4(state[4]);
}

const uint8_t roundConstants[16] = {
    0x3c, 0x2d, 0x1e, 0x0f, 0xf0, 0xe1, 0xd2, 0xc3,
    0xb4, 0xa5, 0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b
};

// Function to apply the Constant Addition Layer
void constantAdditionLayer(uint64_t state[5], int rounds, int currentRound)
{
    state[2] ^= roundConstants[16 - rounds + currentRound]; // round is 1-indexed here
}

// Ascon-p permutation function
void asconP(uint64_t state[5], int rounds)
{
    for (int round = 1; round <= rounds; ++round)
    {
        constantAdditionLayer(state, rounds, round);
        substitutionLayer(state);
        linearDiffusionLayer(state);
    }
}
