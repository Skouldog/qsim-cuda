#pragma once

#include <cuda_runtime.h>

#include <stdexcept>
#include <string>

#define CUDA_CHECK(ans)                              \
  {                                                  \
    qsim::gpu::gpuAssert((ans), __FILE__, __LINE__); \
  }

namespace qsim {
namespace gpu {
inline void gpuAssert(cudaError_t code, const char* file, int line) {
  if (code != cudaSuccess) {
    throw std::runtime_error(std::string("CUDA Error: ") +
                             cudaGetErrorString(code) + std::string(" at ") +
                             std::string(file) + std::string(":") +
                             std::to_string(line));
  }
}
}  // namespace gpu
}  // namespace qsim
