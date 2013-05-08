#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <random>

class Distribution
{
public:
    Distribution() {}
    Distribution(double mean, double stdDev);

    double mean() const { return mean_; }
    double offsetMean(double stdDevs) const;

    // Generate a random sample using gaussian distribution.
    double sample() const;

protected:
    double mean_;
    double stdDev_;
    mutable std::normal_distribution<> norm_;
};

#endif // DISTRIBUTION_H
