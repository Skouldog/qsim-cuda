#pragma once
#include <array>
#include <complex>
#include <cstddef>
#include <utility>

#include "qsim/state.hpp"

namespace qsim {
using Matrix2 = std::array<std::array<std::complex<double>, 2>, 2>;

void applySingleQubitGate(qsim::VectorState& state, int qubit,
                          const Matrix2& matrix);

void applyCnotGate(qsim::VectorState& state, int controlBit, int targetBit);

std::pair<std::size_t, std::size_t> getPairIndices(std::size_t pairNumber,
                                                   int qubit);

namespace gates {
Matrix2 h();
Matrix2 x();
Matrix2 s();
Matrix2 z();
Matrix2 t();
Matrix2 rz(double angle);
}  // namespace gates
}  // namespace qsim
