#include "distributionset.h"

DistributionSet::DistributionSet()
{
}

void DistributionSet::clear()
{
    items_.clear();
}

void DistributionSet::addPoint(double x, double mean, double std)
{
    Item item;
    item.x = x;
    item.y = Distribution(mean, std);
    items_.push_back(item);
}

PointSet DistributionSet::mean() const
{
    PointSet retval;
    for(auto i = items_.begin(); i != items_.end(); ++i)
    {
        retval.addPoint(i->x, i->y.mean());
    }
    return retval;
}

PointSet DistributionSet::offsetMean(double stdDevs) const
{
    PointSet retval;
    for(auto i = items_.begin(); i != items_.end(); ++i)
    {
        retval.addPoint(i->x, i->y.offsetMean(stdDevs));
    }
    return retval;
}

PointSet DistributionSet::sample() const
{
    PointSet retval;
    for(auto i = items_.begin(); i != items_.end(); ++i)
    {
        retval.addPoint(i->x, i->y.sample());
    }
    return retval;
}
