#include "track3d.h"

#include <iostream>

#include "util.h"

Track3D::Track3D()
{
}

void Track3D::clear()
{
    xVals_.clear();
    yVals_.clear();
    zVals_.clear();
}

void Track3D::addPoint(double x, double y, double z)
{
    xVals_.push_back(x);
    yVals_.push_back(y);
    zVals_.push_back(z);
}

void Track3D::setEnd(double x, double y, double z)
{
    xVals_.resize(1);
    yVals_.resize(1);
    zVals_.resize(1);
    xVals_[0] = x;
    yVals_[0] = y;
    zVals_[0] = z;
}

void Track3D::convertAMG66toWGS84()
{
    for(size_t i = 0; i < xVals_.size(); ++i)
    {
        Point p(xVals_[i], yVals_[i]);
        p  = MGRSToUTM(p, "AGD66", "WGS84");
        p  = utmToLatLng(56, p, false);
        xVals_[i] = p.x;
        yVals_[i] = p.y;
    }
}
