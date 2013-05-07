#include <cmath>
#include <ctime>
#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>

#include "util.h"
#include "pointset.h"
#include "kmlfile.h"
#include "track3d.h"
#include "point3d.h"

const int MAP_CELLS_X               = 50;
const int MAP_CELLS_Y               = 50;
const std::string TOWER_GRID_SQUARE = "56HLJ";
const Point TOWER_LOCATION          = { 90345.0, 69908.0 }; // Location of Willianstown Tower, in UTM coords (AGD66) - from map
const double GRID_TO_MAGNETIC       = 11.63; // Grid to magnetic variation - from map. Defined as positive where magnetic angle is clockwise from grid north.

// The range and bearing from Williamstown Tower - from control tower log.
// Standard deviations are guessed.
const Distribution FIX_BEARING      = { DEG2RAD(320.0 + GRID_TO_MAGNETIC), DEG2RAD(2) };
const Distribution FIX_RANGE        = { NMToMetres(48.0), NMToMetres(0.5) };
const time_t FIX_TIME               = stringToTime("19:36:00");

// Aircraft heading, Magnetic bearing, from tower log (I think)
const Distribution AIRCRAFT_HEADING = { DEG2RAD(140.0 + GRID_TO_MAGNETIC), DEG2RAD(10) };

// Air speed of aircraft at start and finish, from John Watson's analysis
const Distribution SPEED_START      = { KnotsToMPS(145.0), KnotsToMPS(10.0) };
const Distribution SPEED_FINISH     = { KnotsToMPS(85.0), KnotsToMPS(10.0) };

// Crash time, estimated, from John Watson's analysis
const time_t CRASH_TIME             = stringToTime("19:39:27");

// Wind speed (knots) at 6000 and 8000 as well as dirction, from John Watson's work.
const Distribution WIND_8000        = { KnotsToMPS(43.0), KnotsToMPS(10.0) };
const Distribution WIND_6000        = { KnotsToMPS(33.0), KnotsToMPS(10.0) };
const Distribution WIND_DIRECTION   = { DEG2RAD(230.0 + GRID_TO_MAGNETIC), DEG2RAD(10) };

// Bank rate at start of simulation (deg/sec) and starting acceleration (deg/sec^2)
const Distribution BANK_RATE_START  = { DEG2RAD(0.0), DEG2RAD(0.1) };
const Distribution BANK_RATE_ACCEL  = { DEG2RAD(0.0), DEG2RAD(0.02) };

// 4=normal accuracy, 5=high accuracy
const int ACCURACY = 4;

// 1s is OK for about 50m accuracy even in extreme cases (1 deg/s bank rate)
const double TIME_INTEGRATION_STEP = 1.0;

