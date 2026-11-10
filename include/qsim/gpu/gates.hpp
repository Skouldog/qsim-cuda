#pragma once

namespace qsim {
namespace gpu {
void startApplySingleQubitGate(qsim::VectorState& state, int qubit,
                               std::complex<double>* devPtr,
                               const Matrix2& matrix);

void startApplyCnotGate(VectorState& state, std::complex<double>* devPtr,
                        int controlBit, int targetBit);
std::complex<double>* upload(VectorState& state);

void download(VectorState& state, std::complex<double>* devPtr);
}  // namespace gpu
}  // namespace qsim
