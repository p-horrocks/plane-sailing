#include "thread.h"

#include "units.h"
#include "util.h"
#include "point3d.h"

void* workerThread(void* params)
{
    ThreadParams* tp = reinterpret_cast<ThreadParams*>(params);

    PointSet altitudeTrack;
    PointSet planeSpeeds;

    bool abort = tp->cancelRequested;
    int count = 0;
    for(int i = 0; i < tp->iterationsPerThread; ++i, ++count)
    {
        Point3D crashPos;
        try
        {
            double time = createPointSets(*tp, true, 0.0, altitudeTrack, planeSpeeds);
            crashPos = CalcTrack(
                        tp->towerLocation,
                        tp->timeStep,
                        altitudeTrack,
                        tp->fixRange.sample(),
                        tp->fixBearing.sample(),
                        time,
                        tp->aircraftHeading.sample(),
                        tp->initialBankRate.sample(),
                        tp->bankRateAccel.sample(),
                        tp->windDirection.sample(),
                        tp->windProfile.sample(),
                        planeSpeeds
                        );
        }
        catch(...)
        {
            continue;
        }

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
        abort = tp->cancelRequested;

        pthread_mutex_unlock(&tp->mutex);

        if(abort)
            break;
    }

    pthread_mutex_lock(&tp->mutex);
    tp->completed += count;
    pthread_mutex_unlock(&tp->mutex);

    return NULL;
}
