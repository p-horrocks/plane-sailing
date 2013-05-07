#include "point2d.h"

#include "util.h"

void Point2D::convertAMG66toWGS84()
{
    Point p(x_, y_);
    p  = MGRSToUTM(p, "AGD66", "WGS84");
    p  = utmToLatLng(56, p, false);
    x_ = p.x;
    y_ = p.y;
}
