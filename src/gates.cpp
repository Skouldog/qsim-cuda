#include <complex>
#include <vector>
#include <utility>
#include <cstddef>
#include "qsim/gates.hpp"
#include "qsim/state.hpp"

namespace qsim
{
    // Forward-Declaring
    std::pair<std::size_t, std::size_t> getPairIndices(
        std::size_t pairNumber, int qubit);

     std::pair<std::complex<double>, std::complex<double>> calculateAmplitudes
     (const std::complex<double> (&matrix)[2][2], std::pair<std::complex<double>, std::complex<double>> pairAmpOld);

    

    /**
     *@brief applies 2x2 Matrix to Qubit
     *@param State Vector by reference, chosen qubit and 2x2 matrix to be calculated with
     *@return none, the State gets manipulated in place
     */
    void applySingleQubitGate(
       qsim::VectorState& stateVector,
        int qubit,
        const std::complex<double> (&matrix)[2][2])
    {

        std::complex<double>* ptrStateVector = stateVector.data();


        // Iterate through each pair
        for (std::size_t pairNumber = 0; pairNumber < stateVector.getSize() / 2; pairNumber++)
        {

            std::pair<std::size_t, std::size_t> pairIndices = getPairIndices(pairNumber, qubit);

            std::pair<std::complex<double>, std::complex<double>> pairAmplitudes;

            // Get Amplitudes from state
            pairAmplitudes.first = ptrStateVector[pairIndices.first];
            pairAmplitudes.second = ptrStateVector[pairIndices.second];

            pairAmplitudes = calculateAmplitudes(matrix, pairAmplitudes);

            // write back to state
            ptrStateVector[pairIndices.first] = pairAmplitudes.first;
            ptrStateVector[pairIndices.second] = pairAmplitudes.second;
        }
    }

    /**
     * @brief Helper Method: Calculates Indices given the pairnumber
     * @param Pairnumber, chosen Qubit
     * @return Indices for certain pair Number
     */
    std::pair<std::size_t, std::size_t> getPairIndices(std::size_t pairNumber, int qubit)
    {

        // find pair of amplitudes through bitmasking
        std::size_t right = ((std::size_t{1} << qubit) - 1) & pairNumber;     // Bit masking 100 -> 011, only keeps bits of right from the inserted qubit.
        std::size_t left = (pairNumber >> qubit) << (qubit + 1); // lose right side bits and fill with zeros

        std::size_t pair0Index = left | right;              // e.g. Pair 01, q = 1 -> 001
        std::size_t pair1Index = pair0Index | (std::size_t{1} << qubit); // 011

        std::pair<std::size_t, std::size_t> pairIndices = {pair0Index, pair1Index};
        return pairIndices;
    }

    /**
     * 
     * @brief Helper: Calculate new pair of Amplitudes
     * @param 2x2 Matrix and amplitude pair to be calculated
     * @return  pair of new amplitudes
     */
    std::pair<std::complex<double>, std::complex<double>> calculateAmplitudes(
        const std::complex<double> (&matrix)[2][2],
        std::pair<std::complex<double>, std::complex<double>> pairAmpOld)
    {
        std::pair<std::complex<double>, std::complex<double>> pairAmpNew;

        pairAmpNew.first = matrix[0][0] * pairAmpOld.first + matrix[0][1] * pairAmpOld.second;
        pairAmpNew.second = matrix[1][0] * pairAmpOld.first + matrix[1][1] * pairAmpOld.second;

        return pairAmpNew;
    }
}