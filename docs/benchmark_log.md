# Benchmark Insights


## Hardware Rooflines

### Theoretical Limits: [Intel(R) Core(TM) Ultra 7 258V, 8 Cores](https://www.intel.com/content/www/us/en/products/sku/240957/intel-core-ultra-7-processor-258v-12m-cache-up-to-4-80-ghz/specifications.html)

All the CPU Benchmarks are being run on this CPU

Maximum Bandwidth = DRAM bandwidth * Bus size /8 / 1000
Maximum Bandwidth =  8533 MT/s * 128 bit bus width / 8 / 1000 = <b>136.528 GB/s</b>

 Looking at our State representation. A Vector of complex<double>. Each amplitude needs 16 Byte. 
 This means for n qubits we need 2^n * 16 Bytes. 

 Hypothesis: 
 L1 = 48KiB = 48 * 1024 = 49152 Bytes

 Solve: 
 2^n* 16 = 49152 for n 
 -> n = 11.5 
 All states with 11 Qubits and less should fit in our L1 Cache.

 Analog:

 L2 -> n = 17.3 
 L3 -> n = 19.6

DRAM(32GiB) n = 31: This is a theoretical since the OS itself needs DRAM and wouldnt be able to spend all to just hold the State, this would result in a crash of the program and/or Computer.


### Practical Limits: 

To have a roofline of this exact machine all benchmarks are also compared to simple arithmetic function. 
We read the state and multiply by double and write the amplitudes. This is one of the most rudementary operations and should provide a good limit for our hardware

 ![Roofline Benchmark](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_roofline.png "Loop Iterating over a state with read and write")

Observation


Our proccessed data / s is far from the theoretical bandwidth in both directions.

Overprocessing (Qubtis 1-14):
The memory bandwidth limit is calculated from the DRAM. Since the States are so small they can fit entirely into L3 cahces or lower. The theoratical limit doesnt apply since DRAM is never in contact for any data exchange. 



Another note worthy anomaly is the irregaular curve from Qubit size 1 till 10. This is likely to becaused, by the overhead from the loop and google benchmark. 


Heavy Underproccesing(Qubits 20+)
One reason is the fact that we are opertaing a single core which is not able to fill the whole memory bandwidth, this could be solved with a multi core implementation. Which will be the focus for our GPU Implementation.

Wrong Theory:

More interestingly our predicted performance drops happen later than anticipated. 

| Cache Switch (from/to)| Predicted Drop | Actual Drop | 
| -------- | -------         |------       |
| L1->L2       | 11.5            |        14     |
| L2->L3       | 17.3            |        18     |
| L3->DRAM       | 19.6            |        21     |

Possible Explanation: 
The original hypothesis assumed that as soon the state doesnt fit in one Cache it gets transfered as whole to the next higher level. The data suggest a different approach. 
The state gets partitially fitted in a Cache Level, which results in only partial cache misses. This creates a somewhat gradual perforamnce decrease until the majority of the state lives in the next cache. this also suggest, that every non arbitrary state lives in multiple cache leves.

Keeping in Mind:
L2 as well as L3 Cache are unified, so the instructions from out google benchmark loop will also occupy space. 

More important the L3 Cache is shared amongst all 8 Cores, meaning the OS and outside applications will occupy space aswell, leaving less for the state. 







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

### Methodoligy

This is the first naive CPU implementation without further optimization.

There are 6 families, all exclude State and Circuit Constructions, shifting the focus of the heavy calculations and memory transfers.


 - singel qubit gate: Runs H Gate on |0...0> State
 - cnot qubit gate:  Runs Cnot Gate on |0...0> State
 // TODO explain other families and explain their benchmarks


### Results

#### Sinlge Qubit Gate

![Single Qubit Gate](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_applySingleQubitGate.png "CPU Naive Singe Qubit Gate Benchmarks")

Observation
Through out L1 to L3 the throughput seems stable, this suggest that the function is not memory bound rather computation bound. It is up to 8 times slower than our roofline function.

Interestingly enough there is a steep drop at the 20 Qubit mark, this is the relocation of the state to DRAM. 

THis would be a good first look for any optimization. We can tell from the roofline that our memory bound ist at roughly 38 GB/s and our computational for the SingleQubitGate lies at roughly 22 GB/s. 
Why is the current throughput 16 GB/s instead of 22GB/s because  the slower dram latency introduces cpu stalls. The Cpu works through the instructions faster than dram can supply new values

