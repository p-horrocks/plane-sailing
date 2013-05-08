#ifndef KMLFILE_H
#define KMLFILE_H

#include <fstream>
#include <vector>
class Track3D;
class Point3D;

class KmlFile
{
public:
    KmlFile(const std::string& path);
    ~KmlFile();

    void startFolder(const std::string& name);

    void addPoint(const Point3D& pt, const char* name);
    void addTrack(const Track3D& track, const char* name, const char* style, bool useZ = true);
    void addMultiTrack(const std::vector<Track3D>& tracks, const char* name, const char* style);
    void addPolygon(const Track3D& track, const char* name, const char* style, bool useZ = true);

protected:
    std::ofstream os_;
    std::string   folderName_;
};

#endif // KMLFILE_H
