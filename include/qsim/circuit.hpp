#pragma once

#include <array>
#include <complex>
#include <list>
#include <memory>

#include "qsim/gates.hpp"
#include "qsim/operation.hpp"
#include "qsim/state.hpp"

namespace qsim {

class Circuit {
 public:
  void add(Matrix2 matrix, int qubit);

  void addCnot(int controlBit, int targetBit);

  void run(qsim::VectorState& state);

 private:
  std::list<std::unique_ptr<qsim::Operation>> circuitList;
};
}  // namespace qsim
