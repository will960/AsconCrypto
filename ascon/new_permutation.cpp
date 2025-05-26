#include <iostream>
#include <cstdint>
#include <array>

// Ascon round constants (Table 5 in the spec)
constexpr std::array<uint64_t, 16> ROUND_CONSTANTS = {
    0x3c, 0x2d, 0x1e, 0x0f, 0xf0, 0xe1, 0xd2, 0xc3,
    0xb4, 0xa5, 0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b
};

void constantAdditionLayer(uint64_t state[5], int rounds, int round)
{
    state[2] ^= ROUND_CONSTANTS[16 - rounds + round];
}

// Apply the S-Box layer (bit-sliced implementation)
void substitutionLayer(uint64_t state[5])
{
    uint64_t x0 = state[0], x1 = state[1], x2 = state[2], x3 = state[3], x4 = state[4];
    uint64_t t0, t1, t2, t3, t4;

    // Initial XORs
    x0 ^= x4; x4 ^= x3; x2 ^= x1;

    // Copy and invert
    t0 = ~x0; t1 = ~x1; t2 = ~x2; t3 = ~x3; t4 = ~x4;

    // AND with shifted variables
    t0 &= x1; t1 &= x2; t2 &= x3; t3 &= x4; t4 &= x0;

    // XOR results back
    x0 ^= t1; x1 ^= t2; x2 ^= t3; x3 ^= t4; x4 ^= t0;

    // Final operations
    x1 ^= x0; x0 ^= x4; x3 ^= x2; x2 = ~x2;

    state[0] = x0;
    state[1] = x1;
    state[2] = x2;
    state[3] = x3;
    state[4] = x4;
}

// Linear diffusion functions (as per spec)
inline uint64_t sigma0(uint64_t x) { return x ^ (x >> 19 | x << (64-19)) ^ (x >> 28 | x << (64-28)); }
inline uint64_t sigma1(uint64_t x) { return x ^ (x >> 61 | x << (64-61)) ^ (x >> 39 | x << (64-39)); }
inline uint64_t sigma2(uint64_t x) { return x ^ (x >> 1  | x << (64-1))  ^ (x >> 6  | x << (64-6)); }
inline uint64_t sigma3(uint64_t x) { return x ^ (x >> 10 | x << (64-10)) ^ (x >> 17 | x << (64-17)); }
inline uint64_t sigma4(uint64_t x) { return x ^ (x >> 7  | x << (64-7))  ^ (x >> 41 | x << (64-41)); }

void linearDiffusionLayer(uint64_t state[5]) {
    state[0] = sigma0(state[0]);
    state[1] = sigma1(state[1]);
    state[2] = sigma2(state[2]);
    state[3] = sigma3(state[3]);
    state[4] = sigma4(state[4]);
}

void asconP(uint64_t state[5], int rounds) {
    for (int round = 1; round <= rounds; ++round) {
        constantAdditionLayer(state, rounds, round);
        substitutionLayer(state);
        linearDiffusionLayer(state);
    }
}

void printState(const uint64_t state[5], const char* label) {
    std::cout << "\n" << label << ":\n";
    for (int i = 0; i < 5; ++i) {
        std::cout << "S" << i << " = 0x" << std::hex << state[i] << std::endl;
    }
}

int main() {
    uint64_t state8[5] = {
        0x0706050403020100, 0x0f0e0d0c0b0a0908,
        0x1716151413121110, 0x1f1e1d1c1b1a1918,
        0x2726252423222120
    };

    printState(state8, "Ascon-p[8] - Initial State");
    asconP(state8, 8);
    printState(state8, "Ascon-p[8] - Final State");

    uint64_t state12[5] = {
        0x0706050403020100, 0x0f0e0d0c0b0a0908,
        0x1716151413121110, 0x1f1e1d1c1b1a1918,
        0x2726252423222120
    };

    printState(state12, "Ascon-p[12] - Initial State");
    asconP(state12, 12);
    printState(state12, "Ascon-p[12] - Final State");

    return 0;
}