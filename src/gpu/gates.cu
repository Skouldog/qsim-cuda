#include <cuda_runtime.h>

#include <cuda/cmath>
#include <cuda/std/array>
#include <cuda/std/complex>
#include <cuda/std/utility>

#include "cuda_check.hpp"
#include "qsim/gates.hpp"
#include "qsim/gpu/device_buffer.hpp"
#include "qsim/gpu/gates.hpp"

namespace qsim {
namespace gpu {
namespace {

__device__ cuda::std::pair<std::size_t, std::size_t> getPair(
    const int qubit, const std::size_t workerId) {
  const std::size_t index =
      ((workerId >> qubit) << (qubit + 1)) |         // Left Bitmasking
      (((std::size_t{1} << qubit) - 1) & workerId);  // Right Bitmasking
  // See gates.cpp for detailed process

  const std::size_t indexPartner = index | (std::size_t{1} << qubit);

  const cuda::std::pair<std::size_t, std::size_t> pairIndices = {index,
                                                                 indexPartner};
  return pairIndices;
}

// d_amps = device State beginning of Array
// matrix = left to right, top to bottom
__global__ void applySingleQubitGateKernel(
    const int qubit, const std::size_t numAmps,
    cuda::std::complex<double>* d_amps,
    const cuda::std::array<cuda::std::complex<double>, 4> matrix) {
  const std::size_t workerId = threadIdx.x + blockDim.x * blockIdx.x;

  if (workerId < numAmps / 2) {
    const cuda::std::pair<std::size_t, std::size_t> pair =
        getPair(qubit, workerId);

    const cuda::std::pair<cuda::std::complex<double>,
                          cuda::std::complex<double>>
        pairAmplitudesOld = {d_amps[pair.first], d_amps[pair.second]};

    d_amps[pair.first] = matrix[0] * pairAmplitudesOld.first +
                         matrix[1] * pairAmplitudesOld.second;
    d_amps[pair.second] = matrix[2] * pairAmplitudesOld.first +
                          matrix[3] * pairAmplitudesOld.second;
  }
}

__global__ void applyCnotGateKernel(cuda::std::complex<double>* d_amps,
                                    const int controlBit, const int targetBit,
                                    const std::size_t numAmps) {
  const std::size_t workerId = threadIdx.x + blockIdx.x * blockDim.x;

  if (workerId < numAmps / 2) {
    const cuda::std::pair<std::size_t, std::size_t> pairIndices =
        getPair(targetBit, workerId);

    if ((pairIndices.first & (std::size_t{1} << controlBit)) != 0) {
      const cuda::std::complex<double> temp = d_amps[pairIndices.second];
      d_amps[pairIndices.second] = d_amps[pairIndices.first];
      d_amps[pairIndices.first] = temp;
    }
  }
}
}  // namespace

void startApplySingleQubitGate(DeviceBuffer& d_buffer, const int qubit,
                               const Matrix2& matrix) {
  const cuda::std::array<cuda::std::complex<double>, 4> matrixGpu = {
      matrix[0][0], matrix[0][1], matrix[1][0], matrix[1][1]};

  const std::size_t size =
      d_buffer.getSizeBytes() / sizeof(std::complex<double>);
  const int blocks =
      cuda::ceil_div(size / 2, static_cast<std::size_t>(kThreads));

  applySingleQubitGateKernel<<<blocks, kThreads>>>(
      qubit, size,
      reinterpret_cast<cuda::std::complex<double>*>(d_buffer.data()),
      matrixGpu);

  CUDA_CHECK(cudaGetLastError());
}

void startApplyCnotGate(DeviceBuffer& d_buffer, const int controlBit,
                        const int targetBit) {
  const std::size_t size =
      d_buffer.getSizeBytes() / sizeof(std::complex<double>);
  const int blocks =
      cuda::ceil_div(size / 2, static_cast<std::size_t>(kThreads));
  applyCnotGateKernel<<<blocks, kThreads>>>(
      reinterpret_cast<cuda::std::complex<double>*>(d_buffer.data()),
      controlBit, targetBit, size);

  CUDA_CHECK(cudaGetLastError());
}

DeviceBuffer upload(VectorState& state) {
  const size_t stateSizeBytes = state.getSize() * sizeof(std::complex<double>);
  DeviceBuffer d_buffer(stateSizeBytes);

  CUDA_CHECK(cudaMemcpy(d_buffer.data(), state.data(), stateSizeBytes,
                        cudaMemcpyDefault));

  return d_buffer;
}

void download(VectorState& state, DeviceBuffer& d_buffer) {
  const size_t stateSizeBytes = d_buffer.getSizeBytes();

  CUDA_CHECK(cudaMemcpy(state.data(), d_buffer.data(), stateSizeBytes,
                        cudaMemcpyDefault));
}

}  // namespace gpu
}  // namespace qsim
