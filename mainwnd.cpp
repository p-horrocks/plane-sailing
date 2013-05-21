#include "mainwnd.h"

#include <cassert>
#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVector2D>
#include <QVector3D>

#include "units.h"
#include "util.h"
#include "point3d.h"
#include "track3d.h"

namespace
{

const QString DATASET_KEY    = "DataSet";
const QString ITERATIONS_KEY = "Iterations";
const QString CELLSIZE_KEY   = "CellSize";
const QString NUMCELLS_KEY   = "NumCells";
const QString TIMESTEP_KEY   = "TimeStep";

const QString TOWEREAST_KEY         = "TowerEasting";
const QString TOWERNORTH_KEY        = "TowerNorthing";
const QString TOWERCELL_KEY         = "TowerGridCell";
const QString GRIDTOMAG_KEY         = "GridToMagnetic";
const QString FIXRANGEMEAN_KEY      = "FixRange.Mean";
const QString FIXRANGESTD_KEY       = "FixRange.Std";
const QString FIXBEARINGMEAN_KEY    = "FixBearing.Mean";
const QString FIXBEARINGSTD_KEY     = "FixBearing.Std";
const QString FIXTIMEMEAN_KEY       = "FixTime.Mean";
const QString FIXTIMESTD_KEY        = "FixTime.Std";
const QString CRASHTIMEMEAN_KEY     = "CrashTime.Mean";
const QString CRASHTIMESTD_KEY      = "CrashTime.Std";
const QString WINDDIRECTIONMEAN_KEY = "WindDirection.Mean";
const QString WINDDIRECTIONSTD_KEY  = "WindDirection.Std";
const QString PLANEHEADINGMEAN_KEY  = "PlaneHeading.Mean";
const QString PLANEHEADINGSTD_KEY   = "PlaneHeading.Std";
const QString INITIALSPEEDMEAN_KEY  = "InitialSpeed.Mean";
const QString INITIALSPEEDSTD_KEY   = "InitialSpeed.Std";
const QString FINALSPEEDMEAN_KEY    = "FinalSpeed.Mean";
const QString FINALSPEEDSTD_KEY     = "FinalSpeed.Std";
const QString BANKRATEMEAN_KEY      = "BankRate.Mean";
const QString BANKRATESTD_KEY       = "BankRate.Std";
const QString BANKACCELMEAN_KEY     = "BankAccel.Mean";
const QString BANKACCELSTD_KEY      = "BankAccel.Std";

const QString WINDPROFILE_KEY       = "WindProfile";
const QString FLIGHTPROFILE_KEY     = "FlightProfile";

const double nullDbl = 1e99;

void populateRow(QTableWidget* table, int row, int cols, ...)
{
    va_list args;
    va_start(args, cols);
    for(int c = 0; c < cols; ++c)
    {
        QTableWidgetItem* item = table->item(row, c);
        if(item == NULL)
        {
            item = new QTableWidgetItem;
            table->setItem(row, c, item);
        }
        double d = va_arg(args, double);
        if(d == nullDbl)
        {
            item->setText(QString());
        }
        else
        {
            item->setText(QString::number(d, 'f', 1));
        }
    }
}

QVariant maybeDouble(QTableWidgetItem* item)
{
    if(item == NULL)
        return QVariant();

    QString s = item->text();
    if(s.isEmpty())
        return QVariant();

    return s.toDouble();
}

} // namespace

MainWnd::MainWnd()
{
    timerId_ = 0;
    pthread_mutex_init(&params_.mutex, NULL);

    QString path = QString("%1/settings.ini").arg(QApplication::applicationDirPath());
    settings_ = new QSettings(path, QSettings::IniFormat);
    progress_ = new QProgressBar;
    progress_->setVisible(false);

    dataSetBox_ = new QComboBox;
    dataSetBox_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    dataSetBox_->setEditable(true);
    bool ok = connect(dataSetBox_, SIGNAL(activated(int)), this, SLOT(loadSettings()));
    assert(ok);

    QPushButton* saveBtn = new QPushButton(tr("Save"));
    ok = connect(saveBtn, SIGNAL(clicked()), this, SLOT(saveSettings()));
    assert(ok);

    QPushButton* resetBtn = new QPushButton(tr("Reset"));
    ok = connect(resetBtn, SIGNAL(clicked()), this, SLOT(loadSettings()));
    assert(ok);

    startBtn_ = new QPushButton(tr("Start"));
    ok = connect(startBtn_, SIGNAL(clicked()), this, SLOT(startStop()));
    assert(ok);

    QHBoxLayout* fileLayout = new QHBoxLayout;
    fileLayout->addWidget(dataSetBox_);
    fileLayout->addWidget(saveBtn);
    fileLayout->addWidget(resetBtn);

    QHBoxLayout* progressLayout = new QHBoxLayout;
    progressLayout->addStretch();
    progressLayout->addWidget(progress_);
    progressLayout->addWidget(startBtn_);

    QGridLayout* layout = new QGridLayout;
    layout->addLayout(fileLayout,        0, 0, 1, 2);
    layout->addWidget(createFixedBox(),  1, 0);
    layout->addWidget(createCalcBox(),   1, 1);
    layout->addWidget(createFlightBox(), 2, 0, 1, 2);
    layout->addWidget(createRandomBox(), 3, 0);
    layout->addWidget(createWindBox(),   3, 1);
    layout->addLayout(progressLayout,    4, 0, 1, 2);
    layout->setRowStretch(1, 1);
    layout->setRowStretch(2, 2);

    QWidget* widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);

    loadSettings();
    resize(850, 850);
}

