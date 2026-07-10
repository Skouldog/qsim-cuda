#pragma once
#include <complex>
#include <string>

extern int failures;

void expectClose(std::complex<double> got, std::complex<double> want,
                 double tol, const std::string& msg);

void expectDifferent(std::complex<double> first, std::complex<double> second,
                     double tol, const std::string& msg);

void expectTrue(bool cond, const std::string& label);
