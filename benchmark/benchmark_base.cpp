#include <benchmark/benchmark.h>

#include <cmath>

static void BM_SqrtA(benchmark::State& state) {
    double x = 1.0;
    for (auto _ : state) {
        x = std::sqrt(x + 1.0);
        benchmark::DoNotOptimize(x);
    }
}
BENCHMARK(BM_SqrtA);

static void BM_SqrtB(benchmark::State& state) {
    double x = 1.0;
    for (auto _ : state) {
        x = std::pow(x + 1.0, 0.5);  // 用 pow 模拟“另一种实现”
        benchmark::DoNotOptimize(x);
    }
}
BENCHMARK(BM_SqrtB);

BENCHMARK_MAIN();