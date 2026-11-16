#pragma once

#include <complex>

#include "qsim/gates.hpp"
#include "qsim/state.hpp"

namespace qsim {
namespace gpu {

constexpr int kThreads = 256;

void startApplySingleQubitGate(qsim::VectorState& state, int qubit,
                               std::complex<double>* devPtr,
                               const Matrix2& matrix);

void startApplyCnotGate(VectorState& state, std::complex<double>* devPtr,
                        int controlBit, int targetBit);
std::complex<double>* upload(VectorState& state);

void download(VectorState& state, std::complex<double>* devPtr);
}  // namespace gpu
}  // namespace qsim
