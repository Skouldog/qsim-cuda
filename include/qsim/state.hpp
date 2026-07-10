#pragma once
#include <complex>
#include <vector>

namespace qsim
{

    class VectorState
    {

    public:
        VectorState(std::size_t sizeQubits);
        double getProbabilityOfIndex(std::size_t index)const;
        std::complex<double> getAmplitudeOfIndex(std::size_t index)const;
        double getNorm()const;
        void restoreNorm();
        std::size_t sample(unsigned int seed)const;
        std::size_t sample()const;


         std::size_t getSize()const;
        std::size_t getQubitSize()const;
   private:
       std::vector<std::complex<double>> state;

    };
}