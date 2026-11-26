#pragma once

#include <complex>

#include "qsim/gates.hpp"
#include "qsim/gpu/device_buffer.hpp"
#include "qsim/state.hpp"

namespace qsim {
namespace gpu {

constexpr int kThreads = 256;

void startApplySingleQubitGate(DeviceBuffer& d_buffer, const int qubit,
                               const Matrix2& matrix);

void startApplyCnotGate(DeviceBuffer& d_buffer, const int controlBit,
                        const int targetBit);
DeviceBuffer upload(VectorState& state);

void download(VectorState& state, DeviceBuffer& d_buffer);
}  // namespace gpu
}  // namespace qsim
