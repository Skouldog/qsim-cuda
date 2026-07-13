#pragma once
#include <complex>
#include <cstddef>
#include <utility>
#include <array>

#include "qsim/state.hpp"

namespace qsim {
using Matrix2 = std::array<std::array<std::complex<double>, 2>, 2>;

void applySingleQubitGate(qsim::VectorState& state, int qubit,
                          const Matrix2& matrix);

void applyCnotGate(qsim::VectorState& state, int controlBit, int targetBit);

std::pair<std::size_t, std::size_t> getPairIndices(std::size_t pairNumber,
                                                   int qubit);

Matrix2 makeMatrixH();
Matrix2 makeMatrixX();

}  // namespace qsim
