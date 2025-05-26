#include <iostream>
#include <functional>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include <x86intrin.h>
#include <openssl/sha.h>
#include "ascon_permutation.h"

// Declare Ascon hash functions
std::vector<uint8_t> ascon_hash256(const std::vector<uint8_t>& msg);
std::vector<uint8_t> ascon_xof128(const std::vector<uint8_t>& msg, size_t L);

// CSV output
std::ofstream csv_out("benchmark_results.csv");

void write_csv_header() {
    csv_out << "Function,InputSize,AvgTime_ms,Throughput_MBps,AvgCycles,Log2Cycles\n";
}

// Generic benchmark runner
void run_benchmark(const std::string& label, size_t input_size, size_t runs,
                   const std::function<std::vector<uint8_t>(const std::vector<uint8_t>&)>& hash_func) {
    std::vector<uint8_t> msg(input_size, 'A');
    hash_func(msg); // Warm-up

    uint64_t total_cycles = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < runs; ++i) {
        uint64_t start_cycles = __rdtsc();
        hash_func(msg);
        uint64_t end_cycles = __rdtsc();
        total_cycles += (end_cycles - start_cycles);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    double avg_time_ms = (elapsed.count() * 1000.0) / runs;
    double throughput_MBps = (input_size * runs / 1e6) / elapsed.count();
    double avg_cycles = static_cast<double>(total_cycles) / runs;
    double log2_cycles = std::log2(avg_cycles);

    std::cout << label << " — Input: " << input_size << " bytes, Runs: " << runs << "\n"
              << "  Avg Time: " << avg_time_ms << " ms\n"
              << "  Throughput: " << throughput_MBps << " MB/s\n"
              << "  Avg Cycles: " << avg_cycles << "\n"
              << "  log2(Cycles): " << log2_cycles << "\n" << std::endl;

    csv_out << label << "," << input_size << "," << avg_time_ms << "," << throughput_MBps
            << "," << avg_cycles << "," << log2_cycles << "\n";
}

// SHA-256 using OpenSSL
std::vector<uint8_t> sha256_hash(const std::vector<uint8_t>& msg) {
    std::vector<uint8_t> digest(SHA256_DIGEST_LENGTH);
    SHA256(msg.data(), msg.size(), digest.data());
    return digest;
}

// Ascon-XOF128 wrapper to fix output length to 32 bytes
std::vector<uint8_t> ascon_xof128_fixed(const std::vector<uint8_t>& msg) {
    return ascon_xof128(msg, 32);  // 256-bit output
}

int main() {
    const size_t runs = 1000;
    write_csv_header();

    // Smaller set of input sizes including 1-byte input
    std::vector<size_t> input_sizes = { 1, 32, 64, 256, 1024 };

    std::cout << "===== Ascon-Hash256 Benchmarks =====\n" << std::endl;
    for (size_t size : input_sizes) {
        run_benchmark("Ascon-Hash256", size, runs, ascon_hash256);
    }

    std::cout << "===== Ascon-XOF128 Benchmarks =====\n" << std::endl;
    for (size_t size : input_sizes) {
        run_benchmark("Ascon-XOF128", size, runs, ascon_xof128_fixed);
    }

    std::cout << "===== SHA-256 Benchmarks (OpenSSL) =====\n" << std::endl;
    for (size_t size : input_sizes) {
        run_benchmark("SHA-256", size, runs, sha256_hash);
    }

    csv_out.close();
    std::cout << "Results saved to benchmark_results.csv" << std::endl;
    return 0;
}

