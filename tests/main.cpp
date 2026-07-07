#include <iostream>
#include "test_util.hpp"        
#include "gates_test.hpp"  


int main()
{
 
    runGateTests();
    if (failures == 0) std::cout << "all tests passed\n";
   return failures;
}