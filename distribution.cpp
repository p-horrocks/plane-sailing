#include "distribution.h"

#include <random>
#include <stdexcept>

namespace {

std::mt19937 createRandomEngine()
{
    std::random_device randomDevice;
    std::seed_seq seed {
        randomDevice(), randomDevice(), randomDevice(), randomDevice(),
        randomDevice(), randomDevice(), randomDevice(), randomDevice()
    };
    return std::mt19937(seed);
}

thread_local std::mt19937 randomEngine = createRandomEngine();

} // namespace

Distribution::Distribution(double mean, double stdDev) :
    mean_(mean), stdDev_(stdDev)
{
}

double Distribution::offsetMean(double stdDevs) const
{
    return mean_ + (stdDev_ * stdDevs);
}

double Distribution::sample() const
{
    if(stdDev_ < 0.0)
        throw std::invalid_argument("Distribution standard deviation cannot be negative");

    if(stdDev_ == 0.0)
        return mean_;

    std::normal_distribution<> distribution(mean_, stdDev_);
    return distribution(randomEngine);
}
