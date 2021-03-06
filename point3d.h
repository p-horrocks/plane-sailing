#ifndef POINT3D_H
#define POINT3D_H

#include "point2d.h"

// Simple point class. If you haven't seen fify bazillion of these you haven't
// lived.
class Point3D : public Point2D
{
public:
    Point3D() {}
    Point3D(double x, double y, double z) : Point2D(x, y), z_(z) {}

    double z_;
};

// Stream operator so that points can be be output to a stream in kml format.
inline std::ostream& operator << (std::ostream& os, const Point3D& pt)
{
    os << std::fixed << std::setprecision(6) << pt.x_ << ',' << pt.y_ << ',' << std::setprecision(2) << pt.z_;
    return os;
}

#endif // POINT3D_H
