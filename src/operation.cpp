#include "qsim/operation.hpp"

#include "qsim/gates.hpp"

namespace qsim {  // namespace qsim

void SingleQubitGate::apply(VectorState& state) const {
  qsim::applySingleQubitGate(state, qubit, matrix);
}

void CnotGate::apply(VectorState& state) const {
  qsim::applyCnotGate(state, controlBit, targetBit);
}

}  // namespace qsim
