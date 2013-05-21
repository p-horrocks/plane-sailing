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

    os_ << "    <Style id=\"std_tracks\">" << std::endl;
    os_ << "      <LineStyle>" << std::endl;
    os_ << "          <color>ffffffff</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <width>1</width>" << std::endl;
    os_ << "      </LineStyle>" << std::endl;
    os_ << "      <PolyStyle>" << std::endl;
    os_ << "          <color>a0ffffff</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <fill>1</fill>" << std::endl;
    os_ << "          <outline>0</outline>" << std::endl;
    os_ << "      </PolyStyle>" << std::endl;
    os_ << "    </Style>" << std::endl;

    os_ << "    <Style id=\"empty_cell\">" << std::endl;
    os_ << "      <LineStyle>" << std::endl;
    os_ << "          <color>ffa0a0a0</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <width>1</width>" << std::endl;
    os_ << "      </LineStyle>" << std::endl;
    os_ << "      <PolyStyle>" << std::endl;
    os_ << "          <fill>0</fill>" << std::endl;
    os_ << "          <outline>1</outline>" << std::endl;
    os_ << "      </PolyStyle>" << std::endl;
    os_ << "    </Style>" << std::endl;

    os_ << "    <Style id=\"origin_cell\">" << std::endl;
    os_ << "      <LineStyle>" << std::endl;
    os_ << "          <color>ffa0a0a0</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <width>1</width>" << std::endl;
    os_ << "      </LineStyle>" << std::endl;
    os_ << "      <PolyStyle>" << std::endl;
    os_ << "          <color>80ffffff</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <fill>1</fill>" << std::endl;
    os_ << "          <outline>1</outline>" << std::endl;
    os_ << "      </PolyStyle>" << std::endl;
    os_ << "    </Style>" << std::endl;

    os_ << "    <Style id=\"cell_25\">" << std::endl;
    os_ << "      <LineStyle>" << std::endl;
    os_ << "          <color>ffa0a0a0</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <width>1</width>" << std::endl;
    os_ << "      </LineStyle>" << std::endl;
    os_ << "      <PolyStyle>" << std::endl;
    os_ << "          <color>8000ff00</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <fill>1</fill>" << std::endl;
    os_ << "          <outline>1</outline>" << std::endl;
    os_ << "      </PolyStyle>" << std::endl;
    os_ << "    </Style>" << std::endl;

    os_ << "    <Style id=\"cell_50\">" << std::endl;
    os_ << "      <LineStyle>" << std::endl;
    os_ << "          <color>ffa0a0a0</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <width>1</width>" << std::endl;
    os_ << "      </LineStyle>" << std::endl;
    os_ << "      <PolyStyle>" << std::endl;
    os_ << "          <color>80ff0000</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <fill>1</fill>" << std::endl;
    os_ << "          <outline>1</outline>" << std::endl;
    os_ << "      </PolyStyle>" << std::endl;
    os_ << "    </Style>" << std::endl;

    os_ << "    <Style id=\"cell_75\">" << std::endl;
    os_ << "      <LineStyle>" << std::endl;
    os_ << "          <color>ffa0a0a0</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <width>1</width>" << std::endl;
    os_ << "      </LineStyle>" << std::endl;
    os_ << "      <PolyStyle>" << std::endl;
    os_ << "          <color>800080ff</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <fill>1</fill>" << std::endl;
    os_ << "          <outline>1</outline>" << std::endl;
    os_ << "      </PolyStyle>" << std::endl;
    os_ << "    </Style>" << std::endl;

    os_ << "    <Style id=\"cell_100\">" << std::endl;
    os_ << "      <LineStyle>" << std::endl;
    os_ << "          <color>ffa0a0a0</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <width>1</width>" << std::endl;
    os_ << "      </LineStyle>" << std::endl;
    os_ << "      <PolyStyle>" << std::endl;
    os_ << "          <color>800000ff</color>" << std::endl;
    os_ << "          <colorMode>normal</colorMode>" << std::endl;
    os_ << "          <fill>1</fill>" << std::endl;
    os_ << "          <outline>1</outline>" << std::endl;
    os_ << "      </PolyStyle>" << std::endl;
    os_ << "    </Style>" << std::endl;
}

KmlFile::~KmlFile()
{
    if(!folderName_.empty())
    {
        os_ << "  </Folder>" << std::endl;
    }
    os_ << "  </Document>" << std::endl;
    os_ << "</kml>" << std::endl;
}

void KmlFile::startFolder(const std::string& name)
{
    if(!folderName_.empty())
    {
        os_ << "  </Folder>" << std::endl;
    }
    folderName_ = name;
    if(!folderName_.empty())
    {
        os_ << "  <Folder>" << std::endl;
        os_ << "    <name>" << folderName_ << "</name>" << std::endl;
    }
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

void KmlFile::addTrack(const Track3D& track, const char* name, const char* style, bool useZ)
{
    os_ << "    <Placemark>" << std::endl;
    os_ << "      <name>" << name << "</name>" << std::endl;
    os_ << "      <styleUrl>#" << style << "</styleUrl>" << std::endl;
    os_ << "      <LineString>" << std::endl;
    if(useZ)
    {
        os_ << "        <altitudeMode>absolute</altitudeMode>" << std::endl;
    }
    os_ << "        <coordinates>";
    for(size_t i = 0; i < track.size(); ++i)
    {
        if(useZ)
        {
            os_ << track[i] << ' ';
        }
        else
        {
            os_ << (Point2D)track[i] << ' ';
        }
    }
    os_ << "</coordinates>" << std::endl;
    os_ << "      </LineString>" << std::endl;
    os_ << "    </Placemark>" << std::endl;
}

void KmlFile::addMultiTrack(const std::vector<Track3D>& tracks, const char* name, const char* style)
{
    os_ << "    <Placemark>" << std::endl;
    os_ << "      <name>" << name << "</name>" << std::endl;
    os_ << "      <styleUrl>#" << style << "</styleUrl>" << std::endl;
    os_ << "      <MultiGeometry>" << std::endl;
    for(auto t = tracks.begin(); t != tracks.end(); ++t)
    {
        os_ << "        <LineString>" << std::endl;
        os_ << "          <coordinates>";
        for(size_t i = 0; i < t->size(); ++i)
        {
            os_ << (Point2D)(*t)[i] << ' ';
        }
        os_ << "</coordinates>" << std::endl;
        os_ << "        </LineString>" << std::endl;
    }
    os_ << "      </MultiGeometry>" << std::endl;
    os_ << "    </Placemark>" << std::endl;
}

void KmlFile::addPolygon(const Track3D& track, const char* name, const char* style, bool useZ)
{
    os_ << "    <Placemark>" << std::endl;
    if(name != NULL)
    {
        os_ << "      <name>" << name << "</name>" << std::endl;
    }
    os_ << "      <styleUrl>#" << style << "</styleUrl>" << std::endl;
    os_ << "      <Polygon>" << std::endl;
    if(useZ)
    {
        os_ << "        <altitudeMode>absolute</altitudeMode>" << std::endl;
    }
    os_ << "        <outerBoundaryIs><LinearRing><coordinates>";
    for(size_t i = 0; i < track.size(); ++i)
    {
        if(useZ)
        {
            os_ << track[i] << ' ';
        }
        else
        {
            os_ << (Point2D)track[i] << ' ';
        }
    }
    os_ << "</coordinates></LinearRing></outerBoundaryIs>" << std::endl;
    os_ << "      </Polygon>" << std::endl;
    os_ << "    </Placemark>" << std::endl;
}
