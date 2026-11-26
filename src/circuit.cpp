#include "qsim/circuit.hpp"

#include <memory>

#include "qsim/gates.hpp"
#include "qsim/gpu/device_buffer.hpp"
#include "qsim/gpu/gates.hpp"
#include "qsim/operation.hpp"
#include "qsim/state.hpp"

namespace qsim {

void Circuit::add(Matrix2 matrix, int qubit) {
  circuitList.push_back(std::make_unique<SingleQubitGate>(matrix, qubit));
}

void Circuit::addCnot(int controlBit, int targetBit) {
  circuitList.push_back(std::make_unique<CnotGate>(controlBit, targetBit));
}

void Circuit::run(qsim::VectorState& state) {
  for (const std::unique_ptr<Operation>& ptrOp : circuitList) {
    ptrOp->apply(state);
  }
}

#ifdef QSIM_USE_CUDA
void Circuit::runGpu(qsim::VectorState& state) {
  qsim::gpu::DeviceBuffer d_buffer = qsim::gpu::upload(state);
  for (const std::unique_ptr<Operation>& ptrOp : circuitList) {
    ptrOp->applyGpu(d_buffer);
  }

  qsim::gpu::download(state, d_buffer);
}
#endif

}  // namespace qsim
