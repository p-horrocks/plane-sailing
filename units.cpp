#include "units.h"

#include <cstring>

double FeetToMetres(double Feet)
{
    return Feet * 0.3048;
}

double MetresToFeet(double Metres)
{
    return Metres / 0.3048;
}

double NMToMetres(double NM)
{
    return NM * 1852.0;
}

double KnotsToMPS(double Knots)
{
    return Knots * 1852.0 / 3600.0;
}

double MPSToKnots(double MPS)
{
    return MPS * 3600.0 / 1852.0;
}

double stringToTime(const QString& timeStr)
{
    return stringToTime(timeStr.toAscii().data());
}

double stringToTime(const char* time)
{
    // strptime doesn't exist on Windows
    int hrs, min, sec;
    sscanf(time, "%d:%d:%d", &hrs, &min, &sec);
    return (hrs * 3600) + (min * 60) + sec;
}
