#include "distribution.h"

double Distribution::offsetMean(double stdDevs) const
{
    return mean_ + (stdDev_ * stdDevs);
}
