#include "qsim/state.hpp"

#include <cmath>
#include <complex>
#include <cstddef>

#include "gtest/gtest.h"
#include "qsim/gates.hpp"
#include "test_util.hpp"

TEST(VectorState, ConstructsWithCorrectDimensions) {
  qsim::VectorState state(5);

  std::size_t stateSize = state.getSize();
  EXPECT_EQ(stateSize, (std::size_t{1} << 5));

  std::size_t stateQubits = state.getQubitSize();
  EXPECT_EQ(stateQubits, std::size_t{5});
}

TEST(VectorState, InitializesToGroundState) {
  qsim::VectorState state(5);

  double probIndex0 = state.getProbabilityOfIndex(0);
  double probIndex1 = state.getProbabilityOfIndex(1);
  EXPECT_EQ(probIndex0, 1.0);
  EXPECT_EQ(probIndex1, 0.0);
}

TEST(VectorState, ReturnsAmplitudeAtIndex) {
  qsim::VectorState state(5);

  std::complex<double> amp0 = state.getAmplitudeOfIndex(0);
  std::complex<double> amp1 = state.getAmplitudeOfIndex(1);
  EXPECT_EQ(amp0, 1.0);
  EXPECT_EQ(amp1, 0.0);
}

TEST(VectorState, StoresAmplitudeAtIndex) {
  qsim::VectorState state(5);

  state.setAmplitudeOfIndex(0, {0.5, 0});
  std::complex<double> amp0 = state.getAmplitudeOfIndex(0);
  EXPECT_NEAR(std::abs(amp0 - 0.5), 0.0, kTol);
}

TEST(VectorState, ComputesNorm) {
  qsim::VectorState state(2);

  state.setAmplitudeOfIndex(0, {0.5, 0});
  state.setAmplitudeOfIndex(1, {0.5, 0});
  state.setAmplitudeOfIndex(2, {0.5, 0});
  state.setAmplitudeOfIndex(3, {0.5, 0});

  double norm = state.getNorm();

  EXPECT_NEAR(std::abs(norm - 1.0), 0.0, kTol);
}

TEST(VectorState, RestoresNormToOne) {
  qsim::VectorState state(2);

  state.setAmplitudeOfIndex(0, {0.49, 0});
  state.setAmplitudeOfIndex(1, {0.49, 0});
  state.setAmplitudeOfIndex(2, {0.49, 0});
  state.setAmplitudeOfIndex(3, {0.49, 0});

  state.restoreNorm();
  double norm = state.getNorm();
  EXPECT_NEAR(std::abs(norm - 1.0), 0.0, kTol);
}

TEST(VectorState, SameSeedProducesSameSample) {
  qsim::VectorState state1(8);
  qsim::VectorState state2(8);

  double amp = 1.0 / 16.0;

  for (std::size_t i = 0; i < state1.getSize(); i++) {
    state1.setAmplitudeOfIndex(i, {amp, 0});
    state2.setAmplitudeOfIndex(i, {amp, 0});
  }

  std::size_t sample1 = state1.sample(42);
  std::size_t sample2 = state2.sample(42);
  EXPECT_EQ(sample1, sample2);
}

TEST(VectorState, SimulatesCircuitEndToEnd) {
  qsim::VectorState state(10);

  qsim::applySingleQubitGate(state, 9, qsim::gates::h());

  double norm = state.getNorm();
  EXPECT_NEAR(std::abs(norm - 1.0), 0.0, kTol);

  qsim::applySingleQubitGate(state, 2, qsim::gates::x());

  norm = state.getNorm();
  EXPECT_NEAR(std::abs(norm - 1.0), 0.0, kTol);

  state.restoreNorm();
  norm = state.getNorm();
  EXPECT_NEAR(std::abs(norm - 1.0), 0.0, kTol);

  std::size_t sample = state.sample(42);
  bool viableIndex = (sample == 4 || sample == 516);
  EXPECT_TRUE(viableIndex);
}

TEST(VectorState, SamplesFollowProbabilityDistribution) {
  qsim::VectorState state(1);
  double tol = 0.1;

  qsim::applySingleQubitGate(state, 0, qsim::gates::h());

  std::size_t runs = 2000;
  std::size_t zeros = 0;

  for (std::size_t run = 0; run < runs; run++) {
    if (state.sample() == 0) {
      zeros++;
    }
  }

  double zeroProb = (double)zeros / (double)runs;
  EXPECT_NEAR(std::abs(zeroProb - 0.5), 0.0, tol);
}

TEST(VectorState, SamplingConvergesWithinBinomialTolerance) {
  qsim::VectorState state(2);
  double tol = 0.0079;

  qsim::applySingleQubitGate(state, 0, qsim::gates::h());
  qsim::applyCnotGate(state, 0, 1);

  std::size_t runs = 100000;
  std::size_t zeros = 0;

  for (std::size_t run = 0; run < runs; run++) {
    if (state.sample() == 0) {
      zeros++;
    }
  }

  double zeroProb = (double)zeros / (double)runs;
  EXPECT_NEAR(std::abs(zeroProb - 0.5), 0.0, tol);
}