MainWnd::~MainWnd()
{
    pthread_mutex_destroy(&params_.mutex);
}

void MainWnd::loadSettings()
{
    QString dataSet = settings_->value(DATASET_KEY, "Default").toString();
    int iterations  = settings_->value(ITERATIONS_KEY, 6).toInt();
    double cellSize = settings_->value(CELLSIZE_KEY, 1000.0).toDouble();
    int numCells    = settings_->value(NUMCELLS_KEY, 50).toInt();
    double timeStep = settings_->value(TIMESTEP_KEY, 1.0).toDouble();

    iterations_->setText(QString::number(iterations));
    cellSize_->setText(QString::number(cellSize, 'f', 1));
    numCells_->setText(QString::number(numCells));
    timeStep_->setText(QString::number(timeStep, 'f', 1));

    dataSetBox_->clear();
    QStringList sets = settings_->childGroups();
    if(sets.isEmpty())
    {
        addDefaultDataSet();
        dataSetBox_->setCurrentIndex(0);
    }
    else
    {
        dataSetBox_->addItems(sets);
        dataSetBox_->setCurrentIndex(sets.indexOf(dataSet));
    }
    dataSet += '/';

    double towerEasting      = settings_->value(dataSet + TOWEREAST_KEY).toDouble();
    double towerNorthing     = settings_->value(dataSet + TOWERNORTH_KEY).toDouble();
    QString towerCell        = settings_->value(dataSet + TOWERCELL_KEY).toString();
    double gridToMag         = settings_->value(dataSet + GRIDTOMAG_KEY).toDouble();
    double fixRangeMean      = settings_->value(dataSet + FIXRANGEMEAN_KEY).toDouble();
    double fixRangeStd       = settings_->value(dataSet + FIXRANGESTD_KEY).toDouble();
    double fixBearingMean    = settings_->value(dataSet + FIXBEARINGMEAN_KEY).toDouble();
    double fixBearingStd     = settings_->value(dataSet + FIXBEARINGSTD_KEY).toDouble();
    QString fixTimeMean      = settings_->value(dataSet + FIXTIMEMEAN_KEY).toString();
    double fixTimeStd        = settings_->value(dataSet + FIXTIMESTD_KEY).toDouble();
    QString crashTimeMean    = settings_->value(dataSet + CRASHTIMEMEAN_KEY).toString();
    double crashTimeStd      = settings_->value(dataSet + CRASHTIMESTD_KEY).toDouble();
    double windDirectionMean = settings_->value(dataSet + WINDDIRECTIONMEAN_KEY).toDouble();
    double windDirectionStd  = settings_->value(dataSet + WINDDIRECTIONSTD_KEY).toDouble();
    double planeHeadingMean  = settings_->value(dataSet + PLANEHEADINGMEAN_KEY).toDouble();
    double planeHeadingStd   = settings_->value(dataSet + PLANEHEADINGSTD_KEY).toDouble();
    double initialSpeedMean  = settings_->value(dataSet + INITIALSPEEDMEAN_KEY).toDouble();
    double initialSpeedStd   = settings_->value(dataSet + INITIALSPEEDSTD_KEY).toDouble();
    double finalSpeedMean    = settings_->value(dataSet + FINALSPEEDMEAN_KEY).toDouble();
    double finalSpeedStd     = settings_->value(dataSet + FINALSPEEDSTD_KEY).toDouble();
    double bankRateMean      = settings_->value(dataSet + BANKRATEMEAN_KEY).toDouble();
    double bankRateStd       = settings_->value(dataSet + BANKRATESTD_KEY).toDouble();
    double bankAccelMean     = settings_->value(dataSet + BANKACCELMEAN_KEY).toDouble();
    double bankAccelStd      = settings_->value(dataSet + BANKACCELSTD_KEY).toDouble();
    QVariantList windProfile = settings_->value(dataSet + WINDPROFILE_KEY).toList();
    QVariantList flightProfile = settings_->value(dataSet + FLIGHTPROFILE_KEY).toList();

    towerEasting_->setText(QString::number(towerEasting, 'f', 1));
    towerNorthing_->setText(QString::number(towerNorthing, 'f', 1));
    towerCell_->setText(towerCell);
    gridToMag_->setText(QString::number(gridToMag, 'f', 2));
    fixRangeMean_->setText(QString::number(fixRangeMean, 'f', 1));
    fixRangeStd_->setText(QString::number(fixRangeStd, 'f', 1));
    fixBearingMean_->setText(QString::number(fixBearingMean, 'f', 1));
    fixBearingStd_->setText(QString::number(fixBearingStd, 'f', 1));
    fixTimeMean_->setText(fixTimeMean);
    fixTimeStd_->setText(QString::number(fixTimeStd, 'f', 1));
    crashTimeMean_->setText(crashTimeMean);
    crashTimeStd_->setText(QString::number(crashTimeStd, 'f', 1));
    windDirectionMean_->setText(QString::number(windDirectionMean, 'f', 1));
    windDirectionStd_->setText(QString::number(windDirectionStd, 'f', 1));
    planeHeadingMean_->setText(QString::number(planeHeadingMean, 'f', 1));
    planeHeadingStd_->setText(QString::number(planeHeadingStd, 'f', 1));
    initialSpeedMean_->setText(QString::number(initialSpeedMean, 'f', 1));
    initialSpeedStd_->setText(QString::number(initialSpeedStd, 'f', 1));
    finalSpeedMean_->setText(QString::number(finalSpeedMean, 'f', 1));
    finalSpeedStd_->setText(QString::number(finalSpeedStd, 'f', 1));
    bankRateMean_->setText(QString::number(bankRateMean, 'f', 1));
    bankRateStd_->setText(QString::number(bankRateStd, 'f', 1));
    bankAccelMean_->setText(QString::number(bankAccelMean, 'f', 2));
    bankAccelStd_->setText(QString::number(bankAccelStd, 'f', 2));

    flightTable_->setRowCount(flightProfile.size());
    for(int i = 0; i < flightProfile.size(); ++i)
    {
        QVariantList v = flightProfile[i].toList();
        populateRow(flightTable_, i, 6, 0.0,
                    v[1].isNull() ? nullDbl : v[1].toDouble(),
                    v[2].isNull() ? nullDbl : v[2].toDouble(),
                    v[3].isNull() ? nullDbl : v[3].toDouble(),
                    v[4].isNull() ? nullDbl : v[4].toDouble(),
                    v[5].isNull() ? nullDbl : v[5].toDouble()
                    );
        flightTable_->item(i, 0)->setText(v[0].toString());
    }

    windTable_->setRowCount(windProfile.size());
    for(int i = 0; i < windProfile.size(); ++i)
    {
        QVector3D v = windProfile[i].value<QVector3D>();
        populateRow(windTable_, i, 3, v.x(), v.y(), v.z());
    }
}

