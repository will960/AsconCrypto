#include <Arduino.h>
#include "SHA256.h"
#include "ascon_hash.h"

// Hash function pointer type
typedef void (*HashFunc)(const uint8_t*, size_t, uint8_t*);

void printHex(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (data[i] < 16) Serial.print("0");
        Serial.print(data[i], HEX);
    }
    Serial.println();
}

void run_benchmark(const char* label, size_t input_size, HashFunc func, size_t runs) {
    uint8_t msg[input_size];
    uint8_t out[32];
    unsigned long total_time = 0;

    memset(msg, 'A', input_size); // Warm-up
    func(msg, input_size, out);

    for (size_t i = 0; i < runs; ++i) {
        for (size_t j = 0; j < input_size; ++j)
            msg[j] = random(0, 256);

        unsigned long start = micros();
        func(msg, input_size, out);
        unsigned long end = micros();

        total_time += (end - start);
    }

    float avg_time_ms = total_time / 1000.0 / runs;
    float throughput_kbps = ((input_size * runs) / 1024.0) / (total_time / 1000000.0);

    Serial.print(label);
    Serial.print(" | Input: ");
    Serial.print(input_size);
    Serial.print(" bytes | Runs: ");
    Serial.print(runs);
    Serial.print(" | Avg Time: ");
    Serial.print(avg_time_ms);
    Serial.print(" ms | Throughput: ");
    Serial.print(throughput_kbps);
    Serial.println(" KB/s");

    delay(50);
}

// SHA-256 wrapper
void sha256_wrapper(const uint8_t* msg, size_t len, uint8_t* out) {
    SHA256 sha;
    sha.update(msg, len);
    auto digest = sha.digest();
    memcpy(out, digest.data(), 32);
}

// Ascon C-style wrapper
extern "C" {
    int crypto_hash(unsigned char* out, const unsigned char* in, unsigned long long inlen);
}

void ascon_hash_wrapper(const uint8_t* msg, size_t len, uint8_t* out) {
    crypto_hash(out, msg, len);
}

void setup() {
    randomSeed(analogRead(0));
    Serial.begin(115200);
    delay(2000);

    size_t sizes[] = {1, 32, 64, 128, 192, 256, 512};
    size_t run_counts[] = {5, 10, 25, 50, 75, 100};

    Serial.println("===== SHA-256 Benchmarks =====");
    for (size_t r = 0; r < sizeof(run_counts) / sizeof(size_t); ++r)
        for (size_t i = 0; i < sizeof(sizes) / sizeof(size_t); ++i)
            run_benchmark("SHA-256", sizes[i], sha256_wrapper, run_counts[r]);

    Serial.println("===== Ascon-Hash256 Benchmarks =====");
    for (size_t r = 0; r < sizeof(run_counts) / sizeof(size_t); ++r)
        for (size_t i = 0; i < sizeof(sizes) / sizeof(size_t); ++i)
            run_benchmark("Ascon-Wrapper", sizes[i], ascon_hash_wrapper, run_counts[r]);
}

void loop() {
    // No loop logic needed
}
