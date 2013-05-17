#ifndef MAINWND_H
#define MAINWND_H

#include <QMainWindow>
#include <QComboBox>
#include <QSettings>
#include <QTableWidget>
#include <QProgressBar>
#include "thread.h"

class MainWnd : public QMainWindow
{
    Q_OBJECT

public:
    MainWnd();
    ~MainWnd();

public slots:
    void loadSettings();
    void saveSettings();
    void startStop();

protected:
    QWidget* createFixedBox();
    QWidget* createRandomBox();
    QWidget* createCalcBox();
    void addDefaultDataSet();
    virtual void timerEvent(QTimerEvent*);

    QSettings*    settings_;
    QComboBox*    dataSetBox_;
    QProgressBar* progress_;
    ThreadParams  params_;
    int           timerId_;

    QTableWidgetItem* timeStep_;
    QTableWidgetItem* numCells_;
    QTableWidgetItem* cellSize_;
    QTableWidgetItem* iterations_;

    QTableWidgetItem* towerEasting_;
    QTableWidgetItem* towerNorthing_;
    QTableWidgetItem* towerCell_;
    QTableWidgetItem* gridToMag_;

    QTableWidgetItem* fixRangeMean_;
    QTableWidgetItem* fixRangeStd_;
    QTableWidgetItem* fixBearingMean_;
    QTableWidgetItem* fixBearingStd_;
    QTableWidgetItem* fixTimeMean_;
    QTableWidgetItem* fixTimeStd_;
    QTableWidgetItem* crashTimeMean_;
    QTableWidgetItem* crashTimeStd_;
    QTableWidgetItem* wind6000Mean_;
    QTableWidgetItem* wind6000Std_;
    QTableWidgetItem* wind8000Mean_;
    QTableWidgetItem* wind8000Std_;
    QTableWidgetItem* windDirectionMean_;
    QTableWidgetItem* windDirectionStd_;
    QTableWidgetItem* planeHeadingMean_;
    QTableWidgetItem* planeHeadingStd_;
    QTableWidgetItem* initialSpeedMean_;
    QTableWidgetItem* initialSpeedStd_;
    QTableWidgetItem* finalSpeedMean_;
    QTableWidgetItem* finalSpeedStd_;
    QTableWidgetItem* bankRateMean_;
    QTableWidgetItem* bankRateStd_;
    QTableWidgetItem* bankAccelMean_;
    QTableWidgetItem* bankAccelStd_;
};

#endif // MAINWND_H
