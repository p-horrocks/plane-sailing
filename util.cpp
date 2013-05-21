#include "util.h"

#include <cmath>
#include <iostream>
#include <cstring>

#include "pointset.h"
#include "track3d.h"
#include "point3d.h"
#include "units.h"
#include "kmlfile.h"

Point2D MGRSToUTM(const Point2D& pos, const std::string& InputDatum, const std::string& OutputDatum)
{
    Point2D retval = pos;
    if(InputDatum == "AGD66")
    {
        retval = AGD66ToWGS84(pos);
    }
    retval.x_ += 300000.0;
    retval.y_ += 6400000.0;
    if(OutputDatum == "AGD66")
    {
        retval = WGS84ToAGD66(retval);
    }
    return retval;
}

Point2D WGS84ToAGD66(const Point2D& pos)
{
    return pos + Point2D(-122, -183);
}

Point2D AGD66ToWGS84(const Point2D& pos)
{
    return pos + Point2D(122, 183);
}

Point3D CalcTrack(
        const Point2D&  towerPosition,
        double          timeStep,
        const PointSet& altitudeTrack,
        double          fixRange,
        double          fixBearing,
        double          elapsedTime,
        double          heading,
        double          initialBankRate,
        double          bankRateAccel,
        double          windHeading,
        const PointSet& windSpeeds,
        const PointSet& planeSpeeds,

        Track3D*        track
        )
{
    if(track != NULL)
    {
        track->clear();
    }

    // Set up the initial conditions.
    double x        = towerPosition.x_ + fixRange * sin(fixBearing);
    double y        = towerPosition.y_ + fixRange * cos(fixBearing);
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

Point2D utmToLatLng(int zone, const Point2D& en, bool northernHemisphere)
{
    const double easting = en.x_;
    const double northing = northernHemisphere ? en.y_ : (10000000 - en.y_);

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

    return Point2D(longitude, latitude);
}

Point3D createStdTracks(KmlFile& kml, ThreadParams& params)
{
    Track3D track1;
    Track3D track2;

    PointSet planeSpeeds;
    planeSpeeds.addPoint(0,                  params.aircraftSpeedStart.mean());
    planeSpeeds.addPoint(params.elapsedTime.mean(), params.aircraftSpeedFinish.mean());

    // Nominal Track
    Point3D nominalCrashPos = CalcTrack(
                params.towerLocation,
                params.timeStep,
                params.knownAltitudes,
                params.fixRange.mean(),
                params.fixBearing.mean(),
                params.elapsedTime.mean(),
                params.aircraftHeading.mean(),
                params.initialBankRate.mean(),
                params.bankRateAccel.mean(),
                params.windDirection.mean(),
                params.windProfile.mean(),
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
        double time = params.elapsedTime.offsetMean(stdDevTracks[i].time);

        planeSpeeds[1].x_ = time;
        planeSpeeds[0].y_ = params.aircraftSpeedStart.offsetMean(stdDevTracks[i].startSpeed);
        planeSpeeds[1].y_ = params.aircraftSpeedFinish.offsetMean(stdDevTracks[i].endSpeed);
        CalcTrack(
                    params.towerLocation,
                    params.timeStep,
                    params.knownAltitudes,
                    params.fixRange.offsetMean(stdDevTracks[i].range),
                    params.fixBearing.offsetMean(stdDevTracks[i].bearing),
                    time,
                    params.aircraftHeading.offsetMean(stdDevTracks[i].heading),
                    params.initialBankRate.offsetMean(stdDevTracks[i].bankRate),
                    params.bankRateAccel.offsetMean(stdDevTracks[i].bankAccel),
                    params.windDirection.offsetMean(stdDevTracks[i].windDir),
                    params.windProfile.offsetMean(stdDevTracks[i].windSpeed),
                    planeSpeeds,
                    &track1
                    );

        time = params.elapsedTime.offsetMean(-stdDevTracks[i].time);
        planeSpeeds[1].x_ = time;
        planeSpeeds[0].y_ = params.aircraftSpeedStart.offsetMean(-stdDevTracks[i].startSpeed);
        planeSpeeds[1].y_ = params.aircraftSpeedFinish.offsetMean(-stdDevTracks[i].endSpeed);
        CalcTrack(
                    params.towerLocation,
                    params.timeStep,
                    params.knownAltitudes,
                    params.fixRange.offsetMean(-stdDevTracks[i].range),
                    params.fixBearing.offsetMean(-stdDevTracks[i].bearing),
                    time,
                    params.aircraftHeading.offsetMean(-stdDevTracks[i].heading),
                    params.initialBankRate.offsetMean(-stdDevTracks[i].bankRate),
                    params.bankRateAccel.offsetMean(-stdDevTracks[i].bankAccel),
                    params.windDirection.offsetMean(-stdDevTracks[i].windDir),
                    params.windProfile.offsetMean(-stdDevTracks[i].windSpeed),
                    planeSpeeds,
                    &track2
                    );

        track1.convertAMG66toWGS84();
        track2.convertAMG66toWGS84();
        track2.reverse();
        kml.addPolygon(track1 + track2, stdDevTracks[i].name, "std_tracks");
    }

    return nominalCrashPos;
}
