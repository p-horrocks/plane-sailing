#ifndef KMLFILE_H
#define KMLFILE_H

#include <fstream>
#include "util.h"

class KmlFile
{
public:
    KmlFile(const std::string& path);
    ~KmlFile();

    void addLine(const std::vector<Point3D>& line, const char* colour);

protected:
    std::ofstream os_;
};

#endif // KMLFILE_H
