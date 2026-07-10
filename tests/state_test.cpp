#include "qsim/state.hpp"

#include <complex>
#include <cstddef>
#include <vector>

#include "test_util.hpp"

void vectorInitTest() {
  qsim::VectorState state(5);

  std::size_t stateSize = state.getSize();
  expectTrue(stateSize == (std::size_t{1} << 5), "Init Vector Size == 2^5");

  std::size_t stateQubits = state.getQubitSize();

  expectTrue(stateQubits == 5, "Init Vector with 5q = 5qubits");
}

void getProbabilityOfIndexTest() {
  qsim::VectorState state(5);

  double probIndex0 = state.getProbabilityOfIndex(0);
  double probIndex1 = state.getProbabilityOfIndex(1);
  expectTrue(probIndex0 == 1, "Index 100% Probability");
  expectTrue(probIndex1 == 0, "Index 0% Probability");
}

void getAmplitudeOfIndexTest() {
  double tol = 1e-9;

  qsim::VectorState state(5);

  std::complex<double> amp0 = state.getAmplitudeOfIndex(0);
  std::complex<double> amp1 = state.getAmplitudeOfIndex(1);
  expectClose(amp0, 1, tol, "Index 0 -> Amp = 1");
  expectClose(amp1, 0, tol, "Index 1 -> Amp = 0");
}

void setAmplitudeOfIndexTest() {
  qsim::VectorState state(5);
  double tol = 1e-9;

  state.setAmplitudeOfIndex(0, {0.5, 0});
  std::complex<double> amp0 = state.getAmplitudeOfIndex(0);

  expectClose(amp0, 0.5, tol, "Index 0 -> Amp = 0.5");
}

void getNormTest() {
  qsim::VectorState state(2);

  double tol = 1e-9;

  state.setAmplitudeOfIndex(0, {0.5, 0});
  state.setAmplitudeOfIndex(1, {0.5, 0});
  state.setAmplitudeOfIndex(2, {0.5, 0});
  state.setAmplitudeOfIndex(3, {0.5, 0});

  double norm = state.getNorm();

  expectClose(norm, 1, tol, "Normal Norm -> 1");
}

void restoreNormTest() {
  qsim::VectorState state(2);

  double tol = 1e-9;

  state.setAmplitudeOfIndex(0, {0.49, 0});
  state.setAmplitudeOfIndex(1, {0.49, 0});
  state.setAmplitudeOfIndex(2, {0.49, 0});
  state.setAmplitudeOfIndex(3, {0.49, 0});

  state.restoreNorm();
  double norm = state.getNorm();
  expectClose(norm, 1, tol, "Restored Norm -> 1");
}

void sampleTest() {
  double tol = 1e-9;
  qsim::VectorState state1(8);
  qsim::VectorState state2(8);

  double ampProb = 1.0 / 16.0;

  for (std::size_t i = 0; i < state1.getSize(); i++) {
    state1.setAmplitudeOfIndex(i, {ampProb, 0});
    state2.setAmplitudeOfIndex(i, {ampProb, 0});
  }

  std::size_t sample1 = state1.sample(42);
  std::size_t sample2 = state2.sample(42);
  expectTrue(sample1 == sample2, "Same Seed > Same Sample");

  sample1 = state1.sample(1);
  sample2 = state2.sample(25);
  expectTrue(sample1 != sample2, "Different Seed > Different Sample");
}

void runStateTests() {
  vectorInitTest();
  getProbabilityOfIndexTest();
  getAmplitudeOfIndexTest();
  setAmplitudeOfIndexTest();
  getNormTest();
  restoreNormTest();
  sampleTest();
}
