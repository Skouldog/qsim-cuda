#include "gtest/gtest.h"
#include "qsim/circuit.hpp"
#include "qsim/gates.hpp"
#include "qsim/gpu/gates.hpp"
#include "qsim/state.hpp"
#include "test_util.hpp"

TEST(HGate, CreatesSuperpositionOnGPU) {
  qsim::VectorState state(1);

  const double s = 1.0 / std::sqrt(2.0);

  std::complex<double>* devPtr = qsim::gpu::upload(state);

  qsim::gpu::startApplySingleQubitGate(state, 0, devPtr, qsim::gates::h());

  qsim::gpu::download(state, devPtr);

  std::vector<std::complex<double>> want = {{s, 0}, {s, 0}};
  compareStates(state, want, kTol);
}

TEST(HGate, HGateSuperpositionBigStateGPU) {
  const int qubits = 22;
  qsim::VectorState state(qubits);
  std::vector<std::complex<double>> want(std::size_t{1} << qubits);
  for (std::size_t amp = 0; amp < (std::size_t{1} << qubits); amp++) {
    want[amp] = {1.0 / std::sqrt(std::size_t{1} << qubits), 0};
  }

  std::complex<double>* devPtr = qsim::gpu::upload(state);
  for (int qubit = 0; qubit < qubits; qubit++) {
    qsim::gpu::startApplySingleQubitGate(state, qubit, devPtr,
                                         qsim::gates::h());
  }
  qsim::gpu::download(state, devPtr);

  compareStates(state, want, kTol);
}
