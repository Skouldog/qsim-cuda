#include "qsim/gates.hpp"

#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <vector>

#include "gtest/gtest.h"
#include "qsim/state.hpp"
#include "test_constants.hpp"

void compareStates(qsim::VectorState& got,
                   const std::vector<std::complex<double>>& want, double tol) {
  std::complex<double>* ptrStateVector = got.data();
  ASSERT_EQ(got.getSize(), want.size());

  for (std::size_t i = 0; i < got.getSize(); i++) {
    EXPECT_NEAR(std::abs(ptrStateVector[i] - want.at(i)), 0.0, tol);
  }
}

void comparePairs(const std::pair<std::size_t, size_t>& gotPair,
                  const std::pair<std::size_t, size_t>& wantPair) {
  EXPECT_EQ(gotPair.first, wantPair.first);
  EXPECT_EQ(gotPair.second, wantPair.second);
}

TEST(HGate, CreatesSuperposition) {
  qsim::VectorState state(1);

  const double s = 1.0 / std::sqrt(2.0);

  qsim::applySingleQubitGate(state, 0, qsim::gates::h());

  std::vector<std::complex<double>> want = {{s, 0}, {s, 0}};
  compareStates(state, want, kTol);
}
TEST(HGate, AppliedTwiceIsIdentity) {
  qsim::VectorState state(1);

  const double s = 1.0 / std::sqrt(2.0);

  qsim::applySingleQubitGate(state, 0, qsim::gates::h());
  qsim::applySingleQubitGate(state, 0, qsim::gates::h());

  std::vector<std::complex<double>> want = {{1, 0}, {0, 0}};
  compareStates(state, want, kTol);
}

TEST(XGate, SwapsQubitZeroAndBack) {
  qsim::VectorState state(3);

  qsim::applySingleQubitGate(state, 0, qsim::gates::x());
  std::vector<std::complex<double>> want = {{0, 0}, {1, 0}, {0, 0}, {0, 0},
                                            {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, kTol);

  qsim::applySingleQubitGate(state, 0, qsim::gates::x());
  want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, kTol);
}
TEST(XGate, SwapsQubitOneAndBack) {
  qsim::VectorState state(3);

  qsim::applySingleQubitGate(state, 1, qsim::gates::x());
  std::vector<std::complex<double>> want = {{0, 0}, {0, 0}, {1, 0}, {0, 0},
                                            {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, kTol);

  qsim::applySingleQubitGate(state, 1, qsim::gates::x());
  want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, kTol);
}
TEST(XGate, SwapsQubitTwoAndBack) {
  qsim::VectorState state(3);
  qsim::applySingleQubitGate(state, 2, qsim::gates::x());
  std::vector<std::complex<double>> want = {{0, 0}, {0, 0}, {0, 0}, {0, 0},
                                            {1, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, kTol);

  qsim::applySingleQubitGate(state, 2, qsim::gates::x());
  want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, kTol);
}

