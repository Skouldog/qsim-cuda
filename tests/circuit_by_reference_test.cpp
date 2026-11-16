#include <complex>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "circuit_util.hpp"
#include "gtest/gtest.h"
#include "qsim/circuit.hpp"
#include "qsim/state.hpp"
#include "test_util.hpp"

using json = nlohmann::json;

/**
 * Take a reference Json(Qiskit) and builds Circuit(cpp)
 * then compares if both get the same result.
 */
class CircuitReferenceParameterizedTestFixture
    : public ::testing::TestWithParam<json> {};

TEST_P(CircuitReferenceParameterizedTestFixture, MatchesQiskitReference) {
  const json& circuitJson = GetParam();
  qsim::VectorState state(circuitJson["num_qubits"]);
  qsim::Circuit cppCircuit = buildCircuitFromJson(circuitJson);

  cppCircuit.run(state);
  std::vector<std::complex<double>> expectedState =
      getExpectedVector(circuitJson);
  compareStates(state, expectedState, kTol);
}

INSTANTIATE_TEST_SUITE_P(CircuitReferenceTests,
                         CircuitReferenceParameterizedTestFixture,
                         ::testing::ValuesIn(getJson()["circuits"]),
                         [](const testing::TestParamInfo<json>& info) {
                           std::string name = info.param["name"];
                           return name;
                         });
