#!/usr/bin/env bash

run_benchmark() {
    local script_path="$1"
    echo "Running $script_path"
    time node "$script_path"
    echo
}

benchmark_dir="build/benchmarks"

run_benchmark "$benchmark_dir/bench_mul_and_add.js"
run_benchmark "$benchmark_dir/bench_fma.js"
run_benchmark "$benchmark_dir/bench_simd_mul.js"
run_benchmark "$benchmark_dir/bench_mul.js"
run_benchmark "$benchmark_dir/bench_mont_mul.js"
