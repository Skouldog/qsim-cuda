#pragma once

#include <complex>

#include "qsim/gates.hpp"
#include "qsim/gpu/device_buffer.hpp"
#include "qsim/state.hpp"

namespace qsim {
namespace gpu {

constexpr int kThreads = 256;

void startApplySingleQubitGate(const qsim::VectorState& state, int qubit,
                               std::complex<double>* d_amps,
                               const Matrix2& matrix);

void startApplyCnotGate(const VectorState& state, std::complex<double>* d_amps,
                        int controlBit, int targetBit);
DeviceBuffer upload(VectorState& state);

void download(VectorState& state, DeviceBuffer& d_buffer);
}  // namespace gpu
}  // namespace qsim
