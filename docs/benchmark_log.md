# Benchmark Insights


## Hardware Rooflines

### Theoretical Limits: [Intel(R) Core(TM) Ultra 7 258V, 8 Cores](https://www.intel.com/content/www/us/en/products/sku/240957/intel-core-ultra-7-processor-258v-12m-cache-up-to-4-80-ghz/specifications.html)

All the CPU Benchmarks are being run on this CPU

Maximum Bandwidth = DRAM bandwidth * Bus size / 8 / 1000
Maximum Bandwidth = 8533 MT/s * 128 bit bus width / 8 / 1000 = <b>136.528 GB/s</b>

 Looking at our State representation — a Vector of complex<double>. 
 Each amplitude requires 16 Byte, therefore a n Qubits-State will grow to Size 2^n * 16 Bytes. 

Knowing this Size we can now compute where our Vector will be cached in.

 Hypothesis: 

 L1 = 48KiB = 48 * 1024 = 49152 Bytes

 Solve: 
 2^n* 16 = 49152 for n 
 -> n = 11.5 

 All states with 11 Qubits and less should fit in our L1 Cache.

 Analog:

 L2 -> n = 17.3 
 L3 -> n = 19.6

DRAM(32GiB) n = 31: 

This is a theoretical Number since the OS itself needs DRAM and would not be able to spend all resources to just hold the State.

### Practical Limits: 

To have a roofline of this exact machine all benchmarks are also compared to a simple arithmetic function. 
We read the state, multiply with a double and write back to the amplitudes. This is one of the most rudimentary operations and should provide a good limit for our hardware.

 ![Roofline Benchmark](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_roofline.png "Loop Iterating over a state with read and write")

Observation:


Our processed data per second is far from the theoretical bandwidth, strangely enough in both directions.

Overprocessing (Qubits 1-14):
The memory bandwidth limit is calculated for DRAM. Since the States are so small they can fit entirely into the L3 cache or lower, the theoretical DRAM limit does not apply.


Another noteworthy anomaly is the irregular curve from Qubit sizes 1 till 10. This is likely to be caused by the overhead from the loop and google benchmarks measuring methods. 


Heavy under processing(Qubits 20+)
One possible explanation is that we operate our function on one single core, which is not able to fill the whole memory bandwidth. 
This could be solved with a multi-core implementation. Which will be the focus for our GPU Implementation.

Using Intels Memory Checker we get a practical limit for Read-Write operation(multi-core) of 
1:1 Reads-Writes : 97666.1 MB/sec
Still shy of our theoretical 135 GB/sec but it is expected to never hit the true roofline

Wrong Theory:

More interestingly our predicted performance drops happen later than anticipated. 

| Cache Switch (from/to)| Predicted Drop | Actual Drop | 
| -------- | -------         |------       |
| L1->L2       | 11.5            |        14     |
| L2->L3       | 17.3            |        18     |
| L3->DRAM       | 19.6            |        21     |

Possible Explanation: 
The original hypothesis assumed that as soon the state does not fit in one cache the whole state gets transferred to the next higher level. The data suggest a different approach. 
The state gets partially fitted in a Cache Level, which results in only partial cache misses. This creates a somewhat gradual performance decrease until the majority of the state lives in the next cache.

Keeping in Mind:
L2 as well as L3 Cache are unified, so the instructions from our Google benchmark loop will also occupy space. 

More important the L3 Cache is shared amongst all 8 Cores, meaning the OS and outside applications will occupy space as well, leaving less for the state. 







## Initial Benchmark 

### System Specs

- git sha: 16f529f
- date: 2026-07-28 10:08
- machine: "tom-Yoga-7-2-in-1-14ILL10",
- CPU:   Intel(R) Core(TM) Ultra 7 258V, 8 Cores
    - "mhz_per_cpu": 4700,
    - "cpu_scaling_enabled": true,
    - Caches:  (on performance core)
        - L1d       48K    
        - (L1i       64K)     
        - L2       2.5M     
        - L3        12M     
