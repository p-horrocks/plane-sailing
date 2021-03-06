#ifndef MAINWND_H
#define MAINWND_H

#include <memory>
#include <QMainWindow>
#include <QComboBox>
#include <QSettings>
#include <QTableWidget>
#include <QProgressBar>
#include <QPushButton>
#include "thread.h"
#include "kmlfile.h"

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

    void addFlightRow();
    void delFlightRow();
    void addWindRow();
    void delWindRow();

protected:
    QWidget* createFixedBox();
    QWidget* createRandomBox();
    QWidget* createCalcBox();
    QWidget* createFlightBox();
    QWidget* createWindBox();
    void addDefaultDataSet();
    virtual void timerEvent(QTimerEvent*);

    QSettings*    settings_;
    QComboBox*    dataSetBox_;
    QProgressBar* progress_;
    QPushButton*  startBtn_;
    ThreadParams  params_;
    int           timerId_;
    std::shared_ptr<KmlFile> kml_;

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
    QTableWidgetItem* windDirectionMean_;
    QTableWidgetItem* windDirectionStd_;
    QTableWidgetItem* planeHeadingMean_;
    QTableWidgetItem* planeHeadingStd_;
    QTableWidgetItem* bankRateMean_;
    QTableWidgetItem* bankRateStd_;
    QTableWidgetItem* bankAccelMean_;
    QTableWidgetItem* bankAccelStd_;

    QTableWidget* flightTable_;
    QTableWidget* windTable_;
};

#endif // MAINWND_H
