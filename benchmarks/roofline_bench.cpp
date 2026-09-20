#include <benchmark/benchmark.h>

#include <complex>

#include "qsim/state.hpp"

static void BM_roofline(benchmark::State& state) {
  const int qubits = static_cast<int>(state.range(0));

  qsim::VectorState qState(qubits);
  std::complex<double>* amps = qState.data();
  const double factor = 1.001;
  const size_t stateSize = qState.getSize();

  for (auto _ : state) {
#pragma omp parallel for
    for (size_t index = 0; index < stateSize; index++) {
      amps[index] = amps[index] * factor;
    }
  }
  const size_t nAmps = size_t{1} << qubits;
  state.SetBytesProcessed(state.iterations() * 32 * nAmps);
  state.SetComplexityN(nAmps);
  state.SetItemsProcessed(state.iterations() * nAmps);
}
BENCHMARK(BM_roofline)->DenseRange(2, 24)->ArgName("qubits")->Complexity();
