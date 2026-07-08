#include <complex>
#include <vector>
#include <cmath>
#include <numeric>
#include <random>

namespace qsim
{

    class VectorState
    {
    public:
        std::vector<std::complex<double>> state;

        VectorState(std::size_t sizeQubits) : state((size_t{1} << sizeQubits)) { state.at(0) = {1, 0}; }

        /**
         * @brief Returns Probability of specific index
         * @return double
         */
        double getProbabilityOfIndex(std::size_t index) const
        {
            return std::norm(state.at(index));
        }

        /**
         * @brief Returns Amplitude of specific index
         * @return complex <double>
         */
        std::complex<double> getAmplitudeOfIndex(std::size_t index) const
        {
            return state.at(index);
        }

        /**
         * @brief Returns Norm of Vector(should be 1)
         */
        double getNorm() const
        {
            double sum = 0;
            for (const auto &amp : state)
            {
                sum += std::norm(amp);
            }
            return std::sqrt(sum);
        }

        /**
         * @brief Restore Norm to 1, to fix rounding errors
         */
        void restoreNorm()
        {

            double norm = getNorm();
            if (norm < 1e-15)
                return; // Catch 0 Vektor

            for (auto &amp : state)
            {
                amp = amp / norm;
            }
        }

        /**
         * @brief Returns random Index based on Vector probabilities
         * @param seed for random generation
         * @return Random Index
         */

        std::size_t sample(unsigned int seed) const
        {

            std::mt19937 gen(seed);

            double randomNumber = std::generate_canonical<double, std::numeric_limits<double>::digits>(gen);

            std::size_t indexState = 0;
            double probability = 0;
            for (const auto &amp : state)
            {
                probability += std::norm(amp);
                if (randomNumber < probability)
                {
                    return indexState;
                }
                indexState++;
            }
            return state.size() - 1; // Fallback to last Index in Case of rounding Errors
        }

        /**
         * @overload
         */
        std::size_t sample() const
        {

            std::random_device rd;

            unsigned int seed = rd();

            return sample(seed);
        }
    };

}