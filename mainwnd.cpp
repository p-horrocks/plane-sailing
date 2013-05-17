#include "mainwnd.h"

#include <cassert>
#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>

namespace
{

const QString ITERATIONS_KEY = "Iterations";
const QString CELLSIZE_KEY   = "CellSize";
const QString NUMCELLS_KEY   = "NumCells";
const QString TIMESTEP_KEY   = "TimeStep";

const QString TOWEREAST_KEY        = "TowerEasting";
const QString TOWERNORTH_KEY       = "TowerNorthing";
const QString TOWERCELL_KEY        = "TowerGridCell";
const QString GRIDTOMAG_KEY        = "GridToMagnetic";
const QString FIXRANGEMEAN_KEY     = "FixRange.Mean";
const QString FIXRANGESTD_KEY      = "FixRange.Std";
const QString FIXBEARINGMEAN_KEY   = "FixBearing.Mean";
const QString FIXBEARINGSTD_KEY    = "FixBearing.Std";
const QString FIXTIMEMEAN_KEY      = "FixTime.Mean";
const QString FIXTIMESTD_KEY       = "FixTime.Std";
const QString CRASHTIMEMEAN_KEY    = "CrashTime.Mean";
const QString CRASHTIMESTD_KEY     = "CrashTime.Std";
const QString WIND6000MEAN_KEY     = "Wind6000.Mean";
const QString WIND6000STD_KEY      = "Wind6000.Std";
const QString WIND8000MEAN_KEY     = "Wind8000.Mean";
const QString WIND8000STD_KEY      = "Wind8000.Std";
const QString PLANEHEADINGMEAN_KEY = "PlaneHeading.Mean";
const QString PLANEHEADINGSTD_KEY  = "PlaneHeading.Std";
const QString INITIALSPEEDMEAN_KEY = "InitialSpeed.Mean";
const QString INITIALSPEEDSTD_KEY  = "InitialSpeed.Std";
const QString FINALSPEEDMEAN_KEY   = "FinalSpeed.Mean";
const QString FINALSPEEDSTD_KEY    = "FinalSpeed.Std";
const QString BANKRATEMEAN_KEY     = "BankRate.Mean";
const QString BANKRATESTD_KEY      = "BankRate.Std";
const QString BANKACCELMEAN_KEY    = "BankAccel.Mean";
const QString BANKACCELSTD_KEY     = "BankAccel.Std";

} // namespace

MainWnd::MainWnd()
{
    QString path = QString("%1/settings.ini").arg(QApplication::applicationDirPath());
    settings_ = new QSettings(path, QSettings::IniFormat);

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

    QHBoxLayout* fileLayout = new QHBoxLayout;
    fileLayout->addWidget(dataSetBox_);
    fileLayout->addWidget(saveBtn);
    fileLayout->addWidget(resetBtn);

    QGridLayout* layout = new QGridLayout;
    layout->addLayout(fileLayout,        0, 0, 1, 2);
    layout->addWidget(createFixedBox(),  1, 0);
    layout->addWidget(createCalcBox(),   1, 1);
    layout->addWidget(createRandomBox(), 2, 0, 1, 2);
    layout->setRowStretch(1, 1);
    layout->setRowStretch(2, 2);

    QWidget* widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);

    loadSettings();
    resize(850, 850);
}

