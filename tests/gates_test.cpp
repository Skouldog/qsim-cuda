#include "qsim/gates.hpp"

#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <string>
#include <vector>

#include "qsim/state.hpp"
#include "test_util.hpp"

void compareStates(qsim::VectorState& got,
                   const std::vector<std::complex<double>>& want, double tol,
                   const std::string& msg) {
  std::complex<double>* ptrStateVector = got.data();
  expectTrue(got.getSize() == want.size(), msg + " (size mismatch)");

  for (std::size_t i = 0; i < got.getSize(); i++) {
    expectClose(ptrStateVector[i], want.at(i), tol, msg);
  }
}

void comparePairs(const std::pair<std::size_t, size_t>& gotPair,
                  const std::pair<std::size_t, size_t>& wantPair,
                  const std::string& msg) {
  expectTrue(gotPair.first == wantPair.first, msg + " :Pair.first: ");
  expectTrue(gotPair.second == wantPair.second, msg + " :Pair.second: ");
}

void runHGateTest() {
  const double cTol = 1e-9;
  qsim::VectorState state(1);

  const double s = 1.0 / std::sqrt(2.0);

  qsim::applySingleQubitGate(state, 0, qsim::gates::h());

  std::vector<std::complex<double>> want = {{s, 0}, {s, 0}};
  compareStates(state, want, cTol, "H on q0");

  qsim::applySingleQubitGate(state, 0, qsim::gates::h());

  want = {{1, 0}, {0, 0}};
  compareStates(state, want, cTol, "H·H=I on q0");
}

void runXGateTest() {
  const double cTol = 1e-9;
  qsim::VectorState state(3);

  // Qubit 0
  qsim::applySingleQubitGate(state, 0, qsim::gates::x());
  std::vector<std::complex<double>> want = {{0, 0}, {1, 0}, {0, 0}, {0, 0},
                                            {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, cTol, "X on q0 -> index 1");

  qsim::applySingleQubitGate(state, 0, qsim::gates::x());
  want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, cTol, "X*X on q0 -> index 0");

  // Qubit 1
  qsim::applySingleQubitGate(state, 1, qsim::gates::x());
  want = {{0, 0}, {0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, cTol, "X on q1 -> index 2");

  qsim::applySingleQubitGate(state, 1, qsim::gates::x());
  want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, cTol, "X*X on q1 -> index 0");

  // Qubit 2
  qsim::applySingleQubitGate(state, 2, qsim::gates::x());
  want = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, cTol, "X on q2 -> index 4");

  qsim::applySingleQubitGate(state, 2, qsim::gates::x());
  want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, cTol, "X*X on q2 -> index 0");
}

void runZGateTest() {
  qsim::VectorState state(2);
  double cTol = 1e-9;
  state.setAmplitudeOfIndex(0, {0.0, 0.0});
  state.setAmplitudeOfIndex(1, {1, 0});

  qsim::applySingleQubitGate(state, 0, qsim::gates::z());
  std::vector<std::complex<double>> want = {{0, 0}, {-1, 0}, {0, 0}, {0, 0}};
  compareStates(state, want, cTol, "Z on q0 -> index 1 = -1");
}

void runSGateTest() {
  qsim::VectorState state(2);
  double cTol = 1e-9;
  state.setAmplitudeOfIndex(0, {0.0, 0.0});
  state.setAmplitudeOfIndex(1, {1, 0});

  qsim::applySingleQubitGate(state, 0, qsim::gates::s());
  std::vector<std::complex<double>> want = {{0, 0}, {0, 1}, {0, 0}, {0, 0}};
  compareStates(state, want, cTol, "S on q0 -> index 1=i");
}

