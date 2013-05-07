#include "track3d.h"

#include <iostream>
#include <algorithm>

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

void Track3D::reverse()
{
    std::reverse(xVals_.begin(), xVals_.end());
    std::reverse(yVals_.begin(), yVals_.end());
    std::reverse(zVals_.begin(), zVals_.end());
}

Track3D Track3D::operator + (const Track3D& o) const
{
    Track3D retval;
    retval.xVals_ = xVals_;
    retval.xVals_.insert(retval.xVals_.end(), o.xVals_.begin(), o.xVals_.end());
    retval.yVals_ = yVals_;
    retval.yVals_.insert(retval.yVals_.end(), o.yVals_.begin(), o.yVals_.end());
    retval.zVals_ = zVals_;
    retval.zVals_.insert(retval.zVals_.end(), o.zVals_.begin(), o.zVals_.end());
    return retval;
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
