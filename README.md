# qsim-cuda: A GPU-accelerated quantum state-vector simulator — CPU baseline stage

[![CI](https://github.com/Skouldog/qsim-cuda/actions/workflows/ci.yml/badge.svg)](https://github.com/Skouldog/qsim-cuda/actions/workflows/ci.yml)
[![codecov](https://codecov.io/github/Skouldog/qsim-cuda/graph/badge.svg?token=U1JBSK90C2)](https://codecov.io/github/Skouldog/qsim-cuda)


The goal is to build a quantum state-vector simulator and optimize speed by using the CUDA API. The current version provides a naive baseline and does not have any GPU code yet.


---

## Status

Currently, we can create vector states and use basic single-qubit gates, CNOT gate and Circuits (consisting of said gates), to manipulate the states. We can sample measurement outcomes. 
This vector simulation is run on a single CPU core and cross-checked against the Qiskit library to ensure correctness. 
The next steps are optimization and the heart of this project: using GPU parallelism to increase operation speed.


---

## Build, test and reproducibility

Important: To build this library you need CMake >= 3.25 and C++20.
We also use GoogleTest, GoogleBenchmark and nlohmann/json, which get automatically fetched. This requires internet.
The target bench also pins the core 3 for all the executions. This is Linux only and will need adjustment for other operating systems. 


### Build and test

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

### Release build

```bash
cmake --preset release
cmake --build --preset release
```

### Run benchmarks

Builds `qsim_bench` and runs it, writing `benchmarks/results/latest.json`.

```bash
cmake --preset benchmark
cmake --build --preset benchmark --target bench
```



---

## Benchmarks

Currently we run the H gate operation and CNOT gate with the following speeds. 


| qubits | H ns/iter | CNOT ns/iter | H / CNOT |
| ------ | --------- | ------------ | -------- |
| 8      | 2.94      | 0.61         | 4.8x     |
| 12     | 2.91      | 0.56         | 5.2x     |
| 16     | 2.92      | 0.57         | 5.1x     |
| 20     | 3.35      | 0.93         | 3.6x     |
| 24     | 3.86      | 1.85         | 2.1x     |


CNOT is faster because it only swaps two amplitudes, skipping pairs whose control bit is zero. The H gate does a 2x2 matrix multiply on every pair. The speed difference gets smaller with bigger states, since they both start to get limited by memory bandwidth.



With a total operation time for a 16-qubit state. 

| Gate          | Time (total)  |
| ------------- | ------------- |
| CNOT gate     | 18663.4173 ns |
| single H gate | 95527.2835 ns |

The exact hardware and numbers can be seen at [the benchmark log](docs/benchmark_log.md)


---

## License

The license is MIT
