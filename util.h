#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include "thread.h"
class PointSet;
class Track3D;
class Point3D;
class Point2D;
class KmlFile;

Point2D MGRSToUTM(const Point2D& pos, const std::string& InputDatum, const std::string& OutputDatum);
Point2D WGS84ToAGD66(const Point2D& pos);
Point2D AGD66ToWGS84(const Point2D& pos);

Point3D CalcTrack(
        // Fixed parameters
        const Point2D&  towerPosition,
        double          timeStep,
        const PointSet& altitudeTrack, // x=time     y=altitude
        // Varying parameters
        double          fixRange,
        double          fixBearing,
        double          elapsedTime,
        double          heading,
        double          initialBankRate,
        double          bankRateAccel,
        double          windHeading,
        const PointSet& windSpeeds,    // x=altitude y=wind speed
        const PointSet& planeSpeeds,   // x=time     y=speed
        // Returned values
        Track3D*        track = NULL
        );

Point2D utmToLatLng(int zone, const Point2D& en, bool northernHemisphere = true);

Point3D createStdTracks(KmlFile& kml, ThreadParams& params);

#endif // UTIL_H
