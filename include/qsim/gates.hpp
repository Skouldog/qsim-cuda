#pragma once
#include <vector>
#include <complex>

namespace qsim
{

    void applySingleQubitGate(std::vector<std::complex<double>> &stateVector,
                              int qubit,
                              const std::complex<double> (&matrix)[2][2]);

    std::pair<std::size_t, std::size_t> getPairIndices(std::size_t pairNumber, int qubit);

}