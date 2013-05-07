#ifndef TRACK3D_H
#define TRACK3D_H

#include <vector>
#include "point3d.h"

class Track3D
{
public:
    Track3D();

    void clear();
    void addPoint(double x, double y, double z);
    void setEnd(double x, double y, double z);
    void reverse();

    bool empty() const { return xVals_.empty(); }
    size_t size() const { return xVals_.size(); }
    Point3D operator [] (size_t i) const { return Point3D(xVals_[i], yVals_[i], zVals_[i]); }

    Track3D operator + (const Track3D& o) const;

    void convertAMG66toWGS84();

protected:
    // The points are kept in separate arrays to be easier to pass to GDAL.
    std::vector<double> xVals_;
    std::vector<double> yVals_;
    std::vector<double> zVals_;
};

#endif // TRACK3D_H
