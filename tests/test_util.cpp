#include <complex>
#include <iostream>
#include "test_util.hpp"



int failures = 0;




void expectClose(std::complex<double> got, std::complex<double> want,
                 double tol, const std::string& msg) {
    if (std::abs(got - want) > tol) {
        std::cerr << "FAIL: " << msg << " got=" << got << " want=" << want << "\n";
        ++failures;
    }
}


void expectTrue(bool cond, const std::string& label) {
    if (!cond) {
        std::cerr << "FAIL: " << label << "\n";
        ++failures;
    }
}