#include <iostream>

#include "gates_test.hpp"
#include "state_test.hpp"
#include "test_util.hpp"

int main() {
  runGateTests();
  runGetPairIndicesTest();
  runStateTests();

  if (failures == 0) std::cout << "all tests passed\n";
  return failures;
}
