#include "units.h"

#include <cstring>

namespace
{

const char* TIME_FORMAT = "%H:%M:%S";

} // namespace

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

time_t stringToTime(const QString& timeStr)
{
    return stringToTime(timeStr.toAscii().data());
}

time_t stringToTime(const char* time)
{
    struct tm t;
    memset(&t, 0, sizeof(t));
    strptime(time, TIME_FORMAT, &t);
    return mktime(&t);
}
