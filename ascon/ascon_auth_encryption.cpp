#include <iostream>
#include <cstring>
#include <cstdint>
#include <iomanip>
#include "ascon_permutation.cpp"
#define ASCON_ROUNDS 12
#define RATE 8   // Bytes absorbed per permutation round (64 bits)

const uint64_t ROUND_CONSTANTS[ASCON_ROUNDS] = {
    0x00000000000000f0ULL, 0x00000000000000e1ULL, 0x00000000000000d2ULL, 0x00000000000000c3ULL,
    0x00000000000000b4ULL, 0x00000000000000a5ULL, 0x0000000000000096ULL, 0x0000000000000087ULL,
    0x0000000000000078ULL, 0x0000000000000069ULL, 0x000000000000005aULL, 0x000000000000004bULL
};

// Bit-based ASCON permutation replacement
void ascon_permutation(uint64_t* state) {
    asconP(state, ASCON_ROUNDS);
}


// Encryption function
void ascon_encrypt(uint8_t* ciphertext, uint8_t* tag,
                   const uint8_t* plaintext, uint64_t plaintext_len,
                   const uint8_t* key, const uint8_t* nonce,
                   const uint8_t* associated_data, uint64_t ad_len) {
    uint64_t state[5] = {0};
    state[0] = 0x80400c0600000000ULL;
    memcpy(&state[1], nonce, 8);
    memcpy(&state[2], key, 8);
    memcpy(&state[3], key + 8, 8);
    state[4] = 0;
    ascon_permutation(state);

    // Associated Data
    for (uint64_t i = 0; i < ad_len; i += RATE) {
        uint64_t block = 0;
        size_t chunk_size = (ad_len - i < RATE) ? (ad_len - i) : RATE;
        memcpy(&block, associated_data + i, chunk_size);
        if (chunk_size < RATE) reinterpret_cast<uint8_t*>(&block)[chunk_size] = 0x80;
        state[0] ^= block;
        ascon_permutation(state);
    }
    state[4] ^= 0x0000000000000001ULL;

    // Encrypt
    for (uint64_t i = 0; i < plaintext_len; i += RATE) {
        uint64_t block = 0;
        memcpy(&block, plaintext + i, RATE);
        state[0] ^= block;
        memcpy(ciphertext + i, &state[0], RATE);
        ascon_permutation(state);
    }

    // Finalization and Tag
    state[1] ^= reinterpret_cast<const uint64_t*>(key)[0];
    state[2] ^= reinterpret_cast<const uint64_t*>(key)[1];
    ascon_permutation(state);
    state[3] ^= reinterpret_cast<const uint64_t*>(key)[0];
    state[4] ^= reinterpret_cast<const uint64_t*>(key)[1];
    ascon_permutation(state);
    memcpy(tag, &state[3], 16);
}

// Decryption function
int ascon_decrypt(uint8_t* plaintext,
                  const uint8_t* ciphertext, uint64_t ciphertext_len,
                  const uint8_t* key, const uint8_t* nonce,
                  const uint8_t* associated_data, uint64_t ad_len,
                  const uint8_t* tag) {
    uint64_t state[5] = {0};
    state[0] = 0x80400c0600000000ULL;
    memcpy(&state[1], nonce, 8);
    memcpy(&state[2], key, 8);
    memcpy(&state[3], key + 8, 8);
    state[4] = 0;
    ascon_permutation(state);

    // Associated Data
    for (uint64_t i = 0; i < ad_len; i += RATE) {
        uint64_t block = 0;
        size_t chunk_size = (ad_len - i < RATE) ? (ad_len - i) : RATE;
        memcpy(&block, associated_data + i, chunk_size);
        if (chunk_size < RATE) reinterpret_cast<uint8_t*>(&block)[chunk_size] = 0x80;
        state[0] ^= block;
        ascon_permutation(state);
    }
    state[4] ^= 0x0000000000000001ULL;

    // Decrypt
    for (uint64_t i = 0; i < ciphertext_len; i += RATE) {
        uint64_t temp = state[0];
        uint64_t block = 0;
        memcpy(&block, ciphertext + i, RATE);
        uint64_t plain_block = temp ^ block;
        memcpy(plaintext + i, &plain_block, RATE);
        state[0] = block;
        ascon_permutation(state);
    }

    // Finalization
    state[1] ^= reinterpret_cast<const uint64_t*>(key)[0];
    state[2] ^= reinterpret_cast<const uint64_t*>(key)[1];
    ascon_permutation(state);
    state[3] ^= reinterpret_cast<const uint64_t*>(key)[0];
    state[4] ^= reinterpret_cast<const uint64_t*>(key)[1];
    ascon_permutation(state);

    uint8_t computed_tag[16];
    memcpy(computed_tag, &state[3], 16);
    return memcmp(computed_tag, tag, 16) == 0 ? 0 : -1;
}

// Main function
int main() {
    uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t nonce[8] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7};
    uint8_t plaintext[16] = "Hello, ASCON!";
    uint8_t ciphertext[16];
    uint8_t tag[16];
    uint8_t associated_data[16] = "Header Data";
    uint8_t decrypted[16];

    std::cout << "Plaintext: " << plaintext << "\n";

    ascon_encrypt(ciphertext, tag, plaintext, 16, key, nonce, associated_data, 16);
    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; i++) std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)ciphertext[i] << " ";
    std::cout << "\nTag: ";
    for (int i = 0; i < 16; i++) std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)tag[i] << " ";
    std::cout << std::dec << "\n";

    if (ascon_decrypt(decrypted, ciphertext, 16, key, nonce, associated_data, 16, tag) == 0) {
        std::cout << "Decryption Successful!\nDecrypted Text: " << decrypted << "\n";
    } else {
        std::cout << "Decryption Failed: Authentication Error!\n";
    }

    return 0;
}