void MainWnd::saveSettings()
{
    QString dataSet = dataSetBox_->currentText();
    settings_->setValue(DATASET_KEY, dataSet);
    dataSet += '/';

    int iterations           = iterations_->text().toInt();
    double cellSize          = cellSize_->text().toDouble();
    int numCells             = numCells_->text().toInt();
    double timeStep          = timeStep_->text().toDouble();
    double towerEasting      = towerEasting_->text().toDouble();
    double towerNorthing     = towerNorthing_->text().toDouble();
    QString towerCell        = towerCell_->text();
    double gridToMag         = gridToMag_->text().toDouble();
    double fixRangeMean      = fixRangeMean_->text().toDouble();
    double fixRangeStd       = fixRangeStd_->text().toDouble();
    double fixBearingMean    = fixBearingMean_->text().toDouble();
    double fixBearingStd     = fixBearingStd_->text().toDouble();
    QString fixTimeMean      = fixTimeMean_->text();
    double fixTimeStd        = fixTimeStd_->text().toDouble();
    QString crashTimeMean    = crashTimeMean_->text();
    double crashTimeStd      = crashTimeStd_->text().toDouble();
    double windDirectionMean = windDirectionMean_->text().toDouble();
    double windDirectionStd  = windDirectionStd_->text().toDouble();
    double planeHeadingMean  = planeHeadingMean_->text().toDouble();
    double planeHeadingStd   = planeHeadingStd_->text().toDouble();
    double initialSpeedMean  = initialSpeedMean_->text().toDouble();
    double initialSpeedStd   = initialSpeedStd_->text().toDouble();
    double finalSpeedMean    = finalSpeedMean_->text().toDouble();
    double finalSpeedStd     = finalSpeedStd_->text().toDouble();
    double bankRateMean      = bankRateMean_->text().toDouble();
    double bankRateStd       = bankRateStd_->text().toDouble();
    double bankAccelMean     = bankAccelMean_->text().toDouble();
    double bankAccelStd      = bankAccelStd_->text().toDouble();

    settings_->setValue(ITERATIONS_KEY, iterations);
    settings_->setValue(CELLSIZE_KEY, cellSize);
    settings_->setValue(NUMCELLS_KEY, numCells);
    settings_->setValue(TIMESTEP_KEY, timeStep);
    settings_->setValue(dataSet + TOWEREAST_KEY, towerEasting);
    settings_->setValue(dataSet + TOWERNORTH_KEY, towerNorthing);
    settings_->setValue(dataSet + TOWERCELL_KEY, towerCell);
    settings_->setValue(dataSet + GRIDTOMAG_KEY, gridToMag);
    settings_->setValue(dataSet + FIXRANGEMEAN_KEY, fixRangeMean);
    settings_->setValue(dataSet + FIXRANGESTD_KEY, fixRangeStd);
    settings_->setValue(dataSet + FIXBEARINGMEAN_KEY, fixBearingMean);
    settings_->setValue(dataSet + FIXBEARINGSTD_KEY, fixBearingStd);
    settings_->setValue(dataSet + FIXTIMEMEAN_KEY, fixTimeMean);
    settings_->setValue(dataSet + FIXTIMESTD_KEY, fixTimeStd);
    settings_->setValue(dataSet + CRASHTIMEMEAN_KEY, crashTimeMean);
    settings_->setValue(dataSet + CRASHTIMESTD_KEY, crashTimeStd);
    settings_->setValue(dataSet + WINDDIRECTIONMEAN_KEY, windDirectionMean);
    settings_->setValue(dataSet + WINDDIRECTIONSTD_KEY, windDirectionStd);
    settings_->setValue(dataSet + PLANEHEADINGMEAN_KEY, planeHeadingMean);
    settings_->setValue(dataSet + PLANEHEADINGSTD_KEY, planeHeadingStd);
    settings_->setValue(dataSet + INITIALSPEEDMEAN_KEY, initialSpeedMean);
    settings_->setValue(dataSet + INITIALSPEEDSTD_KEY, initialSpeedStd);
    settings_->setValue(dataSet + FINALSPEEDMEAN_KEY, finalSpeedMean);
    settings_->setValue(dataSet + FINALSPEEDSTD_KEY, finalSpeedStd);
    settings_->setValue(dataSet + BANKRATEMEAN_KEY, bankRateMean);
    settings_->setValue(dataSet + BANKRATESTD_KEY, bankRateStd);
    settings_->setValue(dataSet + BANKACCELMEAN_KEY, bankAccelMean);
    settings_->setValue(dataSet + BANKACCELSTD_KEY, bankAccelStd);

    QVariantList flightProfile;
    for(int i = 0; i < flightTable_->rowCount(); ++i)
    {
        QVariantList row;
        row.push_back(flightTable_->item(i, 0)->text());
        row.push_back(maybeDouble(flightTable_->item(i, 1)));
        row.push_back(maybeDouble(flightTable_->item(i, 2)));
        row.push_back(maybeDouble(flightTable_->item(i, 3)));
        row.push_back(maybeDouble(flightTable_->item(i, 4)));
        row.push_back(maybeDouble(flightTable_->item(i, 5)));
        flightProfile.push_back(row);
    }
    settings_->setValue(dataSet + FLIGHTPROFILE_KEY, flightProfile);

    QVariantList windProfile;
    for(int i = 0; i < windTable_->rowCount(); ++i)
    {
        QVector3D v;
        v.setX(windTable_->item(i, 0)->text().toDouble());
        v.setY(windTable_->item(i, 1)->text().toDouble());
        v.setZ(windTable_->item(i, 2)->text().toDouble());
        windProfile.push_back(v);
    }
    settings_->setValue(dataSet + WINDPROFILE_KEY, windProfile);

    // Re-populate the combo box and re-format all values.
    loadSettings();
}

