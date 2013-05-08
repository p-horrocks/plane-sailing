#include "point2d.h"

#include "util.h"

void Point2D::convertAMG66toWGS84()
{
    *this = MGRSToUTM(*this, "AGD66", "WGS84");
    *this = utmToLatLng(56, *this, false);
}

Point2D Point2D::operator + (const Point2D& o) const
{
    return Point2D(x_ + o.x_, y_ + o.y_);
}
