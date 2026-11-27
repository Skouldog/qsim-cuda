#include <cuda_runtime.h>

#include <cstddef>
#include <utility>

#include "cuda_check.hpp"
#include "qsim/gpu/device_buffer.hpp"

namespace qsim {
namespace gpu {

DeviceBuffer::DeviceBuffer(std::size_t size) : d_amps_(nullptr), size_(size) {
  CUDA_CHECK(cudaMalloc(&d_amps_, getSizeBytes()));
}

DeviceBuffer& DeviceBuffer::operator=(DeviceBuffer&& other) noexcept {
  if (this != &other) {
    cudaFree(d_amps_);
    d_amps_ = std::exchange(other.d_amps_, nullptr);
    size_ = std::exchange(other.size_, 0);
  }
  return *this;
}

DeviceBuffer::DeviceBuffer(DeviceBuffer&& other) noexcept
    : d_amps_(other.d_amps_), size_(other.size_) {
  other.d_amps_ = nullptr;
  other.size_ = 0;
}

DeviceBuffer::~DeviceBuffer() { cudaFree(d_amps_); }

}  // namespace gpu
}  // namespace qsim
