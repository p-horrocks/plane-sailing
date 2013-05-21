#include <QApplication>

#include "mainwnd.h"

#include <cmath>
#include <ctime>
#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
//#include <gdal/gdal_priv.h>

#include "util.h"
#include "pointset.h"
#include "kmlfile.h"
#include "track3d.h"
#include "point3d.h"
#include "distribution.h"
#include "units.h"
#include "thread.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWnd wnd;
    wnd.setVisible(true);
    return app.exec();

//    GDALAllRegister();
//    GDALDriver* driver   = GetGDALDriverManager()->GetDriverByName("VRT");
//    GDALDataset* ds      = driver->Create("", params.gridCellsX, params.gridCellsY, 1, GDT_Float64, 0);
//    GDALRasterBand* band = ds->GetRasterBand(1);

//    GDALRasterBandH band;
//    OGRLayerH layer;
//    GDALContourGenerate(
//                band, highestCell / 4.0, 0, 0, NULL, FALSE, 0, layer, -1, -1, NULL, NULL
//                );

//    GDALClose(ds);

//    for i in range(MapArray):
//        for j in range(MapArray):
//            dblDistribution[i,j] = 100.0*(float(lngDistribution[i,j]) / float(Iterations))

//    x=numpy.zeros((MapArray), dtype='float64')
//    y=numpy.zeros((MapArray), dtype='float64')

//    for i in range(MapArray):
//        x[i] = float((i+Array_E)*1000)+500.0
//        y[i] = float((i+Array_N)*1000)+500.0

//    MaxProb = dblDistribution.max()

//    # Make contours!
//    points = GetContourPoints(x,y,dblDistribution,MaxProb*0.25)

//    fred=[]
//    for i in range(points[0].size):
//        E,N = MGRSToUTM(points[1][i],points[0][i],"AGD66","WGS84")
//        fred.append(utmToLatLng(56,E,N,0))

//    pol2 = kml.newpolygon(name="{0}% per sq km region".format(MaxProb*0.25),
//                         outerboundaryis=fred)
//    pol2.polystyle.color = '660000ff'
//    pol2.polystyle.outline = 1

//    points = GetContourPoints(x,y,dblDistribution,MaxProb*0.5)

//    fred=[]
//    for i in range(points[0].size):
//        E,N = MGRSToUTM(points[1][i],points[0][i],"AGD66","WGS84")
//        fred.append(utmToLatLng(56,E,N,0))

//    pol3 = kml.newpolygon(name="{0}% per sq km region".format(MaxProb*0.5),
//                         outerboundaryis=fred)
//    pol3.polystyle.color = '6600ff00'
//    pol3.polystyle.outline = 1

//    points = GetContourPoints(x,y,dblDistribution,MaxProb*0.75)

//    fred=[]
//    for i in range(points[0].size):
//        E,N = MGRSToUTM(points[1][i],points[0][i],"AGD66","WGS84")
//        fred.append(utmToLatLng(56,E,N,0))

//    pol3 = kml.newpolygon(name="{0}% per sq km region".format(MaxProb*0.75),
//                         outerboundaryis=fred)
//    pol3.polystyle.color = '66ff0000'
//    pol3.polystyle.outline = 1

//    kml.save("track2quick.kml")

    return 0;
}
