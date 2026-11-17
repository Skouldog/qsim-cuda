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
void SingleQubitGate::applyGpu(VectorState& state,
                               std::complex<double>* d_amps) const {
  qsim::gpu::startApplySingleQubitGate(state, qubit, d_amps, matrix);
}

void CnotGate::applyGpu(VectorState& state,
                        std::complex<double>* d_amps) const {
  qsim::gpu::startApplyCnotGate(state, d_amps, controlBit, targetBit);
}
#endif
}  // namespace qsim
