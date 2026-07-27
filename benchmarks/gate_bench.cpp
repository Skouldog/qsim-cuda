#include <benchmark/benchmark.h>

#include "qsim/gates.hpp"
#include "qsim/state.hpp"

static void BM_applySingleQubitGate(benchmark::State& state) {
  const int64_t amplitudes = int64_t{1} << state.range(0);
  qsim::VectorState qState(state.range(0));
  const auto gate = qsim::gates::h();
  for (auto _ : state) {
    qsim::applySingleQubitGate(qState, 0, gate);
  }
  state.SetBytesProcessed(state.iterations() * 2 * amplitudes * 16);
  state.SetComplexityN(amplitudes);
  state.SetItemsProcessed(state.iterations() * amplitudes);
}
BENCHMARK(BM_applySingleQubitGate)
    ->DenseRange(1, 15)
    ->ArgName("qubits")
    ->Complexity();

static void BM_applySingleQubitGateTargetSweep(benchmark::State& state) {
  const int64_t amplitudes = int64_t{1} << state.range(0);
  const int target = static_cast<int>(state.range(1));
  qsim::VectorState qState(state.range(0));
  const auto gate = qsim::gates::h();
  for (auto _ : state) {
    qsim::applySingleQubitGate(qState, target, gate);
  }
  state.SetBytesProcessed(state.iterations() * 2 * amplitudes * 16);
  state.SetItemsProcessed(state.iterations() * amplitudes);
}

static constexpr int kQubitCounts[] = {6, 12, 24};

static void TargetSweepArgs(benchmark::internal::Benchmark* b) {
  for (int qubits : kQubitCounts) {
    for (int target = 0; target < qubits; target += 5) {
      b->Args({qubits, target});
    }
  }
}
BENCHMARK(BM_applySingleQubitGateTargetSweep)
    ->Apply(TargetSweepArgs)
    ->ArgNames({"qubits", "target"});

static void BM_applyCnotGate(benchmark::State& state) {
  const int64_t pairs = (int64_t{1} << state.range(0)) / 2;
  qsim::VectorState qState(state.range(0));
  for (auto _ : state) {
    qsim::applyCnotGate(qState, 0, 1);
  }
  state.SetBytesProcessed(state.iterations() * 2 * pairs * 16);
  state.SetComplexityN(pairs);
  state.SetItemsProcessed(state.iterations() * pairs);
}
BENCHMARK(BM_applyCnotGate)->DenseRange(2, 10)->ArgName("qubits")->Complexity();

static void BM_getPairIndices(benchmark::State& state) {
  const int qubit = static_cast<int>(state.range(0));
  const size_t pairs = (size_t{1} << state.range(1));
  for (auto _ : state) {
    for (size_t pair = 0; pair < pairs; pair++) {
      benchmark::DoNotOptimize(qsim::detail::getPairIndices(pair, qubit));
    }
  }

  state.SetItemsProcessed(state.iterations() * pairs);
}
BENCHMARK(BM_getPairIndices)
    ->ArgsProduct({{0, 12, 24}, {4, 8, 12, 16, 20, 24}})
    ->ArgNames({"qubit", "pair of qubit"});
