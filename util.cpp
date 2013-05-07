#include "util.h"

#include <cmath>
#include <iostream>
#include <cstring>

#include "pointset.h"
#include "track3d.h"
#include "point3d.h"

// Format string used for times.
const char* TIME_FORMAT = "%H:%M:%S";

double FeetToMetres(double Feet)
{
    return Feet * 0.3048;
}

double MetresToFeet(double Metres)
{
    return Metres / 0.3048;
}

double NMToMetres(double NM)
{
    return NM * 1852.0;
}

double KnotsToMPS(double Knots)
{
    return Knots * 1852.0 / 3600.0;
}

double MPSToKnots(double MPS)
{
    return MPS * 3600.0 / 1852.0;
}

Point MGRSToUTM(const Point& pos, const std::string& InputDatum, const std::string& OutputDatum)
{
    Point retval = pos;
    if(InputDatum == "AGD66")
    {
        retval = AGD66ToWGS84(pos);
    }
    retval.x += 300000.0;
    retval.y += + 6400000.0;
    if(OutputDatum == "AGD66")
    {
        retval = WGS84ToAGD66(retval);
    }
    return retval;
}

Point WGS84ToAGD66(const Point& pos)
{
    Point retval = pos;
    retval.x -= 112.0;
    retval.y -= 183.0;
    return retval;
}

Point AGD66ToWGS84(const Point& pos)
{
    Point retval = pos;
    retval.x += 112.0;
    retval.y += 183.0;
    return retval;
}

Point3D CalcTrack(
        const Point&          towerPosition,
        const RangeBearing&   fix,
        double                timeStep,
        double                elapsedTime,
        const PointSet&       altitudeTrack,
        const PointSet&       windSpeeds,
        double                heading,
        double                initialBankRate,
        double                bankRateAccel,
        const PointSet&       planeSpeeds,
        double                windHeading,
        Track3D*              track
        )
{
    if(track != NULL)
    {
        track->clear();
    }

    // Set up the initial conditions.
    double x        = towerPosition.x + fix.range * sin(fix.bearing);
    double y        = towerPosition.y + fix.range * cos(fix.bearing);
    double z        = 0.0;
    double time     = 0.0;
    double bankRate = initialBankRate;
    windHeading     = (M_PI / -2) - windHeading; // reversed as wind direction is where the wind is FROM
    double sinWind  = sin(windHeading);
    double cosWind  = cos(windHeading);

    while(time < elapsedTime)
    {
        // Get the time for this point.
        double thisStep = timeStep;
        double newTime  = time + timeStep;
        if(newTime > elapsedTime)
        {
            // Last step is incomplete.
            thisStep = elapsedTime - time;
            newTime  = elapsedTime;
        }
        time = newTime;

        // Interpolate the plane characteristics at this time.
        double altitude   = altitudeTrack.interpolate(time);
        double windSpeed  = windSpeeds.interpolate(altitude);
        double planeSpeed = planeSpeeds.interpolate(time);

        // Update the simulated plane location.
        heading    += bankRate * thisStep;
        bankRate   += bankRateAccel * thisStep;
        double hdg = (M_PI / 2) - heading;
        z          = altitude;
        x          += thisStep * (planeSpeed * cos(hdg) + windSpeed * cosWind);
        y          += thisStep * (planeSpeed * sin(hdg) + windSpeed * sinWind);

        if(track != NULL)
        {
            track->addPoint(x, y, z);
        }
    }

    return Point3D(x, y, z);
}

//def GetContourPoints(xarray,yarray,m,level):
//    import matplotlib.pyplot as plt
//    cs = plt.contour(yarray,xarray,m, [level])
//    p = cs.collections[0].get_paths()[0]
//    v = p.vertices
//    x = v[:,0]
//    y = v[:,1]
//    return (x,y)

Point utmToLatLng(int zone, const Point& en, bool northernHemisphere)
{
    const double easting = en.x;
    const double northing = northernHemisphere ? en.y : (10000000 - en.y);

    const double a = 6378137;
    const double e = 0.081819191;
    const double e1sq = 0.006739497;
    const double k0 = 0.9996;

    const double arc = northing / k0;
    const double mu = arc / (a * (1 - pow(e, 2) / 4.0 - 3 * pow(e, 4) / 64.0 - 5 * pow(e, 6) / 256.0));

    const double ei = (1 - pow((1 - e * e), (1 / 2.0))) / (1 + pow((1 - e * e), (1 / 2.0)));

    const double ca = 3 * ei / 2 - 27 * pow(ei, 3) / 32.0;

    const double cb = 21 * pow(ei, 2) / 16 - 55 * pow(ei, 4) / 32;
    const double cc = 151 * pow(ei, 3) / 96;
    const double cd = 1097 * pow(ei, 4) / 512;
    const double phi1 = mu + ca * sin(2 * mu) + cb * sin(4 * mu) + cc * sin(6 * mu) + cd * sin(8 * mu);

    const double n0 = a / pow((1 - pow((e * sin(phi1)), 2)), (1 / 2.0));

    const double r0 = a * (1 - e * e) / pow((1 - pow((e * sin(phi1)), 2)), (3 / 2.0));
    const double fact1 = n0 * tan(phi1) / r0;

    const double _a1 = 500000 - easting;
    const double dd0 = _a1 / (n0 * k0);
    const double fact2 = dd0 * dd0 / 2;

    const double t0 = pow(tan(phi1), 2);
    const double Q0 = e1sq * pow(cos(phi1), 2);
    const double fact3 = (5 + 3 * t0 + 10 * Q0 - 4 * Q0 * Q0 - 9 * e1sq) * pow(dd0, 4) / 24;

    const double fact4 = (61 + 90 * t0 + 298 * Q0 + 45 * t0 * t0 - 252 * e1sq - 3 * Q0 * Q0) * pow(dd0, 6) / 720;

    const double lof1 = _a1 / (n0 * k0);
    const double lof2 = (1 + 2 * t0 + Q0) * pow(dd0, 3) / 6.0;
    const double lof3 = (5 - 2 * Q0 + 28 * t0 - 3 * pow(Q0, 2) + 8 * e1sq + 24 * pow(t0, 2)) * pow(dd0, 5) / 120;
    const double _a2 = (lof1 - lof2 + lof3) / cos(phi1);
    const double _a3 = _a2 * 180 / M_PI;

    double latitude = 180 * (phi1 - fact1 * (fact2 + fact3 + fact4)) / M_PI;

    if(!northernHemisphere)
    {
        latitude = -latitude;
    }

    double longitude = ((zone > 0) ? (6 * zone - 183.0) : 3.0) - _a3;

    return Point(longitude, latitude);
}

time_t stringToTime(const char* time)
{
    struct tm t;
    memset(&t, 0, sizeof(t));
    strptime(time, TIME_FORMAT, &t);
    return mktime(&t);
}
