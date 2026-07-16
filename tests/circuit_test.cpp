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

  qsim::Circuit entangleCircuit;

  entangleCircuit.add(qsim::gates::h(), 0);
  entangleCircuit.add(qsim::gates::h(), 1);

  entangleCircuit.run(state);

  const double s = 1.0 / std::sqrt(4.0);

  std::vector<std::complex<double>> want = {{s, 0}, {s, 0}, {s, 0}, {s, 0}};
  compareStates(state, want, kTol);
}

TEST(Circuit, CreatesBellState) {
  qsim::VectorState state(2);

  qsim::Circuit entangleCircuit;

  entangleCircuit.add(qsim::gates::h(), 0);
  entangleCircuit.addCnot(0, 1);

  entangleCircuit.run(state);

  const double s = 1.0 / std::sqrt(2.0);

  std::vector<std::complex<double>> want = {{s, 0}, {0, 0}, {0, 0}, {s, 0}};
  compareStates(state, want, kTol);
}
