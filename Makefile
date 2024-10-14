# Niall's flags for reference:
#CC := clang-17
#CFLAGS := --target=wasm32 -matomics -msimd128 -mrelaxed-simd -O3 -nostdlib -DWASM
#LD_FLAGS := --no-entry --export=lMultiples --export=hMultiples --export=multiples --export=pointsAddress \
		#--export=scalarsAddress --export=resultAddress --export=initializeCounters --export=computeMSM  \
		#--export=__stack_pointer --export=__heap_base \
            #--features=bulk-memory,atomics,mutable-globals,sign-ext,simd128,relaxed-simd \
		#--lto-O3 --import-memory --shared-memory --initial-memory=460390400 --max-memory=460390400 
#TARGET := hello.wasm
#SRC := hello.c

#CFLAGS := --target=wasm32 -matomics -msimd128 -mrelaxed-simd -O3 -nostdlib -DWASM
CC := emcc
CFLAGS      := -DWASM -msimd128 -mrelaxed-simd -O3
TEST_CFLAGS := -DWASM -msimd128 -mrelaxed-simd -O3

NODE := $(shell which node)
TIME := $(shell which time)

all: clean mkdir tests benchmarks

mkdir:
	mkdir -p build/tests build/benchmarks

clean:
	rm -rf build/*

# Tests
tests: test_rand test_simd test_bigint test_mont

run_tests:
	$(NODE) build/tests/test_rand.js
	$(NODE) build/tests/test_simd.js
	$(NODE) build/tests/test_bigint.js
	$(NODE) build/tests/test_mont.js

test_simd: N := test_simd
test_simd:
	mkdir -p build/tests
	$(CC) $(TEST_CFLAGS) tests/$(N).c -o build/tests/$(N).wasm -o build/tests/$(N).js

run_test_simd:
	$(NODE) build/tests/test_simd.js

test_rand: N := test_rand
test_rand:
	mkdir -p build/tests
	$(CC) $(TEST_CFLAGS) tests/$(N).c -o build/tests/$(N).wasm -o build/tests/$(N).js

run_test_rand:
	$(NODE) build/tests/test_rand.js

test_bigint: N := test_bigint
test_bigint:
	mkdir -p build/tests
	$(CC) $(TEST_CFLAGS) tests/$(N).c -o build/tests/$(N).wasm -o build/tests/$(N).js

run_test_bigint:
	$(NODE) build/tests/test_bigint.js

test_mont: N := test_mont
test_mont:
	mkdir -p build/tests
	$(CC) $(TEST_CFLAGS) tests/$(N).c -o build/tests/$(N).wasm -o build/tests/$(N).js

run_test_mont:
	$(NODE) build/tests/test_mont.js

# Benchmarks
benchmarks: bench_fma bench_mul_and_add bench_mul bench_mont_mul bench_simd_mul

run_benchmarks:
	./run_benchmarks.sh

bench_mul: N := bench_mul
bench_mul:
	mkdir -p build/benchmarks
	$(CC) $(CFLAGS) benchmarks/$(N).c -o build/benchmarks/$(N).wasm -o build/benchmarks/$(N).js

run_bench_mul: N := bench_mul
run_bench_mul:
	$(TIME) $(NODE) build/benchmarks/$(N).js

bench_simd_mul: N := bench_simd_mul
bench_simd_mul:
	mkdir -p build/benchmarks
	$(CC) $(CFLAGS) benchmarks/$(N).c -o build/benchmarks/$(N).wasm -o build/benchmarks/$(N).js

run_bench_simd_mul: N := bench_simd_mul
run_bench_simd_mul:
	$(TIME) $(NODE) build/benchmarks/$(N).js

bench_fma: N := bench_fma
bench_fma:
	mkdir -p build/benchmarks
	$(CC) $(CFLAGS) benchmarks/$(N).c -o build/benchmarks/$(N).wasm -o build/benchmarks/$(N).js

run_bench_fma: N := bench_fma
run_bench_fma:
	$(TIME) $(NODE) build/benchmarks/$(N).js

bench_mul_and_add: N := bench_mul_and_add
bench_mul_and_add:
	mkdir -p build/benchmarks
	$(CC) $(CFLAGS) benchmarks/$(N).c -o build/benchmarks/$(N).wasm -o build/benchmarks/$(N).js

run_bench_mul_and_add: N := bench_mul_and_add
run_bench_mul_and_add:
	$(TIME) $(NODE) build/benchmarks/$(N).js

bench_mont_mul: N := bench_mont_mul
bench_mont_mul:
	mkdir -p build/benchmarks
	$(CC) $(CFLAGS) benchmarks/$(N).c -o build/benchmarks/$(N).wasm -o build/benchmarks/$(N).js

run_bench_mont_mul: N := bench_mont_mul
run_bench_mont_mul:
	$(TIME) $(NODE) build/benchmarks/$(N).js