void MainWnd::startStop()
{
    if(timerId_ == 0)
    {
        // No calculation running. Start a new one.
        double gridToMag = gridToMag_->text().toDouble();

        params_.cancelRequested  = false;
        params_.completed        = 0;
        params_.totalIterations  = std::round(pow(10, iterations_->text().toInt()));
        params_.towerLocation.x_ = towerEasting_->text().toDouble();
        params_.towerLocation.y_ = towerNorthing_->text().toDouble();
        if(towerCell_->text() == "56HLJ")
        {
            params_.towerLocation.y_ -= 100000.0;
        }
        params_.timeStep            = timeStep_->text().toDouble();
        params_.fixRange            = Distribution(NMToMetres(fixRangeMean_->text().toDouble()),               NMToMetres(fixRangeStd_->text().toDouble()));
        params_.fixBearing          = Distribution(DEG2RAD(fixBearingMean_->text().toDouble() + gridToMag),    DEG2RAD(fixBearingStd_->text().toDouble()));
        params_.aircraftHeading     = Distribution(DEG2RAD(planeHeadingMean_->text().toDouble() + gridToMag),  DEG2RAD(planeHeadingStd_->text().toDouble()));
        params_.initialBankRate     = Distribution(DEG2RAD(bankRateMean_->text().toDouble()),                  DEG2RAD(bankRateStd_->text().toDouble()));
        params_.bankRateAccel       = Distribution(DEG2RAD(bankAccelMean_->text().toDouble()),                 DEG2RAD(bankAccelStd_->text().toDouble()));
        params_.windDirection       = Distribution(DEG2RAD(windDirectionMean_->text().toDouble() + gridToMag), DEG2RAD(windDirectionStd_->text().toDouble()));

        time_t startTime = 0;
        params_.flightProfile.clear();
        for(int i = 0; i < flightTable_->rowCount(); ++i)
        {
            FlightPoint p;
            QTableWidgetItem* tm = flightTable_->item(i, 0);
            QTableWidgetItem* ts = flightTable_->item(i, 1);
            QTableWidgetItem* am = flightTable_->item(i, 2);
            QTableWidgetItem* as = flightTable_->item(i, 3);
            QTableWidgetItem* sm = flightTable_->item(i, 4);
            QTableWidgetItem* ss = flightTable_->item(i, 5);
            if(i == 0)
            {
                startTime = stringToTime(tm->text());
            }
            p.time = Distribution(difftime(stringToTime(tm->text()), startTime), ts->text().toDouble());
            if(!am->text().isNull() && !as->text().isNull())
            {
                p.altitude = Distribution(FeetToMetres(am->text().toDouble()), FeetToMetres(as->text().toDouble()));
            }
            if(!sm->text().isNull() && !ss->text().isNull())
            {
                p.speed = Distribution(KnotsToMPS(sm->text().toDouble()), KnotsToMPS(ss->text().toDouble()));
            }
            params_.flightProfile.push_back(p);
        }

        params_.windProfile.clear();
        for(int i = 0; i < windTable_->rowCount(); ++i)
        {
            double x = FeetToMetres(windTable_->item(i, 0)->text().toDouble());
            double y = KnotsToMPS(windTable_->item(i, 1)->text().toDouble());
            double z = KnotsToMPS(windTable_->item(i, 2)->text().toDouble());
            params_.windProfile.addPoint(x, y, z);
        }

        params_.gridCellsX          = numCells_->text().toInt();
        params_.gridCellsY          = numCells_->text().toInt();
        params_.metresPerCell       = cellSize_->text().toDouble();
        params_.grid.resize(params_.gridCellsX * params_.gridCellsY);
        std::fill(params_.grid.begin(), params_.grid.end(), 0);

        QString path = QString("%1/track.kml").arg(QApplication::applicationDirPath());
        kml_.reset(new KmlFile(path.toStdString()));

        // Need to know the nominal crash location to set up the grid origin.
        // This way we can centre the grid on the nominal crash pos.
        Point3D nominalCrashPos = createStdTracks(*kml_, params_);
        params_.gridOrigin      = Point2D(
                    nominalCrashPos.x_ - (params_.gridCellsX * params_.metresPerCell * 0.5),
                    nominalCrashPos.y_ - (params_.gridCellsY * params_.metresPerCell * 0.5)
                    );

        // Divvy the work load between some threads.
        static const int numThreads = 8;
        params_.iterationsPerThread = params_.totalIterations / numThreads;
        params_.totalIterations     = params_.iterationsPerThread * numThreads;
        for(int i = 0; i < numThreads; ++i)
        {
            pthread_t id;
            pthread_create(&id, NULL, workerThread, &params_);
            pthread_detach(id);
        }

        // Start a timer to track progress and check for completion.
        timerId_ = startTimer(100);
        progress_->setRange(0, params_.totalIterations);
        progress_->setVisible(true);
        startBtn_->setText(tr("Cancel"));
    }
    else
    {
        // Cancel the current simulation.
        pthread_mutex_lock(&params_.mutex);
        params_.cancelRequested = true;
        pthread_mutex_unlock(&params_.mutex);

        killTimer(timerId_);
        timerId_ = 0;
        progress_->setVisible(false);
        startBtn_->setText(tr("Start"));
    }
}

