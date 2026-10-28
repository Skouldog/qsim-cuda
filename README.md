# qsim-cuda: A GPU-accelerated quantum state-vector simulator — CPU baseline stage

[![CI](https://github.com/Skouldog/qsim-cuda/actions/workflows/ci.yml/badge.svg)](https://github.com/Skouldog/qsim-cuda/actions/workflows/ci.yml)
[![codecov](https://codecov.io/github/Skouldog/qsim-cuda/graph/badge.svg?token=U1JBSK90C2)](https://codecov.io/github/Skouldog/qsim-cuda)


The goal is to build a quantum state-vector simulator and optimize speed by using the CUDA API. The current version is a multithreaded CPU implementation (OpenMP) and does not have any GPU code yet.


---

## Status

Currently, we can create vector states and use basic single-qubit gates, CNOT gate and Circuits (consisting of said gates), to manipulate the states. We can sample measurement outcomes. 
This vector simulation is run on eight threads – for states with 15+ qubits – and cross-checked against the Qiskit library to ensure correctness. 
The next step is using GPU parallelism to increase operation speed.


---


## Benchmarks

Measured on an Intel Core Ultra 7 258V laptop (4 P + 4 E cores), g++ 15.2, Release build.
All tables use **ns/iter** = time per amplitude pair, lower is better.
The exact hardware and numbers can be seen at [the benchmark log](docs/benchmark_log.md).



### [v0.2.0](https://github.com/Skouldog/qsim-cuda/tree/v0.2.0): multithreaded (current)

| qubits | H ns/iter | H speed-up | CNOT ns/iter | CNOT speed-up |
| ------ | --------- | ---------- | ------------ | ------------- |
| 16     | 0.95      | 3.1x       | 0.50         | 1.1x          |
| 20     | 0.71      | 4.7x       | 0.40         | 2.3x          |
| 24     | 0.90      | 4.3x       | 0.76         | 2.4x          |

*"speed-up" is relative to the v0.1.0 implementation (below). Small states under 15 qubits are run single-threaded, so they are not shown. OpenMP introduces a slow-down for our single-core path, which makes small states slower than v0.1.0. We ignore this, since our focus is on large states.*



Total call time for one operation on a 24-qubit state. 

| Gate          | Time (total) |
| ------------- | ------------ |
| CNOT gate     | 6.40 ms      |
| single H gate | 7.52 ms      |
| GHZ state     | 104.98 ms    |





### [v0.1.0](https://github.com/Skouldog/qsim-cuda/tree/v0.1.0): single core


| qubits | H ns/iter | CNOT ns/iter | H / CNOT |
| ------ | --------- | ------------ | -------- |
| 8      | 2.94      | 0.61         | 4.8x     |
| 12     | 2.91      | 0.56         | 5.2x     |
| 16     | 2.92      | 0.57         | 5.1x     |
| 20     | 3.35      | 0.93         | 3.6x     |
| 24     | 3.86      | 1.85         | 2.1x     |



*CNOT is faster because it only swaps two amplitudes, skipping pairs whose control bit is zero. The H gate does a 2x2 matrix multiply on every pair. The speed difference gets smaller with bigger states, since they both start to get limited by memory bandwidth.*



---

## Build, test and reproducibility

Important: To build this library you need CMake >= 3.25 and C++20.
We also use GoogleTest, GoogleBenchmark and nlohmann/json, which get automatically fetched. This requires internet.
We also use OpenMP to manage our multi-threading, which defaults to single core if it is not found.


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


## License

The license is MIT
