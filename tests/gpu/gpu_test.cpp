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

TEST(GpuCircuit, MatchesCpuForMixedSingleQubitGates) {
  qsim::VectorState state(20);
  qsim::VectorState stateGpu(20);

  qsim::Circuit circuit;

  circuit.add(qsim::gates::h(), 0);
  circuit.add(qsim::gates::s(), 0);

  circuit.add(qsim::gates::x(), 10);

  circuit.add(qsim::gates::h(), 19);
  circuit.add(qsim::gates::rz(0.3), 19);

  circuit.add(qsim::gates::h(), 5);
  circuit.add(qsim::gates::t(), 5);
  circuit.add(qsim::gates::h(), 5);

  circuit.run(state);

  circuit.runGpu(stateGpu);

  compareStates(stateGpu, state, kTol);
}

TEST(GpuCnotGate, MixesIndices) {
  qsim::VectorState state(3);
  qsim::Circuit circuit;

  state.setAmplitudeOfIndex(0, {0.5, 0});
  state.setAmplitudeOfIndex(1, {0.5, 0});
  state.setAmplitudeOfIndex(2, {0.5, 0});
  state.setAmplitudeOfIndex(3, {0.5, 0});

  circuit.addCnot(0, 2);
  circuit.runGpu(state);

  std::vector<std::complex<double>> want = {
      {0.5, 0}, {0, 0}, {0.5, 0}, {0, 0}, {0, 0}, {0.5, 0}, {0, 0}, {0.5, 0}};
  compareStates(state, want, kTol);
}

TEST(GpuCircuit, CreatesGhzState) {
  const int qubits = 20;

  qsim::VectorState state(qubits);

  qsim::Circuit circuit;

  circuit.add(qsim::gates::h(), 0);
  for (int i = 0; i < qubits - 1; i++) {
    circuit.addCnot(i, i + 1);
  }

  circuit.runGpu(state);

  const double s = 1.0 / std::sqrt(2.0);

  std::size_t indices = std::size_t{1} << qubits;
  std::vector<std::complex<double>> want(indices);
  want.at(0) = {s, 0};
  want.at(indices - 1) = {s, 0};
  compareStates(state, want, kTol);
}
