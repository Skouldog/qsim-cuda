#include <cuda_runtime.h>

#include <cstddef>
#include <utility>

#include "cuda_check.hpp"
#include "qsim/gpu/device_buffer.hpp"

namespace qsim {
namespace gpu {

DeviceBuffer::DeviceBuffer(std::size_t sizeBytes)
    : d_amps_(nullptr), sizeBytes_(sizeBytes) {
  CUDA_CHECK(cudaMalloc(&d_amps_, sizeBytes_));
}

DeviceBuffer& DeviceBuffer::operator=(DeviceBuffer&& other) noexcept {
  if (this != &other) {
    cudaFree(d_amps_);
    d_amps_ = std::exchange(other.d_amps_, nullptr);
    sizeBytes_ = std::exchange(other.sizeBytes_, 0);
  }
  return *this;
}

DeviceBuffer::~DeviceBuffer() { cudaFree(d_amps_); }

}  // namespace gpu
}  // namespace qsim
