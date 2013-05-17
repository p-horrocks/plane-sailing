#include "thread.h"

#include "units.h"
#include "util.h"
#include "point3d.h"

void* workerThread(void* params)
{
    ThreadParams* tp = reinterpret_cast<ThreadParams*>(params);

    PointSet windSpeeds;
    windSpeeds.addPoint(FeetToMetres(6000), tp->windSpeed6000.mean());
    windSpeeds.addPoint(FeetToMetres(8000), tp->windSpeed8000.mean());

    PointSet planeSpeeds;
    planeSpeeds.addPoint(0,                      tp->aircraftSpeedStart.mean());
    planeSpeeds.addPoint(tp->elapsedTime.mean(), tp->aircraftSpeedFinish.mean());

    int count = 0;
    for(int i = 0; i < tp->iterationsPerThread; ++i, ++count)
    {
        double time = tp->elapsedTime.sample();
        planeSpeeds[1].x_ = time;
        planeSpeeds[0].y_ = tp->aircraftSpeedStart.sample();
        planeSpeeds[1].y_ = tp->aircraftSpeedFinish.sample();
        windSpeeds[0].y_  = tp->windSpeed6000.sample();
        windSpeeds[1].y_  = tp->windSpeed8000.sample();
        Point3D crashPos = CalcTrack(
                    tp->towerLocation,
                    tp->timeStep,
                    tp->knownAltitudes,
                    tp->fixRange.sample(),
                    tp->fixBearing.sample(),
                    time,
                    tp->aircraftHeading.sample(),
                    tp->initialBankRate.sample(),
                    tp->bankRateAccel.sample(),
                    tp->windDirection.sample(),
                    windSpeeds,
                    planeSpeeds
                    );

        pthread_mutex_lock(&tp->mutex);

        int col = std::round((crashPos.x_ - tp->gridOrigin.x_) / tp->metresPerCell);
        int row = std::round((crashPos.y_ - tp->gridOrigin.y_) / tp->metresPerCell);
        if((col >= 0) && (row >= 0) && (col < tp->gridCellsX) && (row < tp->gridCellsY))
        {
            int idx = col + (row * tp->gridCellsX);
            tp->grid[idx] += 1.0;
        }
        if(count >= 100)
        {
            tp->completed += count;
            count          = 0;
        }

        pthread_mutex_unlock(&tp->mutex);
    }

    pthread_mutex_lock(&tp->mutex);
    tp->completed += count;
    pthread_mutex_unlock(&tp->mutex);

    return NULL;
}