void runTGateTest() {
  qsim::VectorState state(2);
  double cTol = 1e-9;

  state.setAmplitudeOfIndex(0, {0.0, 0.5});
  state.setAmplitudeOfIndex(1, {0.5, 0});
  state.setAmplitudeOfIndex(2, {0.5, 0});
  state.setAmplitudeOfIndex(3, {0.5, 0});

  for (int run = 0; run < 8; run++) {
    qsim::applySingleQubitGate(state, 1, qsim::gates::t());
  }

  std::vector<std::complex<double>> want = {
      {0.0, 0.5}, {0.5, 0}, {0.5, 0}, {0.5, 0}};
  compareStates(state, want, cTol, "8xT on q1 ->  360° Spin state == state");

  for (int run = 0; run < 4; run++) {
    qsim::applySingleQubitGate(state, 0, qsim::gates::t());
  }

  want = {{0.0, 0.5}, {-0.5, 0}, {0.5, 0}, {-0.5, 0}};
  compareStates(state, want, cTol, "4xT on q0 ->  180° Spin ");
}

void runRZGateTest() {
  qsim::VectorState state(2);
  double cTol = 1e-9;
  double angle = std::numbers::pi * 4;
  std::vector<std::complex<double>> want = {{1, 0.0}, {0, 0}, {0.0, 0}, {0, 0}};
  qsim::applySingleQubitGate(state, 1, qsim::gates::rz(angle));
  compareStates(state, want, cTol, "RZ Identity (4Pi) on q1 ");

  want = {{-1, 0.0}, {0, 0}, {0.0, 0}, {0, 0}};
  qsim::applySingleQubitGate(state, 1, qsim::gates::rz(angle / 2));
  compareStates(state, want, cTol, "RZ (2Pi) on q1 ");

  state.setAmplitudeOfIndex(1, {1, 0});
  want = {{1, 0.0}, {-1, 0}, {0.0, 0}, {0, 0}};
  qsim::applySingleQubitGate(state, 1, qsim::gates::rz(angle / 2));
  compareStates(state, want, cTol, "RZ (2Pi) on q1 ");
}

void runCnotGateTests() {
  qsim::VectorState state(3);

  double cTol = 1e-19;

  state.setAmplitudeOfIndex(0, {0.5, 0});
  state.setAmplitudeOfIndex(1, {0.5, 0});
  state.setAmplitudeOfIndex(2, {0.5, 0});
  state.setAmplitudeOfIndex(3, {0.5, 0});

  qsim::applyCnotGate(state, 0, 2);
  std::vector<std::complex<double>> want = {
      {0.5, 0}, {0, 0}, {0.5, 0}, {0, 0}, {0, 0}, {0.5, 0}, {0, 0}, {0.5, 0}};
  compareStates(state, want, cTol, "CNOT(C0,C2)");

  qsim::applyCnotGate(state, 0, 2);

  want = {{0.5, 0}, {0.5, 0}, {0.5, 0}, {0.5, 0},
          {0, 0},   {0, 0},   {0, 0},   {0, 0}};
  compareStates(state, want, cTol, "CNOT*CNOT(C0,C2)= I");
}

void runGetPairIndicesTest() {
  std::pair<std::size_t, size_t> gotPair;
  std::pair<std::size_t, size_t> wantPair;

  gotPair = qsim::getPairIndices(0, 0);
  wantPair = {0, 1};
  comparePairs(gotPair, wantPair, "Pair on Ind 0 with q0 > 0,1");

  gotPair = qsim::getPairIndices(0, 34);
  wantPair = {0, std::size_t{1} << 34};
  comparePairs(gotPair, wantPair, "Pair on Ind 0 with q34 > 0,2^34");

  gotPair = qsim::getPairIndices(7, 2);
  wantPair = {11, 15};
  comparePairs(gotPair, wantPair, "Pair on Ind 7 with q2 > 11,15");
}

void runGatesTest() {
  runHGateTest();
  runXGateTest();
  runZGateTest();
  runSGateTest();
  runTGateTest();
  runRZGateTest();
  runCnotGateTests();
  runGetPairIndicesTest();
}
