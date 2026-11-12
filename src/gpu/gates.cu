#include <cuda_runtime.h>
#include <math.h>

#include <cuda/cmath>
#include <cuda/std/array>
#include <cuda/std/complex>
#include <cuda/std/utility>
#include <iostream>

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

__device__ cuda::std::pair<std::size_t, std::size_t> get_pair(
    cuda::std::complex<double>* ptrAmps, int qubit, std::size_t amps,
    std::size_t workerId) {
  std::size_t index =
      ((workerId >> qubit) << (qubit + 1)) |         // Left Bitmasking
      (((std::size_t{1} << qubit) - 1) & workerId);  // Right Bitmasking
  // See gates.cpp for detailed process

  std::size_t indexPartner = index | (std::size_t{1} << qubit);

  cuda::std::pair<std::size_t, std::size_t> pairIndices = {index, indexPartner};
  return pairIndices;
}

__global__ void applySingleQubitGateK(
    int qubit, std::size_t amps, std::size_t sizeBytes,
    cuda::std::complex<double>* ptrAmps,
    cuda::std::array<cuda::std::complex<double>,
                     4>  // read matrix left to right, top to bottom
        matrix) {        // ptrAmps = devState beginning of Array
  std::size_t workerId = threadIdx.x + blockDim.x * blockIdx.x;

  if (workerId < amps / 2) {
    cuda::std::pair<std::size_t, std::size_t> pair =
        get_pair(ptrAmps, qubit, amps, workerId);

    cuda::std::pair<cuda::std::complex<double>, cuda::std::complex<double>>
        pairAmplitudesOld;
    pairAmplitudesOld.first = ptrAmps[pair.first];
    pairAmplitudesOld.second = ptrAmps[pair.second];

    ptrAmps[pair.first] = matrix[0] * pairAmplitudesOld.first +
                          matrix[1] * pairAmplitudesOld.second;
    ptrAmps[pair.second] = matrix[2] * pairAmplitudesOld.first +
                           matrix[3] * pairAmplitudesOld.second;
  };
}

namespace qsim {
namespace gpu {

void startApplySingleQubitGate(qsim::VectorState& state, int qubit,
                               std::complex<double>* devPtr,
                               const Matrix2& matrix) {
  cuda::std::array<cuda::std::complex<double>, 4> matrixGpu;
  matrixGpu[0] = matrix[0][0];  // 0,1
  matrixGpu[1] = matrix[0][1];  // 2,3
  matrixGpu[2] = matrix[1][0];
  matrixGpu[3] = matrix[1][1];
  cuda::std::complex<double>* ptrGpu =
      reinterpret_cast<cuda::std::complex<double>*>(devPtr);
  size_t size = state.getSize();
  size_t sizeBytes = state.getSize() * sizeof(double) * 2;

  int blocks = cuda::ceil_div(size / 2, kThreads);

  applySingleQubitGateK<<<blocks, kThreads>>>(qubit, size, sizeBytes, ptrGpu,
                                              matrixGpu);

  CUDA_CHECK(cudaGetLastError());
};

void startApplyCnotGate(VectorState& state, std::complex<double>* devPtr,
                        int controlBit, int targetBit) {

};

std::complex<double>* upload(VectorState& state) {
  size_t stateSizeBytes = state.getSize() * sizeof(double) * 2;
  std::complex<double>* devPtr = nullptr;

  CUDA_CHECK(
      cudaHostRegister(state.data(), stateSizeBytes, cudaHostRegisterDefault));

  CUDA_CHECK(cudaMalloc(&devPtr, stateSizeBytes));

  CUDA_CHECK(
      cudaMemcpy(devPtr, state.data(), stateSizeBytes, cudaMemcpyDefault));

  return devPtr;
}

void download(VectorState& state, std::complex<double>* devPtr) {
  size_t stateSizeBytes = state.getSize() * sizeof(double) * 2;

  CUDA_CHECK(
      cudaMemcpy(state.data(), devPtr, stateSizeBytes, cudaMemcpyDefault));
  CUDA_CHECK(cudaFree(devPtr));
  CUDA_CHECK(cudaHostUnregister(state.data()));
}

}  // namespace gpu
}  // namespace qsim
