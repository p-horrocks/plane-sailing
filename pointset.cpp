#include "pointset.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>

#include "util.h"

namespace
{

class CompareX
{
public:
    bool operator () (const Point2D& p1, const Point2D& p2) const
    {
        return p1.x_ < p2.x_;
    }
};

} // namspace

PointSet::PointSet()
{
}

void PointSet::addPoint(double x, double y)
{
    if(!points_.empty() && (x <= points_.back().x_))
        throw std::runtime_error("Points added in incorrect order");

    points_.push_back(Point2D(x, y));
}

double PointSet::interpolate(double x) const
{
    if(points_.size() == 2)
    {
        // Only two points in the set. Just interpolate these.
        const Point2D& p1 = points_.front();
        const Point2D& p2 = points_.back();
        return (p2.y_ - p1.y_) / (p2.x_ - p1.x_) * (x - p1.x_) + p1.y_;
    }
    else
    {
        // Find the two points on either side (x-wise) of the requested x and
        // interpolate between those.
        Point2D p(x, 0);
        auto i = std::lower_bound(points_.begin(), points_.end(), p, CompareX());
        if(i == points_.end())
        {
            i = points_.end() - 1;
        }
        else if(i->x_ == x)
        {
            return i->y_;
        }
        else if(i == points_.begin())
        {
            ++i;
        }
        else
        {
        }

        const Point2D& p1 = *(i - 1);
        const Point2D& p2 = *i;
        return (p2.y_ - p1.y_) / (p2.x_ - p1.x_) * (x - p1.x_) + p1.y_;
    }
}
