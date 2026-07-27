#include <benchmark/benchmark.h>

#include "qsim/circuit.hpp"
#include "qsim/gates.hpp"
#include "qsim/state.hpp"

static void BM_ghzState(benchmark::State& state) {
  const int qubits = static_cast<int>(state.range(0));
  
  qsim::VectorState qState(qubits);
  qsim::Circuit ghzCircuit;
  ghzCircuit.add(qsim::gates::h(), 0);
  for (int gates = 0; gates < qubits - 1; gates++) {
    ghzCircuit.addCnot(gates, gates + 1);
  }
  for (auto _ : state) {
    ghzCircuit.run(qState);
  }
  const size_t nAmps = size_t{1}<< qubits;
  state.SetBytesProcessed(state.iterations() * 16 * nAmps * (qubits + 1));
  state.SetComplexityN(qubits * nAmps);
  state.SetItemsProcessed(state.iterations() * qubits);
}
BENCHMARK(BM_ghzState)->DenseRange(2, 15)->ArgName("qubits")->Complexity();