- compiler: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0  
- build type: Release
- build flags: -O3 -DNDEBUG
- benchmark library: googlebenchmark v1.9.2

### Methodology

This is the first naive CPU implementation without further optimization.

There are 5 families, all exclude State and Circuit Constructions, shifting the focus of the heavy calculations and memory transfers.


 - Singel qubit gate: Runs H Gate on |0...0> State
 - Cnot qubit gate: Runs Cnot Gate on |0...0> State
 - GHZ State: Runs H Gate and then n-1 CNOT Gates on |0...0> State
 - Get Pair Indices: Calculates Pair Indices from Pair Numbers
 - Single Qubit Gate Target Sweep: Applys H Gate on different Targetqubits


### Results

#### Sinlge Qubit Gate

![Single Qubit Gate](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_applySingleQubitGate.png "CPU Naive Single Qubit Gate Benchmarks")

Observation
Throughout L1 to L3 cache the throughput seems stable, this suggests that the function is not memory bound rather computation bound. It is up to 8 times slower than our roofline function.

Interestingly enough there is a steep drop at the 20 Qubit mark, this is likely due to the relocation of the state to DRAM. 

We can tell from the roofline that our memory bound is at roughly 38 GB/s and our computational for the SingleQubitGate lies at roughly 22 GB/s. 
Why is the current throughput 16 GB/s instead of 22GB/s, for high qubits?

Hypothesis:

Our prefetching is not able to provide Cache-Lines fast enough, causing stalls and since the DRAM latency is too high it can't be hidden. 

#### CNOT QUbit Gate

![CNOT Qubit Gate](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_applyCnotGate.png "CPU Naive CNOT Qubit Gate Benchmarks")


Observation:
1. Again we see a computational bound for the L1 and L2 Cache, here the performance degradation happens earlier, as the throughput already starts to decrease in transition to L3 Cache and onwards. 

2. One strange observation is the higher throughput of the CNOT Gate compared to our singleQubitGate. The seemingly more complex Cnot gate costs apparently less than our single qubit gate. Making it up to 3x faster. 

Explantion: 
1. The drop probably stems from the same cause as in our single qubit gate. The high latencies can not be hidden by our CPU anymore. Since our operational speed is alot higher than with the single qubit gate, this happens one memory stage earlier and is visible even with the L3 Cache. We are not able to prefetch enough lines to hide our latencies. 

2. Looking closer at our functions the speed difference is quite obvious. Being made up by branching logic, the Cnot Gates eliminates a lot of operations. The remaining operations are amplitude swap without any computations, compared to the heavy matrix arithmetics in our singleQubitGate. 
This performance difference might be worth looking at after the GPU optimization, since with the single Qubit we will be able to compute parallel whereas for our branching we will mask. So my intuition would suggest that in the gpu version the single qubit will surpass our cnot gate in performance.

#### GHZ State

![GHZ State](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_ghzState.png "CPU Naive GHZ State Benchmarks")

Observation: Until Qubit 16 we have a logarithmic ramp up in performance which starts to drop at Qubit 16 and then stabilizes at Qubit 21 again. 
Although GHZ State consists of N-1 Cnot gates, asymptotic we could say it consists of just Cnot Gates the performance does not match the CNOT Gate output. Being more than 10 % slower. 


Hypothesis: 
The general performance drop is uninteresting and very likely the overhead from our circuit object. It runs each gate as an Operation in a list, this incurs overhead which we did not have when we benchmarked the raw function. 
Although over 10% seems like a high number and might be worth looking into. 

The more interesting – unexpected – effect is the ramp up of the performance. My first instinct is that the H Gate Operation has slower speed and pushes the output per second down, this explains the logarithmic shape of the ramp up well. The initial overhead also takes a big toll on our performance making the lower Qubits increase in speed the more gates get chained.



### Results: Unique

There will be two benchmarks which we will introduce now but will not elaborate more in future benchmarks for the sake of avoiding redundancy.

