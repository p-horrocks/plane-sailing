#ifndef KMLFILE_H
#define KMLFILE_H

#include <fstream>
class Track3D;
class Point3D;

class KmlFile
{
public:
    KmlFile(const std::string& path);
    ~KmlFile();

    void startFolder(const std::string& name);

    void addPoint(const Point3D& pt, const char* name);
    void addTrack(const Track3D& track, const char* name, const char* colour);
    void addPolygon(const Track3D& track, const char* name, const char* colour);

protected:
    std::ofstream os_;
    std::string   folderName_;
};

#endif // KMLFILE_H
