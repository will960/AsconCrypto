# INFO-F514 - Protocols, cryptanalysis and mathematical cryptology
In this repo, you will find the implementation of ASCON permutation, hash (with it's benchmarks) and authenticated encryption algorithms in C++. 

## Permutation Function (ascon foler):
  - header: ascon_permutation.h
  - source files: ascon_permutation.cpp new_permutation.cpp

## Hash(ascon foler):
  - ascon_hash.cpp

## Authenticated Encryption(ascon foler):
  - ascon_auth_encryption.cpp

## Benchmarks
### Intel Benchmark (benchmark/Intel folder):
  - source file: benchmark.cpp
  - results: benchmark.png benchmark_results.csv
  - executable: benchmark
### Arduino Benchmark (benchmark/Arduino folder):
  - source file: benchmark.cpp
  - results: results.txt
