#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <iomanip>
#include "ascon_permutation.h"  // Include the permutation header

// Padding function
std::vector<uint8_t> pad_message(const std::vector<uint8_t>& msg, size_t rate) {
    std::vector<uint8_t> padded = msg;
    padded.push_back(0x80);
    size_t pad_len = (rate - (padded.size() % rate)) % rate;
    padded.insert(padded.end(), pad_len, 0x00);
    return padded;
}

// Converts 8 bytes to uint64_t big endian
uint64_t bytes_to_u64(const uint8_t* bytes) {
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result = (result << 8) | bytes[i];
    }
    return result;
}

// Converts uint64_t to 8 bytes big endian
void u64_to_bytes(uint64_t val, uint8_t* out) {
    for (int i = 7; i >= 0; --i) {
        out[i] = val & 0xFF;
        val >>= 8;
    }
}

// Ascon-Hash256
std::vector<uint8_t> ascon_hash256(const std::vector<uint8_t>& msg) {
    uint64_t state[5] = {
        0x0000080100cc0002ULL, 0, 0, 0, 0
    };

    const size_t rate = 8;
    std::vector<uint8_t> padded = pad_message(msg, rate);

    for (size_t i = 0; i < padded.size(); i += rate) {
        uint64_t block = bytes_to_u64(&padded[i]);
        state[0] ^= block;
        asconP(state, rate);
    }

    std::vector<uint8_t> output;
    for (int i = 0; i < 4; ++i) {  // 4 x 64-bit = 256-bit output
        uint8_t buf[8];
        u64_to_bytes(state[0], buf);
        output.insert(output.end(), buf, buf + 8);
        asconP(state, rate);
    }

    return output;
}

// Ascon-XOF128
std::vector<uint8_t> ascon_xof128(const std::vector<uint8_t>& msg, size_t L) {
    uint64_t state[5] = {
        0x0000080000cc0003ULL, 0, 0, 0, 0
    };

    const size_t rate = 8;
    std::vector<uint8_t> padded = pad_message(msg, rate);

    for (size_t i = 0; i < padded.size(); i += rate) {
        uint64_t block = bytes_to_u64(&padded[i]);
        state[0] ^= block;
        asconP(state, rate);
    }

    std::vector<uint8_t> output;
    while (output.size() < L) {
        uint8_t buf[8];
        u64_to_bytes(state[0], buf);
        output.insert(output.end(), buf, buf + 8);
        asconP(state, rate);
    }

    output.resize(L);  // Trim to exact length
    return output;
}