void MainWnd::addFlightRow()
{
    int row = flightTable_->rowCount();
    auto sel = flightTable_->selectedItems();
    if(!sel.isEmpty())
    {
        row = sel[0]->row();
    }
    flightTable_->insertRow(row);
    for(int i = 0; i < 6; ++i)
    {
        flightTable_->setItem(row, i, new QTableWidgetItem);
    }
}

void MainWnd::delFlightRow()
{
    auto sel = flightTable_->selectedItems();
    if(!sel.isEmpty())
    {
        flightTable_->removeRow(sel[0]->row());
    }
}

void MainWnd::addWindRow()
{
    int row = windTable_->rowCount();
    auto sel = windTable_->selectedItems();
    if(!sel.isEmpty())
    {
        row = sel[0]->row();
    }
    windTable_->insertRow(row);
    for(int i = 0; i < 3; ++i)
    {
        windTable_->setItem(row, i, new QTableWidgetItem);
    }
}

void MainWnd::delWindRow()
{
    auto sel = windTable_->selectedItems();
    if(!sel.isEmpty())
    {
        windTable_->removeRow(sel[0]->row());
    }
}

QWidget* MainWnd::createFixedBox()
{
    QStringList horz;
    horz.push_back(tr("Value"));

    QStringList vert;
    vert.push_back(tr("Tower location E (AMG)"));
    vert.push_back(tr("Tower location N (AMG)"));
    vert.push_back(tr("Tower UTM cell"));
    vert.push_back(tr("Grid to magnetic (deg)"));

    towerEasting_  = new QTableWidgetItem;
    towerNorthing_ = new QTableWidgetItem;
    towerCell_     = new QTableWidgetItem;
    gridToMag_     = new QTableWidgetItem;

    QTableWidget* table = new QTableWidget;
    table->setRowCount(vert.size());
    table->setColumnCount(horz.size());
    table->setHorizontalHeaderLabels(horz);
    table->setVerticalHeaderLabels(vert);
    table->setItem(0, 0, towerEasting_);
    table->setItem(1, 0, towerNorthing_);
    table->setItem(2, 0, towerCell_);
    table->setItem(3, 0, gridToMag_);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(table);

    QGroupBox* retval = new QGroupBox(tr("Fixed Parameters"));
    retval->setLayout(layout);
    return retval;
}

