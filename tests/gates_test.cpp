#include<cmath>
#include<vector>
#include<complex>
#include<string>
#include "qsim/gates.hpp"
#include "test_util.hpp"

void compareStates(const std::vector<std::complex<double>> &state, const std::vector<std::complex<double>>& wantVec, double tol, const std::string &msg)
{
    expectTrue(state.size() == wantVec.size(), msg + " (size mismatch)");

    for (std::size_t i = 0; i < state.size(); i++)
    {
        expectClose(state.at(i), wantVec.at(i), tol, msg);
    }
  
}

void runGateTests()
{
    const double cTol = 1e-9;
    std::vector<std::complex<double>> stateOneQubit = {{1, 0}, {0, 0}};
    std::vector<std::complex<double>> stateThreeQubit = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

    /*
     * H Gate Tests
     */
    const double s = 1.0 / std::sqrt(2.0);
    std::complex<double> matrixH[2][2] = {
        {{s, 0}, {s, 0}},
        {{s, 0}, {-s, 0}}};

    qsim::applySingleQubitGate(stateOneQubit, 0, matrixH);

    std::vector<std::complex<double>> want = {{s, 0}, {s, 0}};
    compareStates(stateOneQubit, want, cTol, "H on q0");

    qsim::applySingleQubitGate(stateOneQubit, 0, matrixH);

    want = {{1, 0}, {0, 0}};
    compareStates(stateOneQubit, want, cTol, "H·H=I on q0");

    /*
     * X Gate Tests
     */
    std::complex<double> matrixX[2][2] = {
        {{0, 0}, {1, 0}},
        {{1, 0}, {0, 0}}};


    //Qubit 0
    qsim::applySingleQubitGate(stateThreeQubit, 0, matrixX);
    want = {{0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
    compareStates(stateThreeQubit, want, cTol, "X on q0 -> index 1");

    qsim::applySingleQubitGate(stateThreeQubit, 0, matrixX);
    want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
    compareStates(stateThreeQubit, want, cTol, "X*X on q0 -> index 0");


    //Qubit 1
    qsim::applySingleQubitGate(stateThreeQubit, 1, matrixX);
    want = {{0, 0}, {0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
    compareStates(stateThreeQubit, want, cTol, "X on q1 -> index 2");

    qsim::applySingleQubitGate(stateThreeQubit, 1, matrixX);
    want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
    compareStates(stateThreeQubit, want, cTol, "X*X on q1 -> index 0");

    //Qubit 2
    qsim::applySingleQubitGate(stateThreeQubit, 2, matrixX);
    want = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}};
    compareStates(stateThreeQubit, want, cTol, "X on q2 -> index 4");

    qsim::applySingleQubitGate(stateThreeQubit, 2, matrixX);
    want = {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
    compareStates(stateThreeQubit, want, cTol, "X*X on q2 -> index 0");
}