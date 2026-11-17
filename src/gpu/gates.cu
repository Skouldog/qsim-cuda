#include <cuda_runtime.h>
#include <math.h>

#include <cuda/cmath>
#include <cuda/std/array>
#include <cuda/std/complex>
#include <cuda/std/utility>

#include "qsim/gates.hpp"
#include "qsim/gpu/gates.hpp"

#define CUDA_CHECK(ans)                   \
  {                                       \
    gpuAssert((ans), __FILE__, __LINE__); \
  }

inline void gpuAssert(cudaError_t code, const char* file, int line,
                      bool abort = true) {
  if (code != cudaSuccess) {
    fprintf(stderr, "CUDA Error: %s %s %d\n", cudaGetErrorString(code), file,
            line);
    if (abort) exit(code);
  }
}

__device__ cuda::std::pair<std::size_t, std::size_t> getPair(
    int qubit, std::size_t workerId) {
  std::size_t index =
      ((workerId >> qubit) << (qubit + 1)) |         // Left Bitmasking
      (((std::size_t{1} << qubit) - 1) & workerId);  // Right Bitmasking
  // See gates.cpp for detailed process

  std::size_t indexPartner = index | (std::size_t{1} << qubit);

  cuda::std::pair<std::size_t, std::size_t> pairIndices = {index, indexPartner};
  return pairIndices;
}

__global__ void applySingleQubitGateKernel(
    int qubit, std::size_t numAmps, cuda::std::complex<double>* d_amps,
    cuda::std::array<cuda::std::complex<double>,
                     4>  // read matrix left to right, top to bottom
        matrix) {        // d_amps = device State beginning of Array
  std::size_t workerId = threadIdx.x + blockDim.x * blockIdx.x;

  if (workerId < numAmps / 2) {
    cuda::std::pair<std::size_t, std::size_t> pair = getPair(qubit, workerId);

    cuda::std::pair<cuda::std::complex<double>, cuda::std::complex<double>>
        pairAmplitudesOld;
    pairAmplitudesOld.first = d_amps[pair.first];
    pairAmplitudesOld.second = d_amps[pair.second];

    d_amps[pair.first] = matrix[0] * pairAmplitudesOld.first +
                         matrix[1] * pairAmplitudesOld.second;
    d_amps[pair.second] = matrix[2] * pairAmplitudesOld.first +
                          matrix[3] * pairAmplitudesOld.second;
  }
}

namespace qsim {
namespace gpu {

void startApplySingleQubitGate(qsim::VectorState& state, int qubit,
                               std::complex<double>* d_amps,
                               const Matrix2& matrix) {
  cuda::std::array<cuda::std::complex<double>, 4> matrixGpu;
  matrixGpu[0] = matrix[0][0];  // 0,1
  matrixGpu[1] = matrix[0][1];  // 2,3
  matrixGpu[2] = matrix[1][0];
  matrixGpu[3] = matrix[1][1];

  std::size_t size = state.getSize();
  int blocks = cuda::ceil_div(size / 2, static_cast<std::size_t>(kThreads));

  applySingleQubitGateKernel<<<blocks, kThreads>>>(
      qubit, size, reinterpret_cast<cuda::std::complex<double>*>(d_amps),
      matrixGpu);

  CUDA_CHECK(cudaGetLastError());
}

__global__ void applyCnotGateKernel(cuda::std::complex<double>* d_amps,
                                    int controlBit, int targetBit,
                                    std::size_t numAmps) {
  std::size_t workerId = threadIdx.x + blockIdx.x * blockDim.x;

  if (workerId < numAmps / 2) {
    cuda::std::pair<std::size_t, std::size_t> pairIndices =
        getPair(targetBit, workerId);

    if ((pairIndices.first & (std::size_t{1} << controlBit)) != 0) {
      cuda::std::complex<double> temp = d_amps[pairIndices.second];
      d_amps[pairIndices.second] = d_amps[pairIndices.first];
      d_amps[pairIndices.first] = temp;
    }
  }
}

void startApplyCnotGate(VectorState& state, std::complex<double>* d_amps,
                        int controlBit, int targetBit) {
  std::size_t size = state.getSize();

  int blocks = cuda::ceil_div(size / 2, static_cast<std::size_t>(kThreads));
  applyCnotGateKernel<<<blocks, kThreads>>>(
      reinterpret_cast<cuda::std::complex<double>*>(d_amps), controlBit,
      targetBit, size);

  CUDA_CHECK(cudaGetLastError());
}

std::complex<double>* upload(VectorState& state) {
  size_t stateSizeBytes = state.getSize() * sizeof(double) * 2;
  std::complex<double>* d_amps = nullptr;

  CUDA_CHECK(
      cudaHostRegister(state.data(), stateSizeBytes, cudaHostRegisterDefault));

  CUDA_CHECK(cudaMalloc(&d_amps, stateSizeBytes));

  CUDA_CHECK(
      cudaMemcpy(d_amps, state.data(), stateSizeBytes, cudaMemcpyDefault));

  return d_amps;
}

void download(VectorState& state, std::complex<double>* d_amps) {
  size_t stateSizeBytes = state.getSize() * sizeof(double) * 2;

  CUDA_CHECK(
      cudaMemcpy(state.data(), d_amps, stateSizeBytes, cudaMemcpyDefault));
  CUDA_CHECK(cudaFree(d_amps));
  CUDA_CHECK(cudaHostUnregister(state.data()));
}

}  // namespace gpu
}  // namespace qsim