void MainWnd::loadSettings()
{
    int iterations  = settings_->value(ITERATIONS_KEY, 6).toInt();
    double cellSize = settings_->value(CELLSIZE_KEY, 1000.0).toDouble();
    int numCells    = settings_->value(NUMCELLS_KEY, 50).toInt();
    double timeStep = settings_->value(TIMESTEP_KEY, 1.0).toDouble();

    iterations_->setText(QString::number(iterations));
    cellSize_->setText(QString::number(cellSize, 'f', 1));
    numCells_->setText(QString::number(numCells));
    timeStep_->setText(QString::number(timeStep, 'f', 1));

    QStringList sets = settings_->childGroups();
    if(sets.isEmpty())
    {
        addDefaultDataSet();
        dataSetBox_->setCurrentIndex(0);
    }
    else
    {
        QString dataSet = dataSetBox_->currentText();
        dataSetBox_->clear();
        dataSetBox_->addItems(sets);
        dataSetBox_->setCurrentIndex(sets.indexOf(dataSet));
    }
    int idx = std::max(0, dataSetBox_->currentIndex());
    QString dataSet = dataSetBox_->itemText(idx).append('/');

    double towerEasting     = settings_->value(dataSet + TOWEREAST_KEY).toDouble();
    double towerNorthing    = settings_->value(dataSet + TOWERNORTH_KEY).toDouble();
    QString towerCell       = settings_->value(dataSet + TOWERCELL_KEY).toString();
    double gridToMag        = settings_->value(dataSet + GRIDTOMAG_KEY).toDouble();
    double fixRangeMean     = settings_->value(dataSet + FIXRANGEMEAN_KEY).toDouble();
    double fixRangeStd      = settings_->value(dataSet + FIXRANGESTD_KEY).toDouble();
    double fixBearingMean   = settings_->value(dataSet + FIXBEARINGMEAN_KEY).toDouble();
    double fixBearingStd    = settings_->value(dataSet + FIXBEARINGSTD_KEY).toDouble();
    QString fixTimeMean     = settings_->value(dataSet + FIXTIMEMEAN_KEY).toString();
    double fixTimeStd       = settings_->value(dataSet + FIXTIMESTD_KEY).toDouble();
    QString crashTimeMean   = settings_->value(dataSet + CRASHTIMEMEAN_KEY).toString();
    double crashTimeStd     = settings_->value(dataSet + CRASHTIMESTD_KEY).toDouble();
    double wind6000Mean     = settings_->value(dataSet + WIND6000MEAN_KEY).toDouble();
    double wind6000Std      = settings_->value(dataSet + WIND6000STD_KEY).toDouble();
    double wind8000Mean     = settings_->value(dataSet + WIND8000MEAN_KEY).toDouble();
    double wind8000Std      = settings_->value(dataSet + WIND8000STD_KEY).toDouble();
    double planeHeadingMean = settings_->value(dataSet + PLANEHEADINGMEAN_KEY).toDouble();
    double planeHeadingStd  = settings_->value(dataSet + PLANEHEADINGSTD_KEY).toDouble();
    double initialSpeedMean = settings_->value(dataSet + INITIALSPEEDMEAN_KEY).toDouble();
    double initialSpeedStd  = settings_->value(dataSet + INITIALSPEEDSTD_KEY).toDouble();
    double finalSpeedMean   = settings_->value(dataSet + FINALSPEEDMEAN_KEY).toDouble();
    double finalSpeedStd    = settings_->value(dataSet + FINALSPEEDSTD_KEY).toDouble();
    double bankRateMean     = settings_->value(dataSet + BANKRATEMEAN_KEY).toDouble();
    double bankRateStd      = settings_->value(dataSet + BANKRATESTD_KEY).toDouble();
    double bankAccelMean    = settings_->value(dataSet + BANKACCELMEAN_KEY).toDouble();
    double bankAccelStd     = settings_->value(dataSet + BANKACCELSTD_KEY).toDouble();

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
    wind6000Mean_->setText(QString::number(wind6000Mean, 'f', 1));
    wind6000Std_->setText(QString::number(wind6000Std, 'f', 1));
    wind8000Mean_->setText(QString::number(wind8000Mean, 'f', 1));
    wind8000Std_->setText(QString::number(wind8000Std, 'f', 1));
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
}

