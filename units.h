#ifndef UNITS_H
#define UNITS_H

#include <ctime>
#include <QString>

#define DEG2RAD(x) ((x) * M_PI / 180.0)
#define RAD2DEG(x) ((x) * 180.0 / M_PI)

double FeetToMetres(double Feet);
double MetresToFeet(double Metres);
double NMToMetres(double NM);
double KnotsToMPS(double Knots);
double MPSToKnots(double MPS);
double stringToTime(const QString& timeStr);
double stringToTime(const char* timeStr);

#endif // UNITS_H
