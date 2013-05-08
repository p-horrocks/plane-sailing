#include "distribution.h"

#include <iostream>

namespace
{

std::mt19937 __rand(time(NULL));

} // namespace

Distribution::Distribution(double mean, double stdDev) :
    mean_(mean), stdDev_(stdDev), norm_(mean, stdDev)
{
}

double Distribution::offsetMean(double stdDevs) const
{
    return mean_ + (stdDev_ * stdDevs);
}

double Distribution::sample() const
{
    return norm_(__rand);
}
