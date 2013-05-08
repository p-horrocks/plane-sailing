#ifndef THREAD_H
#define THREAD_H

#include "pointset.h"
#include "distribution.h"

struct ThreadParams
{
    int          iterations;
    Point2D      towerLocation;
    double       timeStep;
    PointSet     knownAltitudes;

    Distribution elapsedTime;
    Distribution fixRange;
    Distribution fixBearing;
    Distribution aircraftHeading;
    Distribution aircraftSpeedStart;
    Distribution aircraftSpeedFinish;
    Distribution initialBankRate;
    Distribution bankRateAccel;
    Distribution windDirection;
    Distribution windSpeed6000;
    Distribution windSpeed8000;
};

void* workerThread(void* params);

#endif // THREAD_H
