#include <complex>
#include <iostream>
#include <cstddef>
#include "test_util.hpp"



int failures = 0;




void expectClose(std::complex<double> got, std::complex<double> want,
                 double tol, const std::string& msg) {
    if (std::abs(got - want) > tol) {
        std::cerr << "FAIL: " << msg << " got=" << got << " want=" << want << "\n";
        ++failures;
    }
}

void expectDifferent(std::complex<double> first, std::complex<double> second,
                 double tol, const std::string& msg) {
    if (std::abs(first - second) < tol) {
        std::cerr << "FAIL: " << msg << " first=" << first << " second=" << second << "\n";
        ++failures;
    }
}



void expectTrue(bool cond, const std::string& label) {
    if (!cond) {
        std::cerr << "FAIL: " << label << "\n";
        ++failures;
    }
}