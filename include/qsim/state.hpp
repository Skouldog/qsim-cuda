#pragma once
#include <complex>
#include <cstddef>
#include <vector>

namespace qsim {

class VectorState {
 public:
  VectorState(std::size_t sizeQubits);
  double getProbabilityOfIndex(std::size_t index) const;
  std::complex<double> getAmplitudeOfIndex(std::size_t index) const;
  void setAmplitudeOfIndex(std::size_t index, std::complex<double> amp);
  double getNorm() const;
  void restoreNorm();
  std::size_t sample(unsigned int seed) const;
  std::size_t sample() const;

  std::complex<double>* data() { return state.data(); }

  std::size_t getSize() const { return state.size(); }
  std::size_t getQubitSize() const;

 private:
  std::vector<std::complex<double>> state;
};
}  // namespace qsim
