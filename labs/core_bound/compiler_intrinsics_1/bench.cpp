
#include "benchmark/benchmark.h"
#include "solution.h"

static void bench_partial_sum(benchmark::State &state) {
  InputVector inA;
  init(inA);

  OutputVector outB;
  zero(outB, (int)inA.size());

  for (auto _ : state) {
    imageSmoothing(inA, radius, outB);
    benchmark::DoNotOptimize(outB);
  }
}

// Register the function as a benchmark
BENCHMARK(bench_partial_sum)->Unit(benchmark::kMicrosecond);

static void bench_scalar(benchmark::State &state) {
  InputVector inA;
  init(inA);

  OutputVector outB;
  zero(outB, (int)inA.size());

  for (auto _ : state) {
    imageSmoothing_scalar(inA, radius, outB);
    benchmark::DoNotOptimize(outB);
  }
}

BENCHMARK(bench_scalar)->Unit(benchmark::kMicrosecond);

// Run the benchmark
BENCHMARK_MAIN();
