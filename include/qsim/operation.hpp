#pragma once

#include <array>
#include <complex>

#include "qsim/gates.hpp"

namespace qsim {

class Operation {
 public:
  virtual void apply(VectorState& state) const = 0;
#ifdef QSIM_USE_CUDA
  virtual void applyGpu(VectorState& state, std::complex<double>* devPtr) const = 0;
#endif
  virtual ~Operation() {};
};

class SingleQubitGate : public Operation {
 public:
  SingleQubitGate(qsim::Matrix2 matrix, int qubit)
      : matrix(matrix), qubit(qubit) {};
  void apply(VectorState& state) const override;
#ifdef QSIM_USE_CUDA
  void applyGpu(VectorState& state, std::complex<double>* devPtr) const override;
#endif
 private:
  qsim::Matrix2 matrix;
  int qubit;
};

class CnotGate : public Operation {
 public:
  CnotGate(int controlBit, int targetBit)
      : controlBit(controlBit), targetBit(targetBit) {};

  void apply(VectorState& state) const override;
#ifdef QSIM_USE_CUDA
  void applyGpu(VectorState& state,
                std::complex<double>* devPtr) const override;
#endif
 private:
  int controlBit;
  int targetBit;
};

}  // namespace qsim
