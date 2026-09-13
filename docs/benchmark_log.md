# Benchmark Log

This log will document every benchmark run. It acts as a basis for further optimizations, showing areas worth improving and their effectiveness.

- [Setup](#setup)
  - [System Specs](#system-specs)
  - [Methodology](#methodology)
    - [Correctness](#correctness)
    - [Benchmarking](#benchmarking)
- [Hardware Rooflines](#hardware-rooflines)
  - [Theoretical Limits](#theoretical-limits)
  - [Practical Limits](#practical-limits)
    - [Multicore Limits](#multicore-limits)
    - [Single core Limits](#single-core-limits)
    - [Predicted vs measured cache transitions](#predicted-vs-measured-cache-transitions)
- [2026-07-28 - CPU baseline (naive)](#2026-07-28---cpu-baseline-naive)
  - [Limitations](#limitations)
  - [Results](#results)
    - [Headline Numbers](#headline-numbers)
    - [Single Qubit Gate](#single-qubit-gate)
    - [CNOT Gate](#cnot-gate)
    - [GHZ State](#ghz-state)
    - [One-off benchmarks](#one-off-benchmarks)
  - [Conclusion](#conclusion)
    - [Cost Model](#cost-model)
  - [Improvements](#improvements)
    - [Improving Performance](#improving-performance)
    - [Improving Methodology](#improving-methodology)
  - [Reproduction of this Benchmark](#reproduction-of-this-benchmark)
    - [What has to match for the numbers to be comparable](#what-has-to-match-for-the-numbers-to-be-comparable)



## Setup 

### System Specs

All the CPU benchmarks are run on a laptop with the listed specs. 

- machine: "tom-Yoga-7-2-in-1-14ILL10"
- CPU: Intel(R) Core(TM) Ultra 7 258V, 8 Cores
    - "mhz_per_cpu": 4700
    - "cpu_scaling_enabled": true
    - Caches (on performance core):
        - L1d: 48K    
        - L1i: 64K   
        - L2: 2.5M     
        - L3: 12M     
- RAM: 32 GiB LPDDR5X-8533, 128-bit bus.
- compiler: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0  
- build type: Release
- build flags: -O3 -DNDEBUG
- benchmark library: google/benchmark v1.9.2

### Methodology

#### Correctness

We use unit tests with the `googletest` harness to ensure correctness of single functions and the Qiskit library to cross-check our end-to-end calculations.
 

#### Benchmarking 

There are 5 benchmarking families, all exclude state and circuit construction – shifting the focus towards arithmetic and memory transfers.
The families are defined as follows:

 - Single qubit gate: Runs H gate on |0…0> state
 - CNOT qubit gate: Runs CNOT gate on |0…0> state
 - GHZ state: Runs H gate and then n-1 CNOT gates on |0…0> state
 - Get Pair Indices: Calculates pair indices from pair numbers
 - Single Qubit Gate Target Sweep: Applies H gate on different target qubits.

There is a sixth family, the roofline test, which we use to get a realistic hardware limit for the device the tests run on. 


Each family is run 5 times to get a more accurate result and rule out extreme outliers.

All families count 32 bytes of traffic for every amplitude a gate updates: 

16 bytes read + 16 bytes written, one `complex<double>` in each direction. 

The single-qubit gate updates all 2ⁿ amplitudes and is therefore charged 32·2ⁿ bytes per call.

The CNOT gate only updates the half of the state where the control bit is set, so the same rule gives it 32·2ⁿ⁻¹ = 16·2ⁿ. 

GHZ is charged one H gate plus n-1 CNOT gates: 32·2ⁿ + (n-1) · 32 · 2ⁿ⁻¹ = 16 · 2ⁿ · (n+1)


The rule is consistent, so GB/s measures useful throughput within a family. But it is not a speed comparison between families: 


`google/benchmark` does not measure bytes processed, we declare them, and CNOT is charged half the bytes of the H gate for the same number of loop iterations, so its GB/s understates it by 2x.

To compare across families we will use the time, per loop iteration, instead.

The exact numbers can be found in the [numbers.md](../benchmarks/results/2026-07-28-cpu-baseline-naive/numbers.md) of each respective benchmark.

## Hardware Rooflines


### Theoretical Limits

Source: Intel spec sheet for [Intel(R) Core(TM) Ultra 7 258V, 8 Cores](https://www.intel.com/content/www/us/en/products/sku/240957/intel-core-ultra-7-processor-258v-12m-cache-up-to-4-80-ghz/specifications.html)

As a reminder: 

> - Caches (on performance core):
>      - L1d: 48K    
>      - L1i: 64K   
>      - L2: 2.5M     
>      - L3: 12M 
> - maximum bandwidth = **136.528 GB/s**
> - Vector size: 2^n · 16 Bytes


maximum bandwidth = DRAM bandwidth * bus width / 8 / 1000

maximum bandwidth = 8533 MT/s * 128 bit bus width / 8 / 1000 = **136.528 GB/s**

Looking at our state representation – a vector of `complex<double>` – each amplitude requires 16 bytes, therefore our n-qubit state will grow to size 2^n · 16 bytes. 

Knowing this size we can now compute where our vector should fit in.

Hypothesis: 

L1 = 48KiB = 48 * 1024 = 49152 bytes

 Solve: 
 2^n * 16 = 49152 for n 

 -> n = 11.5 

 All states with 11 qubits or fewer should fit in our L1 cache.

 Likewise:

 L2 -> n = 17.3 

 L3 -> n = 19.6

DRAM (32GiB) -> n = 31

This is a theoretical number since the OS itself needs DRAM and would not be able to spend all resources to just hold the state.

### Practical Limits 

#### Multicore Limits

Using Intel's Memory Latency Checker (MLC) we get a practical limit for read-write operation (multi-core) of 
1:1 reads-writes : 97.67 GB/s

Still shy of our theoretical 136.528 GB/s, but nobody expects to hit the theoretical roofline.


#### Single core Limits

To have a roofline for this exact machine all benchmarks are also compared to a simple arithmetic function. 
We read the state, multiply by a double and write back to the amplitudes. This is one of the most rudimentary operations and should provide a good roofline for our hardware.

 ![Roofline Benchmark](../benchmarks/results/2026-07-28-cpu-baseline-naive/BM_roofline.png "Loop Iterating over a state with read and write")

Observation:

Our processed data per second is far from the theoretical bandwidth, strangely enough in both directions.

Explanation: 

**Overprocessing (qubits 2-14):**

The memory bandwidth limit is calculated for DRAM. Since the states are so small they can fit entirely into the L3 cache or lower, the theoretical DRAM limit does not apply.

**Heavy underprocessing (qubits 20+):**

One possible explanation is that we operate our function on one single core, which is not able to fill the whole memory bandwidth. 
This could be solved/improved with a multi-core implementation. (This will be the focus of the CUDA backend.)


Observation:

Another noteworthy anomaly is the irregular curve from qubit sizes 2 to 10. 

Explanation:

This is likely caused by the overhead from the loop and `google/benchmark` measuring methods. 




#### Predicted vs measured cache transitions

More interestingly, our predicted performance drops happen later than anticipated. 

| Cache Switch (from/to) | Predicted Drop | Actual Drop |
| ---------------------- | -------------- | ----------- |
| L1->L2                 | 11.5 (12)      | 14          |
| L2->L3                 | 17.3 (18)      | 18          |
| L3->DRAM               | 19.6 (20)      | 21          |

Possible Explanation: 

The original hypothesis assumed that as soon as the state does not fit in one cache the whole state gets transferred to the next higher level. The data suggests otherwise. 
The state partially fits in a cache level, which results in only partial cache misses. This creates a somewhat gradual performance decrease until the majority of the state lives in the next cache.

Keep in mind:

L2 and L3 are unified caches, so the instructions from our `google/benchmark` loop will also occupy space. 

More importantly, the L3 cache is shared among all 4 performance cores, meaning the OS and outside applications could occupy space as well, leaving less for the state. 







## 2026-07-28 - CPU baseline (naive)

The exact numbers can be found in the [numbers.md](../benchmarks/results/2026-07-28-cpu-baseline-naive/numbers.md).


>git sha: 16f529f
>
>date: 2026-07-28 10:08

This is the first naive CPU implementation without further optimization. It runs the simulator on a single performance core and uses `complex<double>` to store values.
The machine has `cpu_scaling` enabled, but each family is run 5 times with a median coefficient of variation of 0.41%, the max CV being 2.56%, showing the effect of the scaling is negligible noise. 

### Limitations

Before looking at our numbers we need to outline the limitations of this benchmark:
 - single laptop
 - `cpu_scaling` on
 - 4.7 GHz is nominal so cycle counts are approximate
 - one pinned core
 - single compiler
 - target 0 in the main tables, which the sweep shows is the slowest target in DRAM
 - H is the only single-qubit gate measured
 - input is |0…0> (does not matter since the loops are data-independent and do not rely on the previous loop)

 Because of this we will never have a true 1:1 reproducible run, but the averages are enough to show and recognize performance issues.

### Results

#### Headline Numbers


| family         | in-cache plateau (11 qubits)                          | DRAM (24 qubits)                                    |
| -------------- | ----------------------------------------------------- | --------------------------------------------------- |
| roofline       | 159.49 GB/s <br>0.20 ns/iter <br>0.94 cycles/iter     | 36.86 GB/s <br> 0.87 ns/iter<br> 4.08 cycles/iter   |
| single qubit H | 22.03 GB/s<br> 2.91 ns/iter<br> 13.66 cycles/iter     | 16.57 GB/s <br> 3.86 ns/iter <br> 18.16 cycles/iter |
| CNOT           | 57.88 GB/s  <br> 0.55 ns/iter <br>   2.60 cycles/iter | 17.34 GB/s <br> 1.85 ns/iter <br> 8.68 cycles/iter  |
| GHZ            | 44.04 GB/s  <br> 0.79 ns/iter  <br> 3.73 cycles/iter  | 22.53 GB/s <br> 1.48 ns/iter <br> 6.95 cycles/iter  |



#### Single Qubit Gate

![Single Qubit Gate](../benchmarks/results/2026-07-28-cpu-baseline-naive/BM_applySingleQubitGate.png "CPU Naive Single Qubit Gate Benchmarks")

Observation:

Throughout L1 to L3 cache the throughput seems stable, which suggests that the function is compute-bound rather than memory-bound. It is up to 7.2x slower than our roofline function.


Interestingly enough there is a steep drop between the 20 and 21 qubit mark, which is likely due to the relocation of the state to DRAM. 

 We can tell from the roofline plot that our memory bound is at roughly 36.86 GB/s and the compute bound for the single-qubit gate lies at roughly 22 GB/s. 
Why is the current throughput 16 GB/s instead of 22 GB/s, at high qubit counts?
<details><summary>Roofline plot</summary>

![Roofline](../benchmarks/results/2026-07-28-cpu-baseline-naive/BM_roofline.png)

</details> <br>


Explanation:

We are experiencing memory stalls. 
Our prefetching is not able to provide cache lines fast enough, causing stalls, and since the DRAM latency is too high it cannot be hidden: the memory is not fast enough to keep the core constantly fed with information. It is not purely memory-bound though, we also experience a compute bottleneck. Instead of having one specific bound we experience an interplay between compute and memory bounds, where the CPU keeps waiting for memory to finish the loading operations and the memory keeps waiting for the computations to finish.




#### CNOT Gate

![CNOT Qubit Gate](../benchmarks/results/2026-07-28-cpu-baseline-naive/BM_applyCnotGate.png "CPU Naive CNOT Qubit Gate Benchmarks")


Observation:
1. Again we are compute-bound in L1 and L2 cache. Here the performance degradation happens earlier, as the throughput already starts to decrease in the transition to L3 cache and onwards. 

2. One strange observation is the higher throughput of the CNOT gate compared to our single-qubit gate (2.6x higher in GB/s and 5.2x faster per pair, two numbers because of the different declared bytes). The seemingly more complex CNOT gate costs apparently less than our single-qubit gate. 
For a fair comparison we will look at the time it took for one pair to be calculated. <br>
At 24 qubits this advantage collapses to 2.1x. 


| qubits | H ns/pair | CNOT ns/pair | H / CNOT |
| ------ | --------- | ------------- | -------- |
| 8      | 2.94      | 0.61          | 4.8x     |
| 12     | 2.91      | 0.56          | 5.2x     |
| 16     | 2.92      | 0.57          | 5.1x     |
| 20     | 3.35      | 0.93          | 3.6x     |
| 24     | 3.86      | 1.85          | 2.1x     |

Explanation: 
1. The drop probably stems from the same cause as in our single-qubit gate. The high latencies cannot be hidden by our CPU anymore. Since our operational speed is a lot higher than with the single-qubit gate, this happens one memory stage earlier and is visible even with the L3 cache. We are not able to prefetch enough lines to hide our latencies. 

2. Looking closer at our functions, the speed difference is quite obvious. Being made up by branching logic, the CNOT gates eliminate half of the operations. The remaining operations are amplitude swaps without any computations – which are minimal compared to the heavy matrix arithmetic in our single-qubit gate. 
The drop in the performance advantage from 5.1x to 2.1x is probably caused by the fact that both functions experience heavy stalls and have to wait on the DRAM.
This performance difference might be worth looking at after the GPU optimization, since with the single-qubit gate we will be able to compute in parallel whereas the branch will become a mask. So my intuition would suggest that in the GPU version the single-qubit gate will surpass our CNOT gate in performance. 

#### GHZ State

![GHZ State](../benchmarks/results/2026-07-28-cpu-baseline-naive/BM_ghzState.png "CPU Naive GHZ State Benchmarks")

Observation: performance ramps up logarithmically to a peak of 46.15 GB/s at 16 qubits, starts to fall from 17 qubits and flattens at roughly 22 GB/s from 22 qubits. 
A GHZ circuit is one H gate followed by n-1 CNOT gates, so asymptotically it should behave like the CNOT benchmark. It does not: in cache it is 18% slower (46.15 GB/s vs. 56.19 GB/s at 16 qubits) and in DRAM it is actually faster than our raw CNOT gate operation (22.53 GB/s vs. 17.34 GB/s).



Hypothesis: 
The general performance drop – in the lower qubits – could stem from our circuit object overhead. It runs each gate as an `Operation` in a list, which could incur overhead that we did not have when we benchmarked the raw function. 

Test: 
Looking at our numbers we can compare the 18% and evaluate overhead vs. the single H gate slowing us down. 

|                       |                |
| --------------------- | -------------- |
| GHZ call at 16 qubits | 386237.7371 ns |
| CNOT gate             | 18663.4173 ns  |
| single H gate         | 95527.2835 ns  |
| Raw GHZ time          | 375478.543 ns  |
| Slowdown              | 10759.1941 ns  |

Raw GHZ time = H + CNOT * (n-1) <br>
Slowdown = GHZ call - raw GHZ time

So we have residuals (overhead, cache misses) slowing us down by 10759.2 ns or 3%. 
This is minimal and clarifies the 18% from earlier. The logarithmic ramp-up is not coming from our circuit overhead but it is the slow H gate showing a bigger impact when there are fewer CNOT gates.
The 3% is minimal and does not warrant further improvement for now.

Observation: 

By far the most interesting observation is the fact that our GHZ is faster than our CNOT function for high qubits. This is extremely unintuitive, since it is more complex and should be slower with all the overheads. 

Possible Explanation:
This speed-up could be related to target bits. With our CNOT gate we always took the same control and target bits, 0 and 1 respectively. Our GHZ state takes i, i+1 as control and target, increasing the distance between the indices in a pair. 



#### One-off benchmarks

There are two benchmarks which we introduce now but will not elaborate on in future benchmarks, for the sake of avoiding redundancy.

##### GetPairIndices

![Get Pair Indices](../benchmarks/results/2026-07-28-cpu-baseline-naive/BM_getPairIndices.png "CPU Naive Get Pair indices Benchmarks")

The function is, as expected, stable.
We operate directly through bit modifications and without branches. Thanks to the bit operations we do not see a performance degradation for pairs which are far apart (e.g., 1 and 2^n - 1).

Importantly, we cannot look at the speed since the function is seemingly harness-bound. 
Looking at numbers.md we can see that we hover at roughly 3.70 cycles/iter. (The small fluctuations stem from the `google/benchmark` overhead, timing and loop setup – the effect on the measurement becomes smaller with more iterations).
We now compare this data with the CNOT gate function:

| GetPairIndices   | CNOT gate (8 qubits) |
| ---------------- | -------------------- |
| 3.70 cycles/iter | 2.85 cycles/iter     |

This is a contradiction since CNOT calculates pair indices, reads the values of the amplitudes and writes back to them and should therefore take more cycles than our pure function.

Explanation: 
`getPairIndices` is a pure function which reads input, computes and just returns a value, without changing any variables of our code. 
Since we are doing an isolated test and the value never gets used, the compiler will treat the code as dead. To combat this we used the `DoNotOptimize` function, which explicitly forces the function to execute. This does not resemble our real workflow since the compiler would be able to optimize and inline the function, calling it while waiting for loads or stores. 

So the important insight is that `getPairIndices` is harness-bound at the moment, and we cannot measure future improvements. 


##### Apply Single Qubit Gate Target Sweep

![Apply Single Qubit Gate Target Sweep](../benchmarks/results/2026-07-28-cpu-baseline-naive/BM_applySingleQubitGateTargetSweep.png "CPU Naive Apply Single Qubit Gate Target Sweep Benchmarks")

Setup: This benchmark builds various states of different sizes and checks whether different target bits influence the operation speed. 

Original Hypothesis: <br>
Yes, there will be an influence.<br> 
> High target bit -> amplitudes further apart in memory -> two accesses -> to separate cache lines.<br>

Since the amplitudes do not lie next to each other in memory, we cannot access them with one cache line, needing two, which reduces our speed since we have to wait until both arrive.

Observation: <br>
The original hypothesis is inverted. The further apart in memory the amplitudes are, the faster the function goes. We can see that for a 24-qubit state with target bit 0 we get 16.57 GB/s. After we increase the target bit to 10 and higher this degradation plateaus at 20.7 GB/s vs. the 22.0 in-cache bound — 94%. We are approaching our cache-computation bounds.
This effect could already be seen in the GHZ state test. It performed faster than the raw CNOT test for the same reason: our bits become further apart with later CNOT gates.

Possible Explanation:<br>
The processor prefetches amplitudes to queue them to be processed. If the pairs are next to each other the processor will use one stream to prefetch cache lines. If we have two far-apart pairs the processor will use parallel memory access, using two streams for prefetching, essentially doubling our lines in flight and hiding the DRAM latency. 


### Conclusion

#### Cost Model 


| qubits       | BM_Roofline         | BM_applyCnotGate     | BM_applySingleQubitGate |
| ------------ | ------------------- | -------------------- | ----------------------- |
| 8 (in-cache) | 1.07 cycles per amp | 2.85 cycles per pair | 13.84 cycles per pair   |
| 24 (DRAM)    | 4.08 cycles per amp | 8.68 cycles per pair | 18.16 cycles per pair   |
| DRAM tax     | +3.01 per amp       | +5.83 per pair       | +4.32 per pair          |


- Roofline: touching an amplitude
- CNOT: index calculations, branch and swap. +1.78 cycles per pair.
  CNOT only touches pairs where the control bit is set to 1, half the amps of our state but then reads and writes so it will change half of the amps but does two operations on them, so the total is 1 amp on average which makes the comparison 1:1 with the roofline.
- H Gate: Same index same writing but matrix operation. +10.99 cycles per pair.


Roofline states the tax for moving from cache to DRAM is +3.01 cycles per amp. Looking at the CNOT gate we get 5.83 per pair. 
Since the CNOT touches two amps (read and write) it roughly matches the expected cost increase, 2 times our simple roofline DRAM tax. (WRONG its 1 to 1 so doubling comes elsewhere)
Interestingly the single-qubit gate only increases by +4.32, the long calculations are able to hide the memory slowdown. The function keeps calculating while new pairs are slowly moved through memory. This is not a clear compute-bound rather a mix of both. 
Whereas the roofline and CNOT gate are purely memory bound at 24 qubits.



### Improvements

#### Improving Performance 

The most extreme performance gain would be achieved by enabling more cores. We are nowhere near our maximum memory bandwidth and should be able to at least double our performance here. 

Afterward, it would be worth seeing if we can remove arithmetic bottlenecks, since we tend to be compute-bound rather than memory-bound. 
Lastly, we should look closer into our prefetching and cache misses to hide DRAM latencies. 
Our focus should be the single-qubit operations, since the matrix calculations seem to slow us down the most. 


#### Improving Methodology 

The benchmark for the H gate is currently a bit of a black box, 
so for future benchmarks it is worth adding: 
- SingleQubitGate-Index Only: load both amplitudes, read and write back but without any math. With this we can calculate: Full H Gate - Index = arithmetic costs.

- SingleQubitGate-Sequential Index: use a loop to go through the indices and compute sequentially without the `getPairIndices` function. This will measure our `getPairIndices` cost in context.


### Reproduction of this Benchmark 

Prerequisites: Linux (the run is pinned with `taskset`, from util-linux), a C++20
compiler, CMake >= 3.25, Python 3, and network access on the first configure —
googletest, nlohmann/json and google/benchmark are all fetched by FetchContent at
pinned versions.

1. Python environment, needed only for the plots:

       python3 -m venv .venv
       .venv/bin/pip install -r requirements.txt

2. Configure Release once, with benchmarks enabled. Without `QSIM_BENCHMARK=ON` the
   `bench` target does not exist at all:

       cmake --preset benchmark

3. Build and run. Writes `benchmarks/results/latest.json`:

       cmake --build --preset benchmark --target bench

4. Plot, and keep the run under a label:

       .venv/bin/python scripts/plot_benchmarks.py --overlay --caches --save cpu-baseline-naive

   This creates `benchmarks/results/<date>-<label>/` holding a copy of the JSON, the `numbers.md`
   and every plot used in this document. The date comes from the JSON's own
   context block rather than from the clock, so re-plotting an old run still
   files it under the day it was measured.

`latest.json` and `latest/` are gitignored; only labeled runs are committed.

#### What has to match for the numbers to be comparable

The `bench` target pins to CPU 3 with `taskset -c 3`. On this machine CPUs 0-3
are P-cores (4.7 GHz, 2.5 MB private L2) and CPUs 4-7 are LPE-cores (3.7 GHz,
4 MB shared), so which core the scheduler picks changes both the clock and the
cache, and unpinned runs are not comparable. **On any other machine that
number means something different and must be changed.**

The rest of this run: 5 repetitions, aggregates only, Release (-O3 -DNDEBUG),
g++ 15.2.0, google/benchmark v1.9.2, and cpu_scaling_enabled was true (median CV
0.41%, which does not move the numbers materially). The `context` block inside
`results.json` next to the plots is the authoritative record — host, date, and cache sizes all come from there, not from this document.
