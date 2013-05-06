#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
class PointSet;
class Track3D;
class Point3D;

struct Distribution
{
    double mean;
    double stdDev;
};

struct Range
{
    double start;
    double end;
};

struct RangeBearing
{
    double range;
    double bearing;
};

struct Point
{
    Point() {}
    Point(double x_, double y_) : x(x_), y(y_) {}

    double x;
    double y;
};

#define DEG2RAD(x) ((x) * M_PI / 180.0)
#define RAD2DEG(x) ((x) * 180.0 / M_PI)

double FeetToMetres(double Feet);
double MetresToFeet(double Metres);
double NMToMetres(double NM);
double KnotsToMPS(double Knots);
double MPSToKnots(double MPS);
Point MGRSToUTM(const Point& pos, const std::string& InputDatum, const std::string& OutputDatum);
Point WGS84ToAGD66(const Point& pos);
Point AGD66ToWGS84(const Point& pos);
Point3D CalcTrack(
        const Point&          towerPosition,
        const RangeBearing&   fix,
        double                timeStep,
        double                elapsedTime,
        const PointSet&       altitudeTrack, // x=time     y=altitude
        const PointSet&       windSpeeds,    // x=altitude y=wind speed
        double                heading,
        double                initialBankRate,
        double                bankRateAccel,
        const PointSet&       planeSpeeds,   // x=time     y=speed
        double                windHeading,
        Track3D*              track = NULL
        );

Point utmToLatLng(int zone, const Point& en, bool northernHemisphere = true);

time_t stringToTime(const char* timeStr);

#endif // UTIL_H
