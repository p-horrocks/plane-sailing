#ifndef POINTSET_H
#define POINTSET_H

#include <vector>
#include "point2d.h"

class PointSet
{
public:
    PointSet();

    // Points must be added in strictly increasing x order.
    void addPoint(double x, double y);

    // Determines the y valuefor a given x. If the y value is within our range
    // then the nearest (by x) known points are interpolated, otherwise the
    // points at the nearest end are extrapolated.
    double interpolate(double x) const;

    Point2D& operator [] (int idx) { return points_[idx]; }
    const Point2D& operator [] (int idx) const { return points_[idx]; }

protected:
    std::vector<Point2D> points_;
};

#endif // POINTSET_H
