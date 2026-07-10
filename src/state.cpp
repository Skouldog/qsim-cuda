#include <complex>
#include <vector>
#include <cmath>
#include <numeric>
#include <random>
#include <cstddef>
#include "qsim/state.hpp"

namespace qsim
{


    

        VectorState::VectorState(std::size_t sizeQubits) : state((size_t{1} << sizeQubits)) { state.at(0) = {1, 0}; }

        /**
         * @brief Returns Probability of specific index
         * @return double
         */
        double VectorState::getProbabilityOfIndex(std::size_t index) const
        {
            return std::norm(state.at(index));
        }

        /**
         * @brief Returns Amplitude of specific index
         * @return complex <double>
         */
        std::complex<double> VectorState::getAmplitudeOfIndex(std::size_t index) const
        {
            return state.at(index);
        }

        /**
         * @brief Returns Norm of Vector(should be 1)
         */
        double VectorState::getNorm() const
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
        void VectorState::restoreNorm()
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

        std::size_t VectorState::sample(unsigned int seed) const
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
        std::size_t VectorState::sample() const
        {

            std::random_device rd;

            unsigned int seed = rd();

            return sample(seed);
        }

        std::size_t VectorState::getSize()const { return state.size(); }
        std::size_t VectorState::getQubitSize()const { return std::log2(state.size()); }
    }

