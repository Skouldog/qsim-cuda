"""Simulates Bell-State and shows state of Statevector,by using Qiskit."""
from qiskit import QuantumCircuit
from qiskit.quantum_info import Statevector


# Create a new circuit with two qubits
qc = QuantumCircuit(2)

# Add a Hadamard gate to qubit 0
qc.h(0)

# Perform a controlled-X gate on qubit 1, controlled by qubit 0
qc.cx(0, 1)


#returns calculated Vector 
sv = Statevector(qc)


# iterate thorugh sv and return amplitudes
for i, amp in enumerate(sv.data):
    print(f"|{i:02b}>  index {i}:  {amp:.4f}")