# Architecture


```mermaid
classDiagram
    direction TB

    class Circuit {
        -circuitList : list of unique_ptr~Operation~
        +add(Matrix2 matrix, int qubit)
        +addCnot(int controlBit, int targetBit)
        +run(VectorState& state)
    }

    class Operation {
        <<abstract>>
        +apply(VectorState& state)*
    }

    class SingleQubitGate {
        -matrix : Matrix2
        -qubit : int
        +apply(VectorState& state)
    }

    class CnotGate {
        -controlBit : int
        -targetBit : int
        +apply(VectorState& state)
    }

    class GateAppliers {
        <<functions>>
        +applySingleQubitGate(VectorState& state, int qubit, Matrix2 matrix)
        +applyCnotGate(VectorState& state, int controlBit, int targetBit)
    }

    class detail {
        <<namespace>>
        +getPairIndices(size_t pairNumber, int qubit)
        +calculateAmplitudes(Matrix2 matrix, pair oldAmps)
    }

    class VectorState {
        -amplitudes : vector~complex~double~~
        +data()
        +getSize() size_t
        +getAmplitudeOfIndex(size_t index)
        +sample(unsigned seed) size_t
    }

    Circuit "1" *-- "*" Operation : owns
    Operation <|-- SingleQubitGate : inherits
    Operation <|-- CnotGate : inherits
    SingleQubitGate ..> GateAppliers : calls applySingleQubitGate
    CnotGate ..> GateAppliers : calls applyCnotGate
    GateAppliers ..> detail : uses
    GateAppliers ..> VectorState : reads and writes amplitudes via data
    Circuit ..> VectorState : run borrows it by reference
```



## Decisions: 


In order to implement our GPU Backend we need to adjust some of our architecture.

In order to avoid long data copying we will only copy the vector state once from the host to the device. This will happen at the beginning of our `circuit.run()`. We manipulate the state on the device and only copy the state back once we have completed the computations. 

We do not have to change much of our current architecture, but since we initialize the device pointer in our circuit we will have to pass it down through our functions, so our kernel can manipulate our data.
