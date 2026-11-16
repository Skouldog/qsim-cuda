#include "circuit_util.hpp"

#include <complex>
#include <cstddef>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "qsim/circuit.hpp"
#include "qsim/gates.hpp"

using json = nlohmann::json;

json getJson() {
  std::string fileDir = TEST_RESOURCE_DIR;
  std::string path = fileDir + "reference.json";

  std::ifstream fileStream(path);
  if (!fileStream.is_open()) {
    throw std::runtime_error("Broken Path: " + path);
  }

  json jfile = json::parse(fileStream);

  return jfile;
}

std::vector<std::complex<double>> getExpectedVector(const json& circuit) {
  std::vector<std::complex<double>> expectedAmps(
      circuit["expectedVector"].size());

  for (std::size_t i = 0; i < circuit["expectedVector"].size(); i++) {
    double re = circuit["expectedVector"][i][0];
    double imag = circuit["expectedVector"][i][1];

    expectedAmps.at(i) = {re, imag};
  }

  return expectedAmps;
}

qsim::Circuit buildCircuitFromJson(const json& circuitJson) {
  qsim::Circuit cppCircuit;

  for (const auto& gate : circuitJson["gates"]) {
    std::string gateType = gate["gate"];
    if (gateType == "cx") {
      cppCircuit.addCnot(gate["qubits"][0], gate["qubits"][1]);
    } else if (gateType == "rz") {
      cppCircuit.add(qsim::gates::rz(gate["params"][0]), gate["qubits"][0]);
    } else {
      cppCircuit.add(gateMap.at(gateType), gate["qubits"][0]);
    }
  }
  return cppCircuit;
}
