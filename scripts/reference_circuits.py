from qiskit import QuantumCircuit
from qiskit.quantum_info import Statevector
import json
from pathlib import Path

#.venv/bin/python scripts/reference_circuits.py 

HERE = Path(__file__).parent

with open(HERE / "circuits.json", "r") as file:
    data = json.load(file)


for circuit in data["circuits"]:
    qc = QuantumCircuit(circuit["num_qubits"])

    for gate in circuit["gates"]:
        getattr(qc, gate["gate"])(*gate.get("params",[]), *gate["qubits"])

    sv = Statevector(qc)
    amplitudes = []
    for amp in sv.data:
        amplitudes.append([amp.real, amp.imag])

    circuit["expectedVector"] = amplitudes


with open(HERE / "reference.json", "w") as file:
    json.dump(data, file, indent=2)
