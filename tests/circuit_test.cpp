#include "qsim/circuit.hpp"

#include <complex>
#include <cstddef>
#include <vector>

#include "gtest/gtest.h"
#include "qsim/gates.hpp"
#include "qsim/operation.hpp"
#include "qsim/state.hpp"
#include "test_util.hpp"

TEST(Circuit, CreatesUniformDistribution) {
  qsim::VectorState state(2);

  qsim::Circuit circuit;

  circuit.add(qsim::gates::h(), 0);
  circuit.add(qsim::gates::h(), 1);

  circuit.run(state);

  const double s = 1.0 / std::sqrt(4.0);

  std::vector<std::complex<double>> want = {{s, 0}, {s, 0}, {s, 0}, {s, 0}};
  compareStates(state, want, kTol);
}

TEST(Circuit, CreatesBellState) {
  qsim::VectorState state(2);

  qsim::Circuit circuit;

  circuit.add(qsim::gates::h(), 0);
  circuit.addCnot(0, 1);

  circuit.run(state);

  const double s = 1.0 / std::sqrt(2.0);

  std::vector<std::complex<double>> want = {{s, 0}, {0, 0}, {0, 0}, {s, 0}};
  compareStates(state, want, kTol);
}

TEST(Circuit, CreatesGHZStateForNQubits) {
  const int qubits = 10;

  qsim::VectorState state(qubits);

  qsim::Circuit circuit;

  circuit.add(qsim::gates::h(), 0);
  for (int i = 0; i < qubits - 1; i++) {
    circuit.addCnot(i, i + 1);
  }

  circuit.run(state);

  const double s = 1.0 / std::sqrt(2.0);

  std::size_t indices = std::size_t{1} << qubits;
  std::vector<std::complex<double>> want(indices);
  want.at(0) = {s, 0};
  want.at(indices - 1) = {s, 0};
  compareStates(state, want, kTol);
}
