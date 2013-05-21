#include <QApplication>

#include "mainwnd.h"

#include <cmath>
#include <ctime>
#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <gdal/gdal_priv.h>

#include "util.h"
#include "pointset.h"
#include "kmlfile.h"
#include "track3d.h"
#include "point3d.h"
#include "distribution.h"
#include "units.h"
#include "thread.h"

// Size of the sample grid.
const int MAP_CELLS_X               = 50;
const int MAP_CELLS_Y               = 50;
const double METRES_PER_CELL        = 1000;

const std::string TOWER_GRID_SQUARE = "56HLJ";
const Point2D TOWER_LOCATION          ( 90345.0, 69908.0 ); // Location of Willianstown Tower, in UTM coords (AGD66) - from map
const double GRID_TO_MAGNETIC       = 11.63; // Grid to magnetic variation - from map. Defined as positive where magnetic angle is clockwise from grid north.

// The range and bearing from Williamstown Tower - from control tower log.
// Standard deviations are guessed.
const Distribution FIX_BEARING        ( DEG2RAD(320.0 + GRID_TO_MAGNETIC), DEG2RAD(2) );
const Distribution FIX_RANGE          ( NMToMetres(48.0), NMToMetres(0.5) );
const time_t FIX_TIME               = stringToTime("19:36:00");

// Aircraft heading, Magnetic bearing, from tower log (I think)
const Distribution AIRCRAFT_HEADING   ( DEG2RAD(140.0 + GRID_TO_MAGNETIC), DEG2RAD(10) );

// Air speed of aircraft at start and finish, from John Watson's analysis
const Distribution SPEED_START        ( KnotsToMPS(145.0), KnotsToMPS(10.0) );
const Distribution SPEED_FINISH       ( KnotsToMPS(85.0), KnotsToMPS(10.0) );

// Crash time, estimated, from John Watson's analysis
const time_t CRASH_TIME             = stringToTime("19:39:27");

// Wind speed (knots) at 6000 and 8000 as well as direction, from John Watson's work.
const Distribution WIND_8000          ( KnotsToMPS(43.0), KnotsToMPS(10.0) );
const Distribution WIND_6000          ( KnotsToMPS(33.0), KnotsToMPS(10.0) );
const Distribution WIND_DIRECTION     ( DEG2RAD(230.0 + GRID_TO_MAGNETIC), DEG2RAD(10) );

// Bank rate at start of simulation (deg/sec) and starting acceleration (deg/sec^2)
const Distribution BANK_RATE_START    ( DEG2RAD(0.0), DEG2RAD(0.1) );
const Distribution BANK_RATE_ACCEL    ( DEG2RAD(0.0), DEG2RAD(0.02) );

// 4=normal accuracy, 5=high accuracy
const int ACCURACY = 6;

// 1s is OK for about 50m accuracy even in extreme cases (1 deg/s bank rate)
const double TIME_INTEGRATION_STEP = 1.0;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWnd wnd;
    wnd.setVisible(true);
    return app.exec();

//    GDALAllRegister();
//    GDALDriver* driver   = GetGDALDriverManager()->GetDriverByName("VRT");
//    GDALDataset* ds      = driver->Create("", params.gridCellsX, params.gridCellsY, 1, GDT_Float64, 0);
//    GDALRasterBand* band = ds->GetRasterBand(1);

//    GDALRasterBandH band;
//    OGRLayerH layer;
//    GDALContourGenerate(
//                band, highestCell / 4.0, 0, 0, NULL, FALSE, 0, layer, -1, -1, NULL, NULL
//                );

//    GDALClose(ds);

//    for i in range(MapArray):
//        for j in range(MapArray):
//            dblDistribution[i,j] = 100.0*(float(lngDistribution[i,j]) / float(Iterations))

//    x=numpy.zeros((MapArray), dtype='float64')
//    y=numpy.zeros((MapArray), dtype='float64')

//    for i in range(MapArray):
//        x[i] = float((i+Array_E)*1000)+500.0
//        y[i] = float((i+Array_N)*1000)+500.0

//    MaxProb = dblDistribution.max()

//    # Make contours!
//    points = GetContourPoints(x,y,dblDistribution,MaxProb*0.25)

//    fred=[]
//    for i in range(points[0].size):
//        E,N = MGRSToUTM(points[1][i],points[0][i],"AGD66","WGS84")
//        fred.append(utmToLatLng(56,E,N,0))

//    pol2 = kml.newpolygon(name="{0}% per sq km region".format(MaxProb*0.25),
//                         outerboundaryis=fred)
//    pol2.polystyle.color = '660000ff'
//    pol2.polystyle.outline = 1

//    points = GetContourPoints(x,y,dblDistribution,MaxProb*0.5)

//    fred=[]
//    for i in range(points[0].size):
//        E,N = MGRSToUTM(points[1][i],points[0][i],"AGD66","WGS84")
//        fred.append(utmToLatLng(56,E,N,0))

//    pol3 = kml.newpolygon(name="{0}% per sq km region".format(MaxProb*0.5),
//                         outerboundaryis=fred)
//    pol3.polystyle.color = '6600ff00'
//    pol3.polystyle.outline = 1

//    points = GetContourPoints(x,y,dblDistribution,MaxProb*0.75)

//    fred=[]
//    for i in range(points[0].size):
//        E,N = MGRSToUTM(points[1][i],points[0][i],"AGD66","WGS84")
//        fred.append(utmToLatLng(56,E,N,0))

//    pol3 = kml.newpolygon(name="{0}% per sq km region".format(MaxProb*0.75),
//                         outerboundaryis=fred)
//    pol3.polystyle.color = '66ff0000'
//    pol3.polystyle.outline = 1

//    kml.save("track2quick.kml")

    return 0;
}
