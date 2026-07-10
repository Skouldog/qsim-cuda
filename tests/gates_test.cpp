#include "qsim/gates.hpp"

#include <cmath>
#include <complex>
#include <cstddef>
#include <string>
#include <vector>

#include "test_util.hpp"

void compareStates(qsim::VectorState& got,
                   const std::vector<std::complex<double>>& want, double tol,
                   const std::string& msg) {
  std::complex<double>* ptrStateVector = got.data();
  expectTrue(got.getSize() == want.size(),
             msg + " (size mismatch)");

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

void runGateTests() {
  const double cTol = 1e-9;
  qsim::VectorState stateOneQubit(1);
  qsim::VectorState stateThreeQubit(3);

  /*
   * H Gate Tests
   */
  const double s = 1.0 / std::sqrt(2.0);
  std::complex<double> matrixH[2][2] = {{{s, 0}, {s, 0}}, {{s, 0}, {-s, 0}}};

  qsim::applySingleQubitGate(stateOneQubit, 0, matrixH);

  std::vector<std::complex<double>> want = {{s, 0}, {s, 0}};
  compareStates(stateOneQubit, want, cTol, "H on q0");

  qsim::applySingleQubitGate(stateOneQubit, 0, matrixH);

  want = {{1, 0}, {0, 0}};
  compareStates(stateOneQubit, want, cTol, "H·H=I on q0");

  /*
   * X Gate Tests
   */
  std::complex<double> matrixX[2][2] = {{{0, 0}, {1, 0}}, {{1, 0}, {0, 0}}};

  // Qubit 0
  qsim::applySingleQubitGate(stateThreeQubit, 0, matrixX);
  want = {{0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(stateThreeQubit, want, cTol, "X on q0 -> index 1");

  qsim::applySingleQubitGate(stateThreeQubit, 0, matrixX);
  want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(stateThreeQubit, want, cTol, "X*X on q0 -> index 0");

  // Qubit 1
  qsim::applySingleQubitGate(stateThreeQubit, 1, matrixX);
  want = {{0, 0}, {0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(stateThreeQubit, want, cTol, "X on q1 -> index 2");

  qsim::applySingleQubitGate(stateThreeQubit, 1, matrixX);
  want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(stateThreeQubit, want, cTol, "X*X on q1 -> index 0");

  // Qubit 2
  qsim::applySingleQubitGate(stateThreeQubit, 2, matrixX);
  want = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(stateThreeQubit, want, cTol, "X on q2 -> index 4");

  qsim::applySingleQubitGate(stateThreeQubit, 2, matrixX);
  want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  compareStates(stateThreeQubit, want, cTol, "X*X on q2 -> index 0");
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
