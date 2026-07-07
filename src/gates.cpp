

#include <complex>
#include <vector>
#include <utility>

// Forward-Declaring
std::pair<int, int> getPairIndices(int pairNumber, int qubit);
void indexPairIterator(std::vector<std::complex<double>> &stateVector, int qubit, const std::complex<double> (&matrix)[2][2]);

/**
 * @brief Calculate new pair of Amplitudes
 * @param 2x2 Matrix and amplitude pair to be calculated
 * @return  pair of new amplitudes
 */
std::pair<std::complex<double>, std::complex<double>> singleQubitGate(
    const std::complex<double> (&matrix)[2][2],
    std::pair<std::complex<double>, std::complex<double>> pairAmpOld)
{
    std::pair<std::complex<double>, std::complex<double>> pairAmpNew;

    pairAmpNew.first = matrix[0][0] * pairAmpOld.first + matrix[0][1] * pairAmpOld.second;
    pairAmpNew.second = matrix[1][0] * pairAmpOld.first + matrix[1][1] * pairAmpOld.second;

    return pairAmpNew;
}

/**
 * @brief XNOT Gate
 *@param State Vector by reference and chosen Qubit
 *@return none, the State gets manipulated in place
 */
void singleXNOTGate(
    std::vector<std::complex<double>> &stateVector,
    int qubit)
{

    std::complex<double> first(0.0, 0.0), second(1.0, 0.0), // 0 1
        third(1.0, 0.0), fourth(0.0, 0.0);                  // 1 0

    std::complex<double> matrixXNOT[2][2] = {{first, second}, {third, fourth}};

    indexPairIterator(stateVector, qubit, matrixXNOT);
}

/**
 * Helper Method
 * Iterate thorugh each pair and run singleQubitGate Function(calculate new Amps)
 */
void indexPairIterator(std::vector<std::complex<double>> &stateVector, int qubit,
                       const std::complex<double> (&matrix)[2][2])
{

    // Iterate through each pair
    for (unsigned pairNumber = 0; pairNumber < stateVector.size() / 2; pairNumber++)
    {

        std::pair<int, int> pairIndices = getPairIndices(pairNumber, qubit);

        std::pair<std::complex<double>, std::complex<double>> pairAmplitudes;

        // Get Amplitudes from state
        pairAmplitudes.first = stateVector[pairIndices.first];
        pairAmplitudes.second = stateVector[pairIndices.second];

        pairAmplitudes = singleQubitGate(matrix, pairAmplitudes);

        // write back to state
        stateVector[pairIndices.first] = pairAmplitudes.first;
        stateVector[pairIndices.second] = pairAmplitudes.second;
    }
}

/**
 * Helper Method:
 * Calculates Indices given the pairnumber
 */
std::pair<int, int> getPairIndices(int pairNumber, int qubit)
{

    // find pair of amplitudes through bitmasking
    int right = ((1 << qubit) - 1) & pairNumber;     // Bit masking 100 -> 011, only keeps bits of right from the inserted qubit.
    int left = (pairNumber >> qubit) << (qubit + 1); // lose right side bits and fill with zeros

    int pair0Index = left | right;              // e.g. Pair 01, q = 1 -> 001
    int pair1Index = pair0Index | (1 << qubit); // 011

    std::pair<int, int> pairIndices = {pair0Index, pair1Index};
    return pairIndices;
}
