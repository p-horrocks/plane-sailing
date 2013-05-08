#include <cmath>
#include <ctime>
#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "util.h"
#include "pointset.h"
#include "kmlfile.h"
#include "track3d.h"
#include "point3d.h"
#include "distribution.h"

// Size of the sample grid.
const int MAP_CELLS_X               = 50;
const int MAP_CELLS_Y               = 50;
const double METRES_PER_CELL        = 1000;

const std::string TOWER_GRID_SQUARE = "56HLJ";
const Point TOWER_LOCATION          = { 90345.0, 69908.0 }; // Location of Willianstown Tower, in UTM coords (AGD66) - from map
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
const int ACCURACY = 4;

// 1s is OK for about 50m accuracy even in extreme cases (1 deg/s bank rate)
const double TIME_INTEGRATION_STEP = 1.0;

int main(int, char *[])
{
    // Estimated time from the simulation start to crash.
    const Distribution elapsedTime(difftime(CRASH_TIME, FIX_TIME), 35.0);

//    lngDistribution=numpy.zeros((MapArray,MapArray), dtype='int32')
//    dblDistribution=numpy.zeros((MapArray,MapArray), dtype='float64')

    // The x coordinate is elapsed time, y coordinate is altitude. Track starts
    // at 8500 feet.
    PointSet knownAltitudes;
    knownAltitudes.addPoint(0,                                            FeetToMetres(8500));
    knownAltitudes.addPoint(difftime(stringToTime("19:37:39"), FIX_TIME), FeetToMetres(7500)); // Altitude versus time track, point 2 (7500 feet at 19:37:39)
    knownAltitudes.addPoint(difftime(stringToTime("19:38:29"), FIX_TIME), FeetToMetres(6500)); // Altitude versus time track, point 3 (6500 feet at 19:38:29)
    knownAltitudes.addPoint(difftime(stringToTime("19:39:27"), FIX_TIME), FeetToMetres(5500)); // Altitude versus time track, point 4 (5500 feet at 19:39:27)

    PointSet windSpeeds;
    windSpeeds.addPoint(FeetToMetres(6000), WIND_6000.mean());
    windSpeeds.addPoint(FeetToMetres(8000), WIND_8000.mean());

    PointSet planeSpeeds;
    planeSpeeds.addPoint(0,                  SPEED_START.mean());
    planeSpeeds.addPoint(elapsedTime.mean(), SPEED_FINISH.mean());

    Point towerLocation = TOWER_LOCATION;
    if(TOWER_GRID_SQUARE == "56HLJ")
    {
        towerLocation = Point(TOWER_LOCATION.x, TOWER_LOCATION.y - 100000.0);
    }

    KmlFile kml("track.kml");
    Track3D track1;
    Track3D track2;

    // Nominal Track
    Point3D nominalCrashPos = CalcTrack(
                towerLocation,
                TIME_INTEGRATION_STEP,
                knownAltitudes,
                FIX_RANGE.mean(),
                FIX_BEARING.mean(),
                elapsedTime.mean(),
                AIRCRAFT_HEADING.mean(),
                BANK_RATE_START.mean(),
                BANK_RATE_ACCEL.mean(),
                WIND_DIRECTION.mean(),
                windSpeeds,
                planeSpeeds,
                &track1
                );

    std::cout << "Nominal Crash Location: " << nominalCrashPos.x_ << " " << nominalCrashPos.y_ << std::endl;
    track1.convertAMG66toWGS84();
    kml.addTrack(track1, "Nominal track", "99ff7777");
    Point3D end = nominalCrashPos;
    end.convertAMG66toWGS84();
    kml.addPoint(end, "Nominal crash location");

    //    #MTI Line - If beyond this it will not have a moving target indicator

//    const double MTIRange = NMToMetres(44.0); // MTI range from Williamstown Tower
//    Deviation = numpy.radians(2.0)
//    MTI_E1 = TowerPosition[0] + MTIRange * math.sin(TowerFix[1]+Deviation)
//    MTI_N1 = TowerPosition[1] + MTIRange * math.cos(TowerFix[1]+Deviation)
//    MTI_E2 = TowerPosition[0] + MTIRange * math.sin(TowerFix[1]-Deviation)
//    MTI_N2 = TowerPosition[1] + MTIRange * math.cos(TowerFix[1]-Deviation)
//    E1,N1 = MGRSToUTM(MTI_E1,MTI_N1,"AGD66","WGS84")
//    coords1 = utmToLatLng(56,E1,N1,0)
//    E2,N2 = MGRSToUTM(MTI_E2,MTI_N2,"AGD66","WGS84")
//    coords2 = utmToLatLng(56,E2,N2,0)

//    Track = [(coords1[0],coords1[1],FeetToMetres(7000.0)),(coords2[0],coords2[1],FeetToMetres(7000.0))]
//    KmlTrack = kml.newlinestring(name = "MTI Line", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = 20
//    KmlTrack.linestyle.color = '99ff7777'

    const struct
    {
        double range;
        double bearing;
        double time;
        double windSpeed;
        double heading;
        double bankRate;
        double bankAccel;
        double startSpeed;
        double endSpeed;
        double windDir;
        const char* name;
    }
    stdDevTracks[] =
    {
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Tower range +/-1 STD" } ,
        { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, "Tower bearing +/-1 STD" } ,
        { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Elapsed time +/-1 STD" } ,
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, "Wind speed +/-1 STD" } ,
        { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, "Aircraft heading +/-1 STD" } ,
        { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, "Bank rate +/-1 STD" } ,
        { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, "Bank rate acceleration +/-1 STD" } ,
        { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, "Start speed +/-1 STD" } ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, "End speed +/-1 STD" } ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, "Wind direction +/-1 STD" } ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL }
    };

    kml.startFolder("+/-1 STD tracks");
    for(int i = 0; stdDevTracks[i].name; ++i)
    {
        double time = elapsedTime.offsetMean(stdDevTracks[i].time);

        planeSpeeds[1].x = time;
        planeSpeeds[0].y = SPEED_START.offsetMean(stdDevTracks[i].startSpeed);
        planeSpeeds[1].y = SPEED_FINISH.offsetMean(stdDevTracks[i].endSpeed);
        windSpeeds[0].y  = WIND_6000.offsetMean(stdDevTracks[i].windSpeed);
        windSpeeds[1].y  = WIND_8000.offsetMean(stdDevTracks[i].windSpeed);
        CalcTrack(
                    towerLocation,
                    TIME_INTEGRATION_STEP,
                    knownAltitudes,
                    FIX_RANGE.offsetMean(stdDevTracks[i].range),
                    FIX_BEARING.offsetMean(stdDevTracks[i].bearing),
                    time,
                    AIRCRAFT_HEADING.offsetMean(stdDevTracks[i].heading),
                    BANK_RATE_START.offsetMean(stdDevTracks[i].bankRate),
                    BANK_RATE_ACCEL.offsetMean(stdDevTracks[i].bankAccel),
                    WIND_DIRECTION.offsetMean(stdDevTracks[i].windDir),
                    windSpeeds,
                    planeSpeeds,
                    &track1
                    );

        time = elapsedTime.offsetMean(-stdDevTracks[i].time);
        planeSpeeds[1].x = time;
        planeSpeeds[0].y = SPEED_START.offsetMean(-stdDevTracks[i].startSpeed);
        planeSpeeds[1].y = SPEED_FINISH.offsetMean(-stdDevTracks[i].endSpeed);
        windSpeeds[0].y  = WIND_6000.offsetMean(-stdDevTracks[i].windSpeed);
        windSpeeds[1].y  = WIND_8000.offsetMean(-stdDevTracks[i].windSpeed);
        CalcTrack(
                    towerLocation,
                    TIME_INTEGRATION_STEP,
                    knownAltitudes,
                    FIX_RANGE.offsetMean(-stdDevTracks[i].range),
                    FIX_BEARING.offsetMean(-stdDevTracks[i].bearing),
                    time,
                    AIRCRAFT_HEADING.offsetMean(-stdDevTracks[i].heading),
                    BANK_RATE_START.offsetMean(-stdDevTracks[i].bankRate),
                    BANK_RATE_ACCEL.offsetMean(-stdDevTracks[i].bankAccel),
                    WIND_DIRECTION.offsetMean(-stdDevTracks[i].windDir),
                    windSpeeds,
                    planeSpeeds,
                    &track2
                    );

        track1.convertAMG66toWGS84();
        track2.convertAMG66toWGS84();
        track2.reverse();
        kml.addPolygon(track1 + track2, stdDevTracks[i].name, "std_tracks");
    }

