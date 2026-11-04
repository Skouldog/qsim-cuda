#include <cuda_runtime.h>
#include <math.h>

#include <cuda/cmath>
#include <iostream>

// nvcc -c src/gpu/gates.cu -o /dev/null

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

/**

__global__ void applySingleQubitGate(qsim::VectorState& state, int qubit,
                          const Matrix2& matrix, std::complex<double> * ptrAmps)
{

  for (std::size_t pairNumber = 0; pairNumber < state.getSize() / 2;
       pairNumber++) {
    std::pair<std::size_t, std::size_t> pairIndices =
        qsim::detail::getPairIndices(pairNumber, qubit);

    std::pair<std::complex<double>, std::complex<double>> pairAmplitudes;

    // Get Amplitudes from state
    pairAmplitudes.first = ptrAmps[pairIndices.first];
    pairAmplitudes.second = ptrAmps[pairIndices.second];

    pairAmplitudes = qsim::detail::calculateAmplitudes(matrix, pairAmplitudes);

    // write back to state
    ptrAmps[pairIndices.first] = pairAmplitudes.first;
    ptrAmps[pairIndices.second] = pairAmplitudes.second;
  }


  int workerIndex = threadIdx.x;


}
*/

__global__ void get_pairs(double* state, int qubit, int amps) {
  int worker = threadIdx.x + blockDim.x * blockIdx.x;

  if (worker < amps / 2) {
    state[worker] =
        ((worker >> qubit) << (qubit + 1)) |         // Left Bitmasking
        (((std::size_t{1} << qubit) - 1) & worker);  // Right Bitmasking
  }  // See gates.cpp for detailed process
}

int main(void) {
  int amps = 1 << 3;
  int stateSizeBytes = amps * sizeof(double);

  const int threads = 256;

  double* state = nullptr;

  double* devState = nullptr;

  // Buffer for CPU
  CUDA_CHECK(cudaMallocHost(&state, stateSizeBytes));

  // Buffer for GPU
  CUDA_CHECK(cudaMalloc(&devState, stateSizeBytes));

  // init arrays
  for (int i = 0; i < amps; i++) {
    state[i] = 1.0;
  }

  // CUDA_CHECK(cudaMemcpy(devState, state, stateSizeBytes, cudaMemcpyDefault));
  // CUDA_CHECK(cudaMemset(devState, 0, stateSizeBytes));

  int blocks = cuda::ceil_div(amps, threads);

  get_pairs<<<blocks, threads>>>(devState, 0, amps);
  CUDA_CHECK(cudaGetLastError());

  CUDA_CHECK(cudaDeviceSynchronize());

  cudaMemcpy(state, devState, stateSizeBytes, cudaMemcpyDefault);

  for (int i = 0; i < amps; i++)
    std::cout << "Paired with: " << state[i] << std::endl;

  cudaFreeHost(state);

  cudaFree(devState);

  return 0;
}
