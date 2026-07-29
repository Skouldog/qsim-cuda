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

![Single Qubit Gate](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_applySingleQubitGate.png "CPU Naive Single Qubit Gate Benchmarks")

Observation
Through out L1 to L3 the throughput seems stable, this suggest that the function is not memory bound rather computation bound. It is up to 8 times slower than our roofline function.

Interestingly enough there is a steep drop at the 20 Qubit mark, this is the relocation of the state to DRAM. 

THis would be a good first look for any optimization. We can tell from the roofline that our memory bound ist at roughly 38 GB/s and our computational for the SingleQubitGate lies at roughly 22 GB/s. 
Why is the current throughput 16 GB/s instead of 22GB/s because  the slower dram latency introduces cpu stalls. The Cpu works through the instructions faster than dram can supply new values

#### CNOT QUbit Gate

![CNOT Qubit Gate](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_applyCnotGate.png "CPU Naive CNOT Qubit Gate Benchmarks")


Observation: First again see a computational bound for the L1 and L2 Cache, here the performance degradation happens earlier as the thorughput already starts to decrease in transtiton to L3 Cache and onwards. 

One strange observation is the higher thorughput of the CNOT Gate compared to our singleQubitGate. The seemingly more complex Cnot gate costs apparently less then our singlequbit gate. making it up to 3x faster. 

Explantion: 
the drop probably stems from the same cause as in our single qubit gate. The high latencys cant be hidden by our CPU anymore. Since our operational speed is alot higher than with the single qubit gate, this happens one memory stage earlier and is visible even with the L3 Cache.

Looking closer at our functins the speed difference is quite obvious. Cnot having the branching logik, eleminating alot of operations and the follow through being just a simple amlpitude swap without any computations compared to the heavy matrix artihtmeics in our singleQubitGate. This might be worth looking at after the GPU optimization, since with the single Qubit we will be able to compute parralel whereas for our branching we will mask. So my intution would suggest that in the gpu version the single qubit will surpase our cnot gate in performance.

#### GHZ State

![GHZ State](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_ghzState.png "CPU Naive GHZ State Benchmarks")

Observation: Until we Qubit 13 we have a ramp up in performance which plateus and then starts to drop at Qubit 16 and then stabilizes at Qubit 21 again. 
Although GHZ State consists basically of N-1 Cnot gates, asymptotic we could say it consist of just Cnot Gates the performance doesnt match the CNOT Gate output. Being more than 10 % slower. 


Hypothesis: 
The general is uninteresting and very likely the overhead from our circuit object. It runs each gate as an Operation in a list, this incurs overhead which we didnt have when we benchmarked the raw function. 
Although over 10% seems like a high number and might be worth looking into. 

The more interesting unexpected effect is the ramp up of the performance. My first instinct is that the H Gate Operation has slower speed and pushes the ouput per second down, this explains the logarithmic shape of the ramp up well. The initial overhead also takes a big toll on our performance making the lower qubits increase in speed the more gates get chained.



### Results: Unique

There has been two benchmarks which i will introduce now but will not elaborate more in future benchmakrs for the sake of redundancy

#### GetPairIndices

![Get Pair Inidices](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_getPairIndices.png "CPU Naive Get Pair indices Benchmarks")

The function is as expected stable and very likely to be computational bound. 
We operate directly thorugh bit modifacations and without branches. Thanks to the bit operations we dont see a performance degradationn for pairs which are far apart ( eg. 1 and 2^n -1 ).


#### Apply Singe Qubit Gate Target Sweep

![Apply Singe Qubit Gate Target Sweep](/benchmarks/results/2026-07-28-cpu-baseline-naive/BM_applySingleQubitGateTargetSweep.png "CPU Naive Apply Singe Qubit Gate Target Sweep Benchmarks")

Explanation: This Benchmark build various State of different Size and then checked wether different target Bits would influence operation speeds. 

Origianl Hypothesis: Yes there will be an influence, since the Amplitudes wont lie next to each other im memory, the higher the target bit -> the furter apart in memory -> the slower the function gets.

THe original hypothesis is twisted. The further apart in memory the amplitudes are the faster the function goes. We can see that for a 24 qubit state we get only 16GB/s for target 0, just like we did in our applySinge Qubit benchmark. But interestingly enough this degradation completly vanishes after we increase the target bit to 10 and higher. Here we are able to reach the computation bounds liek we did with the l1 to l3 caches. 

possible Explanation: The porcessor prefecthes amps to queue up to be proccessed. if the pairs are next to each other the processor will prefetch the whole page from those pairs resulting in a 4KB prefetch. If we have two far apart pairs the proccesor will acces the two parallel prefecthicn their respective pages. prefecthing 8 KB in total, essentially hiding the dram latency again. 


### Conclusion Inital Benchmakr

The most extreme perfomance gain would be achieved by enabling more cores. We are no where near our maximal memory bandwidht and should be able to atleast double our perfomance here. 

Afterwards it would be worth to see if we can remove arithmetic throttle necks. since we tend to be computational bound rather than memory bound. 
at last i would look closer into the our prefecthing and misses to try and hide dram latencies. 



### Reproduction of this Benchmark 

to do and look at dram latency again. 
