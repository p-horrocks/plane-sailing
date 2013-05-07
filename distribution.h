#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

class Distribution
{
public:
    Distribution(double mean, double stdDev) : mean_(mean), stdDev_(stdDev) {}

    double mean() const { return mean_; }

protected:
    double mean_;
    double stdDev_;
};

#endif // DISTRIBUTION_H
