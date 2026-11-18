#include <cuda_runtime.h>

#include <cstddef>
#include <stdexcept>
#include <string>

#include "qsim/gpu/device_buffer.hpp"

#define CUDA_CHECK(ans)                   \
  {                                       \
    gpuAssert((ans), __FILE__, __LINE__); \
  }

namespace qsim {
namespace gpu {
namespace {

// TODO: move/create to cuda_check.hpp
inline void gpuAssert(cudaError_t code, const char* file, int line) {
  if (code != cudaSuccess) {
    throw std::runtime_error(std::string("CUDA Error: ") +
                             cudaGetErrorString(code) + std::string(" at ") +
                             std::string(file) + std::string(":") +
                             std::to_string(line));
  }
}
}  // namespace

DeviceBuffer::DeviceBuffer(std::size_t sizeBytes)
    : d_amps_(nullptr), sizeBytes_(sizeBytes) {
  CUDA_CHECK(cudaMalloc(&d_amps_, sizeBytes_));
}

DeviceBuffer::~DeviceBuffer() { cudaFree(d_amps_); }

}  // namespace gpu
}  // namespace qsim
