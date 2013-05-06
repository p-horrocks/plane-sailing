#include "track3d.h"

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
