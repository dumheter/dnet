/*
 * Benchmark the performance of virtual function calls.
 */

// ============================================================ //
// Headers
// ============================================================ //

#include <chrono>
#include <iostream>
#include <functional>
#include <benchmark/benchmark.h>

// ============================================================ //

using mtype = long;

mtype fadd(mtype a, mtype b)
{
  return a + b;
}

class Direct
{
public:
  mtype add(mtype a, mtype b)
  {
    return a + b;
  }
};

// ============================================================ //

class mtypeerface
{
  virtual mtype add(mtype, mtype) = 0;
};

class Impl : public mtypeerface
{
public:
  mtype add(mtype a, mtype b) override
  {
    return a + b;
  }
};

// ============================================================ //

static void BM_virtual(benchmark::State& state)
{
  Impl impl{};

  for (auto _ : state) {
    benchmark::DoNotOptimize(impl.add(1, 2));
  }
}
BENCHMARK(BM_virtual);

static void BM_direct_class(benchmark::State& state)
{
  Direct direct{};

  for (auto _ : state) {
    benchmark::DoNotOptimize(direct.add(1, 2));
  }
}
BENCHMARK(BM_direct_class);

static void BM_direct(benchmark::State& state)
{
  for (auto _ : state) {
    benchmark::DoNotOptimize(fadd(1, 2));
  }
}
BENCHMARK(BM_direct);

static void BM_indirect(benchmark::State& state)
{
  std::function<int(int, int)> add_ptr = fadd;

  for (auto _ : state) {
    benchmark::DoNotOptimize(add_ptr(1, 2));
  }
}
BENCHMARK(BM_indirect);

static void BM_indirect_class_bind(benchmark::State& state)
{
  Direct direct{};
  using std::placeholders::_1;
  using std::placeholders::_2;
  std::function<int(int,int)> fun = std::bind(&Direct::add, direct, _1, _2);

  for (auto _ : state) {
    benchmark::DoNotOptimize(fun(1, 2));
  }
}
BENCHMARK(BM_indirect_class_bind);

static void BM_inline(benchmark::State& state)
{
  for (auto _ : state) {
    benchmark::DoNotOptimize(1 + 2);
  }
}
BENCHMARK(BM_inline);

// ============================================================ //

BENCHMARK_MAIN();