//    Array_E = int(math.floor(Nominal_E/1000.0))-int(MapArray*0.5)
//    Array_N = int(math.floor(Nominal_N/1000.0))-int(MapArray*0.5)

    const int numIterations = (int)pow(10, ACCURACY);
    int ptFreq = numIterations / 100;

    std::vector<double> grid(MAP_CELLS_X * MAP_CELLS_Y, 0);
    const Point2D gridOrigin(
                nominalCrashPos.x_ - (MAP_CELLS_X * METRES_PER_CELL * 0.5),
                nominalCrashPos.y_ - (MAP_CELLS_Y * METRES_PER_CELL * 0.5)
                );

    kml.startFolder("Stochastic points (sample)");
    double highestCell = 0;
    for(int i = 0; i < numIterations; ++i)
    {
        double time = elapsedTime.sample();
        planeSpeeds[1].x = time;
        planeSpeeds[0].y = SPEED_START.sample();
        planeSpeeds[1].y = SPEED_FINISH.sample();
        windSpeeds[0].y  = WIND_6000.sample();
        windSpeeds[1].y  = WIND_8000.sample();
        Point3D crashPos = CalcTrack(
                    towerLocation,
                    TIME_INTEGRATION_STEP,
                    knownAltitudes,
                    FIX_RANGE.sample(),
                    FIX_BEARING.sample(),
                    time,
                    AIRCRAFT_HEADING.sample(),
                    BANK_RATE_START.sample(),
                    BANK_RATE_ACCEL.sample(),
                    WIND_DIRECTION.sample(),
                    windSpeeds,
                    planeSpeeds
                    );

        int col = std::round((crashPos.x_ - gridOrigin.x_) / METRES_PER_CELL);
        int row = std::round((crashPos.y_ - gridOrigin.y_) / METRES_PER_CELL);
        if((col >= 0) && (row >= 0) && (col < MAP_CELLS_X) && (row < MAP_CELLS_Y))
        {
            int idx = col + (row * MAP_CELLS_X);
            grid[idx] += 1.0;
            highestCell = std::max(highestCell, grid[idx]);
        }

        if((i % ptFreq) == 0)
        {
            std::stringstream ss;
            ss << ((100 * i) / numIterations) << "%";
            crashPos.convertAMG66toWGS84();
            kml.addPoint(crashPos, ss.str().c_str());
        }
    }

    kml.startFolder("Grid");
    int idx = 0;
    for(int row = 0; row <= MAP_CELLS_Y; ++row)
    {
        double y = gridOrigin.y_ + (row * METRES_PER_CELL);
        for(int col = 0; col <= MAP_CELLS_X; ++col, ++idx)
        {
            double x = gridOrigin.x_ + (col * METRES_PER_CELL);
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
