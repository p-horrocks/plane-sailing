#include "pointset.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "util.h"

namespace
{

class CompareX
{
public:
    bool operator () (const PointSet::Point& p1, const PointSet::Point& p2) const
    {
        return p1.x < p2.x;
    }
};

} // namspace

PointSet::PointSet()
{
}

void PointSet::addPoint(double x, double y)
{
    // Points must be added in x order.
    assert(points_.empty() || (x > points_.back().x));

    Point p = { x, y };
    points_.push_back(p);
}

double PointSet::interpolate(double x) const
{
    Point p = { x, 0 };
    auto i = std::lower_bound(points_.begin(), points_.end(), p, CompareX());
    if(i == points_.end())
    {
        i = points_.end() - 1;
    }
    else if(i->x == x)
    {
        return i->y;
    }
    else if(i == points_.begin())
    {
        ++i;
    }
    else
    {
    }

    const Point& p1 = *(i - 1);
    const Point& p2 = *i;
    return interpolate(p1.x, p1.y, p2.x, p2.y, x);
}

double PointSet::interpolate(double x1, double y1, double x2, double y2, double x)
{
    return (y2 - y1) / (x2 - x1) * (x - x1) + y1;
}
