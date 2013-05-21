#ifndef DISTRIBUTIONSET_H
#define DISTRIBUTIONSET_H

#include "pointset.h"
#include "distribution.h"

class DistributionSet
{
public:
    DistributionSet();

    void clear();
    void addPoint(double x, double mean, double std);

    PointSet mean() const;
    PointSet offsetMean(double stdDevs) const;
    PointSet sample() const;

protected:
    struct Item
    {
        double x;
        Distribution y;
    };
    std::vector<Item> items_;
};

#endif // DISTRIBUTIONSET_H
