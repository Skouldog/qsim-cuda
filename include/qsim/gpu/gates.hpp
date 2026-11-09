#pragma once

namespace qsim {
namespace gpu {
void startApplySingleQubitGate(qsim::VectorState& state, int qubit,
                               double* devPtr, const Matrix2& matrix);

void startApplyCnotGate(VectorState& state, double* devPtr, int controlBit,
                        int targetBit);

}  // namespace gpu
}  // namespace qsim
