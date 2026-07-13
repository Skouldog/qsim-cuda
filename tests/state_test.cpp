#include "qsim/state.hpp"

#include <array>
#include <complex>
#include <cstddef>
#include <vector>

#include "qsim/gates.hpp"
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

  double amp = 1.0 / 16.0;

  for (std::size_t i = 0; i < state1.getSize(); i++) {
    state1.setAmplitudeOfIndex(i, {amp, 0});
    state2.setAmplitudeOfIndex(i, {amp, 0});
  }

  std::size_t sample1 = state1.sample(42);
  std::size_t sample2 = state2.sample(42);
  expectTrue(sample1 == sample2, "Same Seed > Same Sample");

  sample1 = state1.sample(1);
  sample2 = state2.sample(25);
  expectTrue(sample1 != sample2, "Different Seed > Different Sample");
}

void workflowTest() {
  double tol = 1e-9;

  qsim::VectorState state(10);

  qsim::applySingleQubitGate(state, 9, qsim::makeMatrixH());

  double norm = state.getNorm();
  expectClose(norm, 1, tol, " after H on q9");

  qsim::applySingleQubitGate(state, 2, qsim::makeMatrixX());

  norm = state.getNorm();
  expectClose(norm, 1, tol, " after X on q9");

  state.restoreNorm();
  norm = state.getNorm();
  expectClose(norm, 1, tol, "Restored Norm -> 1");

  std::size_t sample = state.sample(42);
  bool viableIndex = (sample == 4 || sample == 516);
  expectTrue(viableIndex, "Sample Index 4 or 516");
}

void randomnessTest() {
  qsim::VectorState state(1);
  double tol = 0.1;

  qsim::applySingleQubitGate(state, 0, qsim::makeMatrixH());

  std::size_t runs = 2000;
  std::size_t zeros = 0;

  for (std::size_t run = 0; run < runs; run++) {
    if (state.sample() == 0) {
      zeros++;
    }
  }

  double zeroProb = (double)zeros / (double)runs;

  expectClose(zeroProb, 0.5, tol, "Randomness 50/50");
}

void runStateTests() {
  vectorInitTest();
  getProbabilityOfIndexTest();
  getAmplitudeOfIndexTest();
  setAmplitudeOfIndexTest();
  getNormTest();
  restoreNormTest();
  sampleTest();
  workflowTest();
  randomnessTest();
}
