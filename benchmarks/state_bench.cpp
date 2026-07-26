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
    ->DenseRange(1, 24)
    ->ArgName("qubits")
    ->Complexity();
