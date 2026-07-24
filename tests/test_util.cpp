#include "test_util.hpp"

#include <complex>
#include <vector>

#include "gtest/gtest.h"
#include "qsim/state.hpp"

void compareStates(qsim::VectorState& got,
                   const std::vector<std::complex<double>>& want, double tol) {
  std::complex<double>* ptrStateVector = got.data();
  ASSERT_EQ(got.getSize(), want.size());

  for (std::size_t i = 0; i < got.getSize(); i++) {
    EXPECT_NEAR(std::abs(ptrStateVector[i] - want.at(i)), 0.0, tol);
  }
}