QWidget* MainWnd::createRandomBox()
{
    QStringList horz;
    horz.push_back(tr("Mean"));
    horz.push_back(tr("STD"));

    QStringList vert;
    vert.push_back(tr("Tower fix range (NM)"));
    vert.push_back(tr("Tower fix bearing (mag)"));
    vert.push_back(tr("Time of tower fix"));
    vert.push_back(tr("Time of crash"));
    vert.push_back(tr("Wind Direction (mag)"));
    vert.push_back(tr("Aircraft heading (mag)"));
    vert.push_back(tr("Initial aircraft speed (kn)"));
    vert.push_back(tr("Final aircraft speed (kn)"));
    vert.push_back(tr("Initial bank rate (deg/s)"));
    vert.push_back(tr("Bank rate acceleration (deg/s/s)"));

    fixRangeMean_      = new QTableWidgetItem;
    fixRangeStd_       = new QTableWidgetItem;
    fixBearingMean_    = new QTableWidgetItem;
    fixBearingStd_     = new QTableWidgetItem;
    fixTimeMean_       = new QTableWidgetItem;
    fixTimeStd_        = new QTableWidgetItem;
    crashTimeMean_     = new QTableWidgetItem;
    crashTimeStd_      = new QTableWidgetItem;
    windDirectionMean_ = new QTableWidgetItem;
    windDirectionStd_  = new QTableWidgetItem;
    planeHeadingMean_  = new QTableWidgetItem;
    planeHeadingStd_   = new QTableWidgetItem;
    initialSpeedMean_  = new QTableWidgetItem;
    initialSpeedStd_   = new QTableWidgetItem;
    finalSpeedMean_    = new QTableWidgetItem;
    finalSpeedStd_     = new QTableWidgetItem;
    bankRateMean_      = new QTableWidgetItem;
    bankRateStd_       = new QTableWidgetItem;
    bankAccelMean_     = new QTableWidgetItem;
    bankAccelStd_      = new QTableWidgetItem;
    fixTimeStd_->setFlags(Qt::NoItemFlags);

    QTableWidget* table = new QTableWidget;
    table->setRowCount(vert.size());
    table->setColumnCount(horz.size());
    table->setHorizontalHeaderLabels(horz);
    table->setVerticalHeaderLabels(vert);
    table->setItem(0, 0, fixRangeMean_);
    table->setItem(0, 1, fixRangeStd_);
    table->setItem(1, 0, fixBearingMean_);
    table->setItem(1, 1, fixBearingStd_);
    table->setItem(2, 0, fixTimeMean_);
    table->setItem(2, 1, fixTimeStd_);
    table->setItem(3, 0, crashTimeMean_);
    table->setItem(3, 1, crashTimeStd_);
    table->setItem(4, 0, windDirectionMean_);
    table->setItem(4, 1, windDirectionStd_);
    table->setItem(5, 0, planeHeadingMean_);
    table->setItem(5, 1, planeHeadingStd_);
    table->setItem(6, 0, initialSpeedMean_);
    table->setItem(6, 1, initialSpeedStd_);
    table->setItem(7, 0, finalSpeedMean_);
    table->setItem(7, 1, finalSpeedStd_);
    table->setItem(8, 0, bankRateMean_);
    table->setItem(8, 1, bankRateStd_);
    table->setItem(9, 0, bankAccelMean_);
    table->setItem(9, 1, bankAccelStd_);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(table);

    QGroupBox* retval = new QGroupBox(tr("Randomised Parameters"));
    retval->setLayout(layout);
    return retval;
}

