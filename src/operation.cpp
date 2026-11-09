#include "qsim/operation.hpp"

#include "qsim/gates.hpp"
#include "qsim/gpu/gates.hpp"

namespace qsim {  // namespace qsim

void SingleQubitGate::apply(VectorState& state) const {
  qsim::applySingleQubitGate(state, qubit, matrix);
}

void CnotGate::apply(VectorState& state) const {
  qsim::applyCnotGate(state, controlBit, targetBit);
}
#ifdef QSIM_USE_CUDA
void SingleQubitGate::applyGpu(VectorState& state, double* devPtr) const {
  qsim::gpu::startApplySingleQubitGate(state, qubit, devPtr, matrix);
}

void CnotGate::applyGpu(VectorState& state, double* devPtr) const {
  qsim::gpu::startApplyCnotGate(state, devPtr, controlBit, targetBit);
}
#endif
}  // namespace qsim
