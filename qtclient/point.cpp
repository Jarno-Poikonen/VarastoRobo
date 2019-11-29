#include "point.h"

Point::Point() : x(0), y(0)
{
    str = "{x: " + QString::number(x) + ", y: " + QString::number(y) + "}";
}

Point::Point(quint8 x, quint8 y) : x(x), y(y)
{
    str = "{x: " + QString::number(x) + ", y: " + QString::number(y) + "}";
}