QWidget* MainWnd::createCalcBox()
{
    QStringList horz;
    horz.push_back(tr("Value"));

    QStringList vert;
    vert.push_back(tr("Time integration step (s)"));
    vert.push_back(tr("Iterations"));
    vert.push_back(tr("Grid cells"));
    vert.push_back(tr("Metres per cell"));

    iterations_ = new QTableWidgetItem;
    numCells_   = new QTableWidgetItem;
    cellSize_   = new QTableWidgetItem;
    timeStep_   = new QTableWidgetItem;

    QTableWidget* table = new QTableWidget;
    table->setRowCount(vert.size());
    table->setColumnCount(horz.size());
    table->setHorizontalHeaderLabels(horz);
    table->setVerticalHeaderLabels(vert);
    table->setItem(0, 0, timeStep_);
    table->setItem(1, 0, iterations_);
    table->setItem(2, 0, numCells_);
    table->setItem(3, 0, cellSize_);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(table);

    QGroupBox* retval = new QGroupBox(tr("Calculation Parameters"));
    retval->setLayout(layout);
    return retval;
}

QWidget* MainWnd::createFlightBox()
{
    QStringList horz;
    horz.push_back(tr("Time Mean"));
    horz.push_back(tr("Time STD"));
    horz.push_back(tr("Altitude Mean (ft)"));
    horz.push_back(tr("Altitude STD (ft)"));
    horz.push_back(tr("Speed Mean (kn)"));
    horz.push_back(tr("Speed STD (kn)"));

    flightTable_ = new QTableWidget;
    flightTable_->setRowCount(0);
    flightTable_->setColumnCount(horz.size());
    flightTable_->setHorizontalHeaderLabels(horz);
    flightTable_->setSelectionMode(QAbstractItemView::SingleSelection);

    QPushButton* addBtn    = new QPushButton(tr("Insert"));
    bool ok = connect(addBtn, SIGNAL(clicked()), this, SLOT(addFlightRow()));
    assert(ok);

    QPushButton* deleteBtn = new QPushButton(tr("Delete"));
    ok = connect(deleteBtn, SIGNAL(clicked()), this, SLOT(delFlightRow()));
    assert(ok);

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(flightTable_, 0, 0, 1, 3);
    layout->addWidget(addBtn,     1, 1);
    layout->addWidget(deleteBtn,  1, 2);

    QGroupBox* retval = new QGroupBox(tr("Flight Profile"));
    retval->setLayout(layout);
    return retval;
}

QWidget* MainWnd::createWindBox()
{
    QStringList horz;
    horz.push_back(tr("Altitude (ft)"));
    horz.push_back(tr("Wind Speed Mean (kn)"));
    horz.push_back(tr("Wind Speed STD (kn)"));

    windTable_ = new QTableWidget;
    windTable_->setRowCount(0);
    windTable_->setColumnCount(horz.size());
    windTable_->setHorizontalHeaderLabels(horz);
    windTable_->setSelectionMode(QAbstractItemView::SingleSelection);

    QPushButton* addBtn    = new QPushButton(tr("Insert"));
    bool ok = connect(addBtn, SIGNAL(clicked()), this, SLOT(addWindRow()));
    assert(ok);

    QPushButton* deleteBtn = new QPushButton(tr("Delete"));
    ok = connect(deleteBtn, SIGNAL(clicked()), this, SLOT(delWindRow()));
    assert(ok);

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(windTable_, 0, 0, 1, 3);
    layout->addWidget(addBtn,     1, 1);
    layout->addWidget(deleteBtn,  1, 2);

    QGroupBox* retval = new QGroupBox(tr("Wind Profile"));
    retval->setLayout(layout);
    return retval;
}

