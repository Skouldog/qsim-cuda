#include "qsim/operation.hpp"

#include "qsim/gates.hpp"
#ifdef QSIM_USE_CUDA
#include "qsim/gpu/device_buffer.hpp"
#include "qsim/gpu/gates.hpp"
#endif

namespace qsim {

void SingleQubitGate::apply(VectorState& state) const {
  qsim::applySingleQubitGate(state, qubit, matrix);
}

void CnotGate::apply(VectorState& state) const {
  qsim::applyCnotGate(state, controlBit, targetBit);
}
#ifdef QSIM_USE_CUDA
void SingleQubitGate::applyGpu(gpu::DeviceBuffer& d_buffer) const {
  qsim::gpu::startApplySingleQubitGate(d_buffer, qubit, matrix);
}

void CnotGate::applyGpu(gpu::DeviceBuffer& d_buffer) const {
  qsim::gpu::startApplyCnotGate(d_buffer, controlBit, targetBit);
}
#endif
}  // namespace qsim