int main(int argc, char *argv[])
{
    // Estimated time from the simulation start to crash.
    const Distribution elapsedTime = { difftime(CRASH_TIME, FIX_TIME), 35.0};

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
    windSpeeds.addPoint(FeetToMetres(6000), WIND_6000.mean);
    windSpeeds.addPoint(FeetToMetres(8000), WIND_8000.mean);

    PointSet planeSpeeds;
    planeSpeeds.addPoint(0,                SPEED_START.mean);
    planeSpeeds.addPoint(elapsedTime.mean, SPEED_FINISH.mean);

    const double MTIRange = NMToMetres(44.0); // MTI range from Williamstown Tower

    Point towerLocation = TOWER_LOCATION;
    if(TOWER_GRID_SQUARE == "56HLJ")
    {
        towerLocation = Point(TOWER_LOCATION.x, TOWER_LOCATION.y - 100000.0);
    }

    RangeBearing towerFix = { FIX_RANGE.mean, FIX_BEARING.mean };

//    Deviation = numpy.radians(2.0)

//    MTI_E1 = TowerPosition[0] + MTIRange * math.sin(TowerFix[1]+Deviation)
//    MTI_N1 = TowerPosition[1] + MTIRange * math.cos(TowerFix[1]+Deviation)
//    MTI_E2 = TowerPosition[0] + MTIRange * math.sin(TowerFix[1]-Deviation)
//    MTI_N2 = TowerPosition[1] + MTIRange * math.cos(TowerFix[1]-Deviation)

    KmlFile kml("track.kml");

    // Nominal Track
    Track3D track;
    Point3D end = CalcTrack(
                towerLocation,
                towerFix,
                TIME_INTEGRATION_STEP,
                elapsedTime.mean,
                knownAltitudes,
                windSpeeds,
                AIRCRAFT_HEADING.mean,
                BANK_RATE_START.mean,
                BANK_RATE_ACCEL.mean,
                planeSpeeds,
                WIND_DIRECTION.mean,
                &track
                );

    std::cout << "Nominal Crash Location: " << end.x_ << " " << end.y_ << std::endl;
    track.convertAMG66toWGS84();
    kml.addTrack(track, "Nominal track", "99ff7777");
    end.convertAMG66toWGS84();
    kml.addPoint(end, "Nominal crash location");

//    coordsUTM = MGRSToUTM(Nominal_E,Nominal_N,"AGD66","WGS84")
//    kml.newpoint(name="Nominal Crash Location",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    #MTI Line - If beyond this it will not have a moving target indicator
//    E1,N1 = MGRSToUTM(MTI_E1,MTI_N1,"AGD66","WGS84")
//    coords1 = utmToLatLng(56,E1,N1,0)
//    E2,N2 = MGRSToUTM(MTI_E2,MTI_N2,"AGD66","WGS84")
//    coords2 = utmToLatLng(56,E2,N2,0)

//    Track = [(coords1[0],coords1[1],FeetToMetres(7000.0)),(coords2[0],coords2[1],FeetToMetres(7000.0))]
//    KmlTrack = kml.newlinestring(name = "MTI Line", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = 20
//    KmlTrack.linestyle.color = '99ff7777'


//    #1STD Tracks
//    kmlFolder = kml.newfolder(name = "+/-1 STD Tracks", description = "A set of tracks showing +/-1 STD of all key parameters. This gives a guide as to what the signficant variables are.")
//    STD_Line_Width = 2
//    STD_Line_Colour = '99888888'

//    E,N,Track = CalcTrack(TowerPosition, (FixRangeMean+FixRangeSTD,FixBearingMean),
//                                TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "+1 STD Tower Range", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="+1 STD Tower Range",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, (FixRangeMean-FixRangeSTD,FixBearingMean),
//                                TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "-1 STD Tower Range", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="-1 STD Tower Range",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, (FixRangeMean,FixBearingMean+FixBearingSTD),
//                                TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "+1 STD Tower Bearing", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="+1 STD Tower Bearing",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, (FixRangeMean,FixBearingMean-FixBearingSTD),
//                                TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "-1 STD Tower Bearing", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="-1 STD Tower Bearing",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean+ElapsedTimeSTD, AltitudeTrack,
//                                       WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "+1 STD Elapsed Time", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="+1 STD Elapsed Time",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean-ElapsedTimeSTD, AltitudeTrack,
//                                       WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "-1 STD Elapsed Time", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="+1 STD Elapsed Time",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, [Wind6000Mean+Wind6000STD,Wind8000Mean+Wind8000STD],
//                                       AircraftHeadingMean, BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "+1 STD Wind Speed", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="+1 STD Wind Speed",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, [Wind6000Mean-Wind6000STD,Wind8000Mean-Wind8000STD],
//                                       AircraftHeadingMean, BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "-1 STD Wind Speed", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="-1 STD Wind Speed",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean+AircraftHeadingSTD,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "+1 STD Aircraft Heading", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="+1 STD Aircraft Heading",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean-AircraftHeadingSTD,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "-1 STD Aircraft Heading", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="-1 STD Aircraft Heading",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    if BankRateStartSTD>0:
//        E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean+BankRateStartSTD, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//        KmlTrack = kmlFolder.newlinestring(name = "+1 STD Bank Rate", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//        KmlTrack.linestyle.width = STD_Line_Width
//        KmlTrack.linestyle.color = STD_Line_Colour
//        coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//        kmlFolder.newpoint(name="+1 STD Bank Rate",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//        E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean-BankRateStartSTD, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean, True)

//        KmlTrack = kmlFolder.newlinestring(name = "-1 STD Bank Rate", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//        KmlTrack.linestyle.width = STD_Line_Width
//        KmlTrack.linestyle.color = STD_Line_Colour
//        coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//        kmlFolder.newpoint(name="-1 STD Bank Rate",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    if BankRateAccelerationSTD>0:
//        E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean+BankRateAccelerationSTD,
//                                       IASTrack, WindDirectionMean, True)

//        KmlTrack = kmlFolder.newlinestring(name = "+1 STD Bank Rate Acceleration", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//        KmlTrack.linestyle.width = STD_Line_Width
//        KmlTrack.linestyle.color = STD_Line_Colour
//        coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//        kmlFolder.newpoint(name="+1 STD Bank Rate Acceleration",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//        E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean-BankRateAccelerationSTD,
//                                       IASTrack, WindDirectionMean, True)

//        KmlTrack = kmlFolder.newlinestring(name = "-1 STD Bank Rate Acceleration", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//        KmlTrack.linestyle.width = STD_Line_Width
//        KmlTrack.linestyle.color = STD_Line_Colour
//        coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//        kmlFolder.newpoint(name="-1 STD Bank Rate Acceleration",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       [SpeedStartMean+SpeedStartSTD,SpeedFinishMean], WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "+1 STD Start Speed", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="+1 STD Start Speed",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       [SpeedStartMean-SpeedStartSTD,SpeedFinishMean], WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "-1 STD Start Speed", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="-1 STD Start Speed",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       [SpeedStartMean,SpeedFinishMean+SpeedFinishSTD], WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "+1 STD Finish Speed", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="+1 STD Finish Speed",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       [SpeedStartMean,SpeedFinishMean-SpeedFinishSTD], WindDirectionMean, True)

//    KmlTrack = kmlFolder.newlinestring(name = "-1 STD Finish Speed", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="-1 STD Finish Speed",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean+WindDirectionSTD, True)

//    KmlTrack = kmlFolder.newlinestring(name = "+1 STD Wind Direction", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="+1 STD Wind Direction",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    E,N,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTimeMean, AltitudeTrack, WindSpeedProfile, AircraftHeadingMean,
//                                       BankRateStartMean, BankRateAccelerationMean,
//                                       IASTrack, WindDirectionMean-WindDirectionSTD, True)

//    KmlTrack = kmlFolder.newlinestring(name = "-1 STD Wind Direction", coords = Track,
//                                 altitudemode = simplekml.AltitudeMode.absolute)
//    KmlTrack.linestyle.width = STD_Line_Width
//    KmlTrack.linestyle.color = STD_Line_Colour
//    coordsUTM = MGRSToUTM(E,N,"AGD66","WGS84")
//    kmlFolder.newpoint(name="-1 STD Wind Direction",
//                       coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

//    #Now let's start doing some calcs!

//    kmlFolder2 = kml.newfolder(name = "Stochastic Points", description = "1 in 100 of the stochastic points.")

//    Array_E = int(math.floor(Nominal_E/1000.0))-int(MapArray*0.5)
//    Array_N = int(math.floor(Nominal_N/1000.0))-int(MapArray*0.5)

    std::vector<double> map(MAP_CELLS_X * MAP_CELLS_Y, 0);
    const int numIterations = (int)pow(10, ACCURACY);

//    for i in range(Iterations):
//        FixBearing = random.gauss(FixBearingMean, FixBearingSTD)
//        FixRange = random.gauss(FixRangeMean, FixRangeSTD)
//        AircraftHeading = random.gauss(AircraftHeadingMean, AircraftHeadingSTD)
//        SpeedStart = random.gauss(SpeedStartMean, SpeedStartSTD)
//        SpeedFinish = random.gauss(SpeedFinishMean, SpeedFinishSTD)
//        ElapsedTime = abs(random.gauss(ElapsedTimeMean, ElapsedTimeSTD))
//        Wind8000 = random.gauss(Wind8000Mean, Wind8000STD)
//        Wind6000 = random.gauss(Wind6000Mean, Wind6000STD)
//        WindDirection = random.gauss(WindDirectionMean, WindDirectionSTD)
//        BankRateStart = random.gauss(BankRateStartMean, BankRateStartSTD)
//        BankRateAcceleration = random.gauss(BankRateAccelerationMean, BankRateAccelerationSTD)

//        WindSpeedProfile = [Wind6000,Wind8000]
//        IASTrack = [SpeedStart,SpeedFinish]

//        TowerFix = (FixRange,FixBearing)

//        CrashE,CrashN,Track = CalcTrack(TowerPosition, TowerFix, TimeIntegrationStep,
//                                       ElapsedTime, AltitudeTrack,
//                                       WindSpeedProfile, AircraftHeading,
//                                       BankRateStart, BankRateAcceleration,
//                                       IASTrack, WindDirection, False)

//        BinE = int(math.floor(CrashE/1000.0))-Array_E
//        BinN = int(math.floor(CrashN/1000.0))-Array_N
//        if BinE >= 0 and BinN >= 0 and BinE <= MapArray-1 and BinN <= MapArray-1:
//            lngDistribution[BinE, BinN] += 1

//        if i%1000==0:
//            # print CrashE, CrashN, BinE, BinN
//            print (100.0*float(i) / float(Iterations)),"% Done"
//            coordsUTM = MGRSToUTM(CrashE,CrashN,"AGD66","WGS84")
//            kmlFolder2.newpoint(name=str(i),coords=[utmToLatLng(56,coordsUTM[0],coordsUTM[1],0)])

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
