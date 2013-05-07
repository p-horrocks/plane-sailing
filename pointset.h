#ifndef POINTSET_H
#define POINTSET_H

#include <vector>

class PointSet
{
public:
    struct Point
    {
        double x;
        double y;
    };

    PointSet();

    // Points must be added in strictly increasing x order.
    void addPoint(double x, double y);

    // Determines the y valuefor a given x. If the y value is within our range
    // then the nearest (by x) known points are interpolated, otherwise the
    // points at the nearest end are extrapolated.
    double interpolate(double x) const;

    Point& operator [] (int idx) { return points_[idx]; }
    const Point& operator [] (int idx) const { return points_[idx]; }

protected:
    static double interpolate(double x1, double y1, double x2, double y2, double x);

    std::vector<Point> points_;
};

#endif // POINTSET_H
