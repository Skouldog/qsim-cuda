#include <complex>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "gtest/gtest.h"
#include "qsim/circuit.hpp"
#include "qsim/gates.hpp"
#include "qsim/state.hpp"
#include "test_util.hpp"

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
const std::unordered_map<std::string, qsim::Matrix2> gateMap = {
    {"x", qsim::gates::x()},
    {"h", qsim::gates::h()},
    {"z", qsim::gates::z()},
    {"t", qsim::gates::t()},
    {"s", qsim::gates::s()}};

/**
 * Take A reference Json(Qiskit) and builds Circuit(cpp)
 * then compares if both get the same result.
 */
class CircuitReferenceParameterizedTestFixture
    : public ::testing::TestWithParam<json> {};

TEST_P(CircuitReferenceParameterizedTestFixture, MatchesQiskitReference) {
  const json& circuit = GetParam();

  qsim::VectorState state(circuit["num_qubits"]);
  qsim::Circuit cppCircuit;

  for (const auto& gate : circuit["gates"]) {
    std::string gateType = gate["gate"];
    if (gateType == "cx") {
      cppCircuit.addCnot(gate["qubits"][0], gate["qubits"][1]);
    } else if (gateType == "rz") {
      cppCircuit.add(qsim::gates::rz(gate["params"][0]), gate["qubits"][0]);
    } else {
      cppCircuit.add(gateMap.at(gateType), gate["qubits"][0]);
    }
  }

  cppCircuit.run(state);
  std::vector<std::complex<double>> expectedState = getExpectedVector(circuit);
  compareStates(state, expectedState, kTol);
}

INSTANTIATE_TEST_SUITE_P(CircuitReferenceTests,
                         CircuitReferenceParameterizedTestFixture,
                         ::testing::ValuesIn(getJson()["circuits"]),
                         [](const testing::TestParamInfo<json>& info) {
                           std::string name = info.param["name"];
                           return name;
                         });
