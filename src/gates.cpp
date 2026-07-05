

#include <complex>
#include <vector>



/**
 * @brief Calculate 2x2 Single Qubit
 * @param 2x2 Matrix and amplitude pair to be calculated
 * @return vector<complex> pair of new amplitudes
 */
std::vector<std::complex<double>>  singleQubitGate(
                    const std::vector<std::vector<std::complex<double>>>& matrix,
                    const std::vector<std::complex<double>>& pair
                )
{
    //Initiliaze New Vector with 0, 0 
    std::vector<std::complex<double>> newPair(2,0);
    newPair.at(0)=matrix.at(0).at(0)*pair.at(0)+matrix.at(0).at(1)*pair.at(1);
    newPair.at(1)=matrix.at(1).at(0)*pair.at(0)+matrix.at(1).at(1)*pair.at(1);

    return newPair;
   
}


//toDO all Real single bit Gates