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

    ThreadParams params;

    params.iterations    = 1;
    params.timeStep      = TIME_INTEGRATION_STEP;
    params.elapsedTime   = Distribution(difftime(CRASH_TIME, FIX_TIME), 35.0);
    params.towerLocation = TOWER_LOCATION;
    if(TOWER_GRID_SQUARE == "56HLJ")
    {
        params.towerLocation.y_ -= 100000.0;
    }

    params.knownAltitudes.addPoint(0,                                            FeetToMetres(8500));
    params.knownAltitudes.addPoint(difftime(stringToTime("19:37:39"), FIX_TIME), FeetToMetres(7500)); // Altitude versus time track, point 2 (7500 feet at 19:37:39)
    params.knownAltitudes.addPoint(difftime(stringToTime("19:38:29"), FIX_TIME), FeetToMetres(6500)); // Altitude versus time track, point 3 (6500 feet at 19:38:29)
    params.knownAltitudes.addPoint(difftime(stringToTime("19:39:27"), FIX_TIME), FeetToMetres(5500)); // Altitude versus time track, point 4 (5500 feet at 19:39:27)

    params.fixRange   = FIX_RANGE;
    params.fixBearing = FIX_BEARING;

    params.aircraftHeading     = AIRCRAFT_HEADING;
    params.aircraftSpeedStart  = SPEED_START;
    params.aircraftSpeedFinish = SPEED_FINISH;
    params.initialBankRate     = BANK_RATE_START;
    params.bankRateAccel       = BANK_RATE_ACCEL;

    params.windSpeed6000 = WIND_6000;
    params.windSpeed8000 = WIND_8000;
    params.windDirection = WIND_DIRECTION;

    KmlFile kml("track.kml");

    Point3D nominalCrashPos = createStdTracks(kml, params);

    const int numIterations = (int)pow(10, ACCURACY);
    params.iterations = numIterations;
//    int ptFreq = numIterations / 100;

    std::vector<double> grid(MAP_CELLS_X * MAP_CELLS_Y, 0);
    params.gridCellsX    = MAP_CELLS_X;
    params.gridCellsY    = MAP_CELLS_Y;
    params.metresPerCell = METRES_PER_CELL;
    params.gridOrigin    = Point2D(
                nominalCrashPos.x_ - (MAP_CELLS_X * METRES_PER_CELL * 0.5),
                nominalCrashPos.y_ - (MAP_CELLS_Y * METRES_PER_CELL * 0.5)
                );
    params.grid = &grid;

    pthread_mutex_init(&params.mutex, NULL);

    // Allocate space for enough threads and reduce the iterations per thread
    // to get the same total.
    std::vector<pthread_t> threads(8);
    params.iterations /= threads.size();

    // Create all the threads.
    for(size_t i = 0; i < threads.size(); ++i)
    {
        pthread_t id;
        pthread_create(&id, NULL, workerThread, &params);
        threads[i] = id;
    }

    // Wait for all the threads to finish.
    for(size_t i = 0; i < threads.size(); ++i)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&params.mutex);
    double highestCell = *std::max_element(grid.begin(), grid.end());

    kml.startFolder("Grid");
    int idx  = 0;
    double y = params.gridOrigin.y_;
    for(int row = 0; row < MAP_CELLS_Y; ++row, y += METRES_PER_CELL)
    {
        double x = params.gridOrigin.x_;
        for(int col = 0; col < MAP_CELLS_X; ++col, ++idx, x += METRES_PER_CELL)
        {
            Track3D cell;
            cell.addPoint(x, y, 0);
            cell.addPoint(x + METRES_PER_CELL, y, 0);
            cell.addPoint(x + METRES_PER_CELL, y + METRES_PER_CELL, 0);
            cell.addPoint(x, y + METRES_PER_CELL, 0);
            cell.addPoint(x, y, 0);
            cell.convertAMG66toWGS84();

            const char* style;
            int level = std::round((4 * grid[idx]) / highestCell);
            switch(level)
            {
            case 1:
                style = "cell_25"; break;
            case 2:
                style = "cell_50"; break;
            case 3:
                style = "cell_75"; break;
            case 4:
                style = "cell_100"; break;
            default:
                style = ((row == 0) && (col == 0)) ? "origin_cell" : "empty_cell"; break;
            }
            kml.addPolygon(cell, NULL, style, false);
        }
    }

    GDALAllRegister();
    GDALDriver* driver   = GetGDALDriverManager()->GetDriverByName("VRT");
    GDALDataset* ds      = driver->Create("", params.gridCellsX, params.gridCellsY, 1, GDT_Float64, 0);
    GDALRasterBand* band = ds->GetRasterBand(1);

//    GDALRasterBandH band;
//    OGRLayerH layer;
//    GDALContourGenerate(
//                band, highestCell / 4.0, 0, 0, NULL, FALSE, 0, layer, -1, -1, NULL, NULL
//                );

    GDALClose(ds);

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