void MainWnd::addDefaultDataSet()
{
    QString dataSet = "Default";

    dataSetBox_->addItem(dataSet);

    dataSet.append('/');
    settings_->setValue(dataSet + TOWEREAST_KEY,        90345.0);
    settings_->setValue(dataSet + TOWERNORTH_KEY,       69908.0);
    settings_->setValue(dataSet + TOWERCELL_KEY,        "56HLJ");
    settings_->setValue(dataSet + GRIDTOMAG_KEY,        11.63);

    settings_->setValue(dataSet + FIXRANGEMEAN_KEY,      48.0);
    settings_->setValue(dataSet + FIXRANGESTD_KEY,       0.5);
    settings_->setValue(dataSet + FIXBEARINGMEAN_KEY,    320.0);
    settings_->setValue(dataSet + FIXBEARINGSTD_KEY,     2.0);
    settings_->setValue(dataSet + FIXTIMEMEAN_KEY,       "19:36:00");
    settings_->setValue(dataSet + FIXTIMESTD_KEY,        0.0);
    settings_->setValue(dataSet + CRASHTIMEMEAN_KEY,     "19:39:27");
    settings_->setValue(dataSet + CRASHTIMESTD_KEY,      35.0);
    settings_->setValue(dataSet + WINDDIRECTIONMEAN_KEY, 230.0);
    settings_->setValue(dataSet + WINDDIRECTIONSTD_KEY,  10.0);
    settings_->setValue(dataSet + PLANEHEADINGMEAN_KEY,  140.0);
    settings_->setValue(dataSet + PLANEHEADINGSTD_KEY,   10.0);
    settings_->setValue(dataSet + INITIALSPEEDMEAN_KEY,  145.0);
    settings_->setValue(dataSet + INITIALSPEEDSTD_KEY,   10.0);
    settings_->setValue(dataSet + FINALSPEEDMEAN_KEY,    85.0);
    settings_->setValue(dataSet + FINALSPEEDSTD_KEY,     10.0);
    settings_->setValue(dataSet + BANKRATEMEAN_KEY,      0.0);
    settings_->setValue(dataSet + BANKRATESTD_KEY,       0.1);
    settings_->setValue(dataSet + BANKACCELMEAN_KEY,     0.0);
    settings_->setValue(dataSet + BANKACCELSTD_KEY,      0.02);

    static const struct
    {
        QVariant timeMean;
        QVariant timeStd;
        QVariant altitudeMean;
        QVariant altitudeStd;
        QVariant speedMean;
        QVariant speedStd;
    }
    defaultFlight[] =
    {
        { "19:36:00", 0.0,  8500.0, 0.0, 145.0, 10.0 },
        { "19:37:39", 0.0,  7500.0, 0.0, QVariant(), QVariant() },
        { "19:38:29", 0.0,  6500.0, 0.0, QVariant(), QVariant() },
        { "19:39:27", 35.0, 5500.0, 0.0, 85.0,  10.0 },
        { QVariant(), QVariant(), QVariant(), QVariant(), QVariant(), QVariant() }
    };

    QVariantList flight;
    for(int i = 0; !defaultFlight[i].timeMean.isNull(); ++i)
    {
        QVariantList item;
        item.push_back(defaultFlight[i].timeMean);
        item.push_back(defaultFlight[i].timeStd);
        item.push_back(defaultFlight[i].altitudeMean);
        item.push_back(defaultFlight[i].altitudeStd);
        item.push_back(defaultFlight[i].speedMean);
        item.push_back(defaultFlight[i].speedStd);
        flight.push_back(item);
    }
    settings_->setValue(dataSet + FLIGHTPROFILE_KEY, flight);

    QVariantList wind;
    wind.push_back(QVector3D(6000, 33, 10));
    wind.push_back(QVector3D(8000, 43, 10));
    settings_->setValue(dataSet + WINDPROFILE_KEY, wind);
}

void MainWnd::timerEvent(QTimerEvent*)
{
    pthread_mutex_lock(&params_.mutex);
    int complete  = params_.completed;
    bool finished = (params_.completed == params_.totalIterations);
    pthread_mutex_unlock(&params_.mutex);

    progress_->setValue(complete);
    if(finished)
    {
        killTimer(timerId_);
        timerId_ = 0;

        kml_->startFolder("Grid");
        int idx            = 0;
        double y           = params_.gridOrigin.y_;
        double highestCell = *std::max_element(params_.grid.begin(), params_.grid.end());
        for(int row = 0; row < params_.gridCellsY; ++row, y += params_.metresPerCell)
        {
            double x = params_.gridOrigin.x_;
            for(int col = 0; col < params_.gridCellsX; ++col, ++idx, x += params_.metresPerCell)
            {
                Track3D cell;
                cell.addPoint(x, y, 0);
                cell.addPoint(x + params_.metresPerCell, y, 0);
                cell.addPoint(x + params_.metresPerCell, y + params_.metresPerCell, 0);
                cell.addPoint(x, y + params_.metresPerCell, 0);
                cell.addPoint(x, y, 0);
                cell.convertAMG66toWGS84();

                const char* style;
                int level = std::round((4 * params_.grid[idx]) / highestCell);
                switch(level)
                {
                case 1:
                    style = "cell_25"; break;
                case 2:
                    style = "cell_50"; break;
                case 3:
                    style = "cell_75"; break;
                case 4:
                    style = "cell_100"; break;
                default:
                    style = ((row == 0) && (col == 0)) ? "origin_cell" : "empty_cell"; break;
                }
                kml_->addPolygon(cell, NULL, style, false);
            }
        }
        kml_.reset();

        progress_->setVisible(false);
        startBtn_->setText(tr("Start"));
    }
}
