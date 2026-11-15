#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>

#include "qsim/circuit.hpp"
#include "qsim/gates.hpp"

using json = nlohmann::json;

json getJson();

std::vector<std::complex<double>> getExpectedVector(const json& circuit);

qsim::Circuit buildCircuitFromJson(const json& circuitJson);

const std::unordered_map<std::string, qsim::Matrix2> gateMap = {
    {"x", qsim::gates::x()},
    {"h", qsim::gates::h()},
    {"z", qsim::gates::z()},
    {"t", qsim::gates::t()},
    {"s", qsim::gates::s()}};
