#ifndef POINT2D_H
#define POINT2D_H

#include <ostream>
#include <iomanip>

class Point2D
{
public:
    Point2D() {}
    Point2D(double x, double y) : x_(x), y_(y) {}

    void convertAMG66toWGS84();

    double x_;
    double y_;
};

// Stream operator so that points can be be output to a stream in kml format.
inline std::ostream& operator << (std::ostream& os, const Point2D& pt)
{
    os << std::fixed << std::setprecision(6) << pt.x_ << ',' << pt.y_;
    return os;
}

#endif // POINT2D_H
