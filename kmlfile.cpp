#include "kmlfile.h"

#include <sstream>
#include <stdexcept>
#include <cstring>

#include "track3d.h"

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
    os_ << "  <Document>" << std::endl;
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
    os_ << "  </Document>" << std::endl;
    os_ << "</kml>" << std::endl;
}

void KmlFile::addPoint(const Point3D& pt, const char* name)
{
    os_ << "    <Placemark>" << std::endl;
    os_ << "      <name>" << name << "</name>" << std::endl;
    os_ << "      <Point>" << std::endl;
    os_ << "        <coordinates>" << (Point2D)pt << "</coordinates>" << std::endl;
    os_ << "      </Point>" << std::endl;
    os_ << "    </Placemark>" << std::endl;
}

void KmlFile::addTrack(const Track3D& track, const char* name, const char* colour)
{
    os_ << "    <Placemark>" << std::endl;
    os_ << "      <name>" << name << "</name>" << std::endl;
    os_ << "      <LineString>" << std::endl;
    os_ << "        <LineStyle>" << std::endl;
    os_ << "          <color>" << colour << "</color>" << std::endl;
    os_ << "          <width>1</width>" << std::endl;
    os_ << "        </LineStyle>" << std::endl;
    os_ << "        <altitudeMode>absolute</altitudeMode>" << std::endl;
    os_ << "        <coordinates>";
    for(size_t i = 0; i < track.size(); ++i)
    {
        os_ << track[i] << ' ';
    }
    os_ << "</coordinates>" << std::endl;
    os_ << "      </LineString>" << std::endl;
    os_ << "    </Placemark>" << std::endl;
}
