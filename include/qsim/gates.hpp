#pragma once
#include <complex>
#include <cstddef>
#include <utility>
#include "qsim/state.hpp"

namespace qsim
{

    void applySingleQubitGate(qsim::VectorState& stateVector,
                              int qubit,
                              const std::complex<double> (&matrix)[2][2]);

    std::pair<std::size_t, std::size_t> getPairIndices(std::size_t pairNumber, int qubit);

}
