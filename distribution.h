#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

class Distribution
{
public:
    Distribution() : mean_(1e99), stdDev_(1e99) {}
    Distribution(double mean, double stdDev);

    bool isNull() const { return (mean_ == 1e99) && (stdDev_ == 1e99); }
    double mean() const { return mean_; }
    double offsetMean(double stdDevs) const;

    // Generate a random sample using gaussian distribution.
    double sample() const;

protected:
    double mean_;
    double stdDev_;
};

#endif // DISTRIBUTION_H
