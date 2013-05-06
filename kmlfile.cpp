#include "kmlfile.h"

#include <sstream>
#include <stdexcept>
#include <cstring>

KmlFile::KmlFile(const std::string& path)
{
    os_.open(path);
    if(!os_.is_open())
    {
        std::stringstream ss;
        ss << "KML file '" << path << "' could not be opened: " << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
    }

    os_ << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    os_ << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl;
    os_ << "<Folder>" << std::endl;
//        <Style id="stylesel_0">
//            <LineStyle id="substyle_0">
//                <color>99ff7777</color>
//                <colorMode>normal</colorMode>
//                <width>10</width>
//            </LineStyle>
//        </Style>
}

KmlFile::~KmlFile()
{
    os_ << "</Folder>" << std::endl;
}

void KmlFile::addLine(const std::vector<Point3D>& line, const char* colour)
{
}
