#ifndef KMLFILE_H
#define KMLFILE_H

#include <fstream>
class Track3D;

class KmlFile
{
public:
    KmlFile(const std::string& path);
    ~KmlFile();

    void addTrack(const Track3D& track, const char* colour);

protected:
    std::ofstream os_;
};

#endif // KMLFILE_H
