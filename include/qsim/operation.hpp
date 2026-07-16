#pragma once

#include <array>
#include <complex>

#include "qsim/gates.hpp"

namespace qsim {

class Operation {
 public:
  virtual void apply(VectorState& state) const = 0;
  virtual ~Operation() {};
};

class SingleQubitGate : public Operation {
 public:
  SingleQubitGate(qsim::Matrix2 matrix, int qubit)
      : matrix(matrix), qubit(qubit) {};
  void apply(VectorState& state) const override;

 private:
  qsim::Matrix2 matrix;
  int qubit;
};

class CnotGate : public Operation {
 public:
  CnotGate(int controlBit, int targetBit)
      : controlBit(controlBit), targetBit(targetBit) {};

  void apply(VectorState& state) const override;

 private:
  int controlBit;
  int targetBit;
};
}  // namespace qsim