void MainWnd::saveSettings()
{
    QString dataSet = dataSetBox_->currentText().append('/');

    int iterations          = iterations_->text().toInt();
    double cellSize         = cellSize_->text().toDouble();
    int numCells            = numCells_->text().toInt();
    double timeStep         = timeStep_->text().toDouble();
    double towerEasting     = towerEasting_->text().toDouble();
    double towerNorthing    = towerNorthing_->text().toDouble();
    QString towerCell       = towerCell_->text();
    double gridToMag        = gridToMag_->text().toDouble();
    double fixRangeMean     = fixRangeMean_->text().toDouble();
    double fixRangeStd      = fixRangeStd_->text().toDouble();
    double fixBearingMean   = fixBearingMean_->text().toDouble();
    double fixBearingStd    = fixBearingStd_->text().toDouble();
    QString fixTimeMean     = fixTimeMean_->text();
    double fixTimeStd       = fixTimeStd_->text().toDouble();
    QString crashTimeMean   = crashTimeMean_->text();
    double crashTimeStd     = crashTimeStd_->text().toDouble();
    double wind6000Mean     = wind6000Mean_->text().toDouble();
    double wind6000Std      = wind6000Std_->text().toDouble();
    double wind8000Mean     = wind8000Mean_->text().toDouble();
    double wind8000Std      = wind8000Std_->text().toDouble();
    double planeHeadingMean = planeHeadingMean_->text().toDouble();
    double planeHeadingStd  = planeHeadingStd_->text().toDouble();
    double initialSpeedMean = initialSpeedMean_->text().toDouble();
    double initialSpeedStd  = initialSpeedStd_->text().toDouble();
    double finalSpeedMean   = finalSpeedMean_->text().toDouble();
    double finalSpeedStd    = finalSpeedStd_->text().toDouble();
    double bankRateMean     = bankRateMean_->text().toDouble();
    double bankRateStd      = bankRateStd_->text().toDouble();
    double bankAccelMean    = bankAccelMean_->text().toDouble();
    double bankAccelStd     = bankAccelStd_->text().toDouble();

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
    settings_->setValue(dataSet + WIND6000MEAN_KEY, wind6000Mean);
    settings_->setValue(dataSet + WIND6000STD_KEY, wind6000Std);
    settings_->setValue(dataSet + WIND8000MEAN_KEY, wind8000Mean);
    settings_->setValue(dataSet + WIND8000STD_KEY, wind8000Std);
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

    // Re-populate the combo box and re-format all values.
    loadSettings();
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
    vert.push_back(tr("Wind speed at 6000ft (kn)"));
    vert.push_back(tr("Wind speed at 8000ft (kn)"));
    vert.push_back(tr("Aircraft heading (mag)"));
    vert.push_back(tr("Initial aircraft speed (kn)"));
    vert.push_back(tr("Final aircraft speed (kn)"));
    vert.push_back(tr("Initial bank rate (deg/s)"));
    vert.push_back(tr("Bank rate acceleration (deg/s/s)"));

    fixRangeMean_     = new QTableWidgetItem;
    fixRangeStd_      = new QTableWidgetItem;
    fixBearingMean_   = new QTableWidgetItem;
    fixBearingStd_    = new QTableWidgetItem;
    fixTimeMean_      = new QTableWidgetItem;
    fixTimeStd_       = new QTableWidgetItem;
    crashTimeMean_    = new QTableWidgetItem;
    crashTimeStd_     = new QTableWidgetItem;
    wind6000Mean_     = new QTableWidgetItem;
    wind6000Std_      = new QTableWidgetItem;
    wind8000Mean_     = new QTableWidgetItem;
    wind8000Std_      = new QTableWidgetItem;
    planeHeadingMean_ = new QTableWidgetItem;
    planeHeadingStd_  = new QTableWidgetItem;
    initialSpeedMean_ = new QTableWidgetItem;
    initialSpeedStd_  = new QTableWidgetItem;
    finalSpeedMean_   = new QTableWidgetItem;
    finalSpeedStd_    = new QTableWidgetItem;
    bankRateMean_     = new QTableWidgetItem;
    bankRateStd_      = new QTableWidgetItem;
    bankAccelMean_    = new QTableWidgetItem;
    bankAccelStd_     = new QTableWidgetItem;

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
    table->setItem(4, 0, wind6000Mean_);
    table->setItem(4, 1, wind6000Std_);
    table->setItem(5, 0, wind8000Mean_);
    table->setItem(5, 1, wind8000Std_);
    table->setItem(6, 0, planeHeadingMean_);
    table->setItem(6, 1, planeHeadingStd_);
    table->setItem(7, 0, initialSpeedMean_);
    table->setItem(7, 1, initialSpeedStd_);
    table->setItem(8, 0, finalSpeedMean_);
    table->setItem(8, 1, finalSpeedStd_);
    table->setItem(9, 0, bankRateMean_);
    table->setItem(9, 1, bankRateStd_);
    table->setItem(10, 0, bankAccelMean_);
    table->setItem(10, 1, bankAccelStd_);

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

void MainWnd::addDefaultDataSet()
{
    QString dataSet = "Default";

    dataSetBox_->addItem(dataSet);

    dataSet.append('/');
    settings_->setValue(dataSet + TOWEREAST_KEY,        90345.0);
    settings_->setValue(dataSet + TOWERNORTH_KEY,       69908.0);
    settings_->setValue(dataSet + TOWERCELL_KEY,        "56HLJ");
    settings_->setValue(dataSet + GRIDTOMAG_KEY,        11.63);

    settings_->setValue(dataSet + FIXRANGEMEAN_KEY,     48.0);
    settings_->setValue(dataSet + FIXRANGESTD_KEY,      0.5);
    settings_->setValue(dataSet + FIXBEARINGMEAN_KEY,   320.0);
    settings_->setValue(dataSet + FIXBEARINGSTD_KEY,    2.0);
    settings_->setValue(dataSet + FIXTIMEMEAN_KEY,      "19:36:00");
    settings_->setValue(dataSet + FIXTIMESTD_KEY,       20.0);
    settings_->setValue(dataSet + CRASHTIMEMEAN_KEY,    "19:39:27");
    settings_->setValue(dataSet + CRASHTIMESTD_KEY,     20.0);
    settings_->setValue(dataSet + WIND6000MEAN_KEY,     33.0);
    settings_->setValue(dataSet + WIND6000STD_KEY,      10.0);
    settings_->setValue(dataSet + WIND8000MEAN_KEY,     43.0);
    settings_->setValue(dataSet + WIND8000STD_KEY,      10.0);
    settings_->setValue(dataSet + PLANEHEADINGMEAN_KEY, 140.0);
    settings_->setValue(dataSet + PLANEHEADINGSTD_KEY,  10.0);
    settings_->setValue(dataSet + INITIALSPEEDMEAN_KEY, 145.0);
    settings_->setValue(dataSet + INITIALSPEEDSTD_KEY,  10.0);
    settings_->setValue(dataSet + FINALSPEEDMEAN_KEY,   85.0);
    settings_->setValue(dataSet + FINALSPEEDSTD_KEY,    10.0);
    settings_->setValue(dataSet + BANKRATEMEAN_KEY,     0.0);
    settings_->setValue(dataSet + BANKRATESTD_KEY,      0.1);
    settings_->setValue(dataSet + BANKACCELMEAN_KEY,    0.0);
    settings_->setValue(dataSet + BANKACCELSTD_KEY,     0.02);
}