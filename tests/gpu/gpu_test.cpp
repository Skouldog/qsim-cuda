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