TEST(ZGate, NegatesAmplitudeOfStateOne) {
  qsim::VectorState state(2);
  state.setAmplitudeOfIndex(0, {0.0, 0.0});
  state.setAmplitudeOfIndex(1, {1, 0});

  qsim::applySingleQubitGate(state, 0, qsim::gates::z());
  std::vector<std::complex<double>> want = {{0, 0}, {-1, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, kTol);
}

TEST(SGate, MultipliesStateOneByI) {
  qsim::VectorState state(2);
  state.setAmplitudeOfIndex(0, {0.0, 0.0});
  state.setAmplitudeOfIndex(1, {1, 0});

  qsim::applySingleQubitGate(state, 0, qsim::gates::s());
  std::vector<std::complex<double>> want = {{0, 0}, {0, 1}, {0, 0}, {0, 0}};
  compareStates(state, want, kTol);
}

TEST(TGate, FullRotationOnQubitOne) {
  qsim::VectorState state(2);

  state.setAmplitudeOfIndex(0, {0.0, 0.5});
  state.setAmplitudeOfIndex(1, {0.5, 0});
  state.setAmplitudeOfIndex(2, {0.5, 0});
  state.setAmplitudeOfIndex(3, {0.5, 0});

  for (int run = 0; run < 8; run++) {
    qsim::applySingleQubitGate(state, 1, qsim::gates::t());
  }

  std::vector<std::complex<double>> want = {
      {0.0, 0.5}, {0.5, 0}, {0.5, 0}, {0.5, 0}};
  compareStates(state, want, kTol);
}
TEST(TGate, HalfRotationOnQubitZero) {
  qsim::VectorState state(2);

  state.setAmplitudeOfIndex(0, {0.0, 0.5});
  state.setAmplitudeOfIndex(1, {0.5, 0});
  state.setAmplitudeOfIndex(2, {0.5, 0});
  state.setAmplitudeOfIndex(3, {0.5, 0});

  for (int run = 0; run < 4; run++) {
    qsim::applySingleQubitGate(state, 0, qsim::gates::t());
  }

  std::vector<std::complex<double>> want = {
      {0.0, 0.5}, {-0.5, 0}, {0.5, 0}, {-0.5, 0}};
  compareStates(state, want, kTol);
}

TEST(RzGate, FullRotationOnQubitOne) {
  qsim::VectorState state(2);
  double angle = std::numbers::pi * 4;
  std::vector<std::complex<double>> want = {{1, 0.0}, {0, 0}, {0.0, 0}, {0, 0}};
  qsim::applySingleQubitGate(state, 1, qsim::gates::rz(angle));
  compareStates(state, want, kTol);
}
TEST(RzGate, TwoHalfRotationsOnQubitOne) {
  qsim::VectorState state(2);
  double angle = std::numbers::pi * 4;
  std::vector<std::complex<double>> want = {
      {-1, 0.0}, {0, 0}, {0.0, 0}, {0, 0}};
  qsim::applySingleQubitGate(state, 1, qsim::gates::rz(angle / 2));
  compareStates(state, want, kTol);

  state.setAmplitudeOfIndex(1, {1, 0});
  want = {{1, 0.0}, {-1, 0}, {0.0, 0}, {0, 0}};
  qsim::applySingleQubitGate(state, 1, qsim::gates::rz(angle / 2));
  compareStates(state, want, kTol);
}

TEST(CnotGate, AppliedTwiceIsIdentity) {
  qsim::VectorState state(3);

  state.setAmplitudeOfIndex(0, {0.5, 0});
  state.setAmplitudeOfIndex(1, {0.5, 0});
  state.setAmplitudeOfIndex(2, {0.5, 0});
  state.setAmplitudeOfIndex(3, {0.5, 0});

  qsim::applyCnotGate(state, 0, 2);
  std::vector<std::complex<double>> want = {
      {0.5, 0}, {0, 0}, {0.5, 0}, {0, 0}, {0, 0}, {0.5, 0}, {0, 0}, {0.5, 0}};
  compareStates(state, want, kTol);

  qsim::applyCnotGate(state, 0, 2);

  want = {{0.5, 0}, {0.5, 0}, {0.5, 0}, {0.5, 0},
          {0, 0},   {0, 0},   {0, 0},   {0, 0}};
  compareStates(state, want, kTol);
}

TEST(GetPairIndices, FirstPair) {
  std::pair<std::size_t, size_t> gotPair;
  std::pair<std::size_t, size_t> wantPair;

  gotPair = qsim::getPairIndices(0, 0);
  wantPair = {0, 1};
  comparePairs(gotPair, wantPair);
}
TEST(GetPairIndices, BorderPair) {
  std::pair<std::size_t, size_t> gotPair;
  std::pair<std::size_t, size_t> wantPair;

  gotPair = qsim::getPairIndices(0, 34);
  wantPair = {0, std::size_t{1} << 34};
  comparePairs(gotPair, wantPair);
}
TEST(GetPairIndices, InTheMiddle) {
  std::pair<std::size_t, size_t> gotPair;
  std::pair<std::size_t, size_t> wantPair;

  gotPair = qsim::getPairIndices(7, 2);
  wantPair = {11, 15};
  comparePairs(gotPair, wantPair);
}
