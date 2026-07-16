#pragma once

#include <complex>
#include <vector>

#include "qsim/state.hpp"

constexpr double kTol = 1e-9;

void compareStates(qsim::VectorState& got,
                   const std::vector<std::complex<double>>& want, double tol);