#### GetPairIndices

![Get Pair Indices](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_getPairIndices.png "CPU Naive Get Pair indices Benchmarks")

The function is as expected stable and very likely to be computational bound. 
We operate directly through bit modifications and without branches. Thanks to the bit operations we do not see a performance degradation for pairs which are far apart (e.g., 1 and 2^n -1).


#### Apply Singe Qubit Gate Target Sweep

![Apply Singe Qubit Gate Target Sweep](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_applySingleQubitGateTargetSweep.png "CPU Naive Apply Singe Qubit Gate Target Sweep Benchmarks")

Explanation: This Benchmark builds various States of different Sizes and checks whether different target Bits influence operation speeds. 

Original Hypothesis: Yes there will be an influence, 
high target bit -> amplitudes further apart in memory -> two accesses to separate cache lines. Since the Amplitudes do not lie next to each other in memory, we can not access them with one cache-line, needing two which reduces our speed since we have to wait until both arrive.

The original hypothesis is twisted. The further apart in memory the amplitudes are, the faster the function goes. We can see that for a 24 Qubit State with Target Bit 0 we get 16GB/s. But interestingly enough this degradation completely vanishes after we increase the target bit to 10 and higher. Here we are able to reach the computation bounds, just like we did with the l1 to l3 caches. 

Possible Explanation: The processor prefetches amplitudes to queue them to be processed. If the pairs are next to each other the processor will prefetch the whole page. If we have two far apart pairs the processor will cause parallel memory access to get the two respective pages.  Essentially doubling our lines in flight and hiding the dram latency. 


### Conclusion Initial Benchmark

The most extreme performance gain would be achieved by enabling more cores. We are nowhere near our maximal memory bandwidth and should be able to at least double our performance here. 

Afterward it would be worth to see if we can remove arithmetic throttle necks, since we tend to be computational to bound rather than memory bound. 
At last, we should look closer into our prefetching and cache misses to hide dram latencies. 



### Reproduction of this Benchmark 

Prerequisites: Linux (the run is pinned with `taskset`, from util-linux), a C++20
compiler, CMake >= 3.25, Python 3, and network access on the first configure —
googletest, nlohmann/json and google/benchmark are all fetched by FetchContent at
pinned versions.

1. Python environment, needed only for the plots:

       python3 -m venv .venv
       .venv/bin/pip install -r requirements.txt

2. Configure Release with benchmarks enabled. Without `QSIM_BENCHMARK=ON` the
   `bench` target does not exist at all:

        cmake --preset benchmark                        (once, to configure)
        cmake --build --preset benchmark --target bench

3. Build and run. Writes `benchmarks/results/latest.json`:

       cmake --build --preset benchmark --target bench

4. Plot, and keep the run under a label:

       .venv/bin/python scripts/plot_benchmarks.py --overlay --caches --save cpu-baseline-naive

   This creates `benchmarks/results/<date>-<label>/` holding a copy of the JSON
   and every plot used in this document. The date comes from the JSON's own
   context block rather than from the clock, so re-plotting an old run still
   files it under the day it was measured.

`latest.json` and `latest/` are gitignored; only labelled runs are committed.

#### What has to match for the numbers to be comparable

The `bench` target pins to CPU 3 with `taskset -c 3`. On this machine CPUs 0-3
are P-cores (4.8 GHz, 2.5 MB private L2) and CPUs 4-7 are LPE-cores (3.7 GHz,
4 MB shared), so which core the scheduler picks changes both the clock and the
cache, and unpinned runs are not comparable. **On any other machine that
number means something different and must be changed.**

The rest of this run: 5 repetitions, aggregates only, Release (-O3 -DNDEBUG),
g++ 15.2.0, google/benchmark v1.9.2, and cpu_scaling_enabled was true (me
0.41%, which does not move the numbers materially). The `context` block inside
`results.json` next to the plots is the authoritative record — host, date, and cache sizes all come from there, not from this document.
