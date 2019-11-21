#include "point.h"

Point::Point() : x(0), y(0)
{
    str = QString("{x: ") + QString::number(x) + QString(", y: ") + QString::number(y) + QString("}");
}


Point::Point(quint8 x, quint8 y) : x(x), y(y)
{
    str = QString("{x: ") + QString::number(x) + QString(", y: ") + QString::number(y) + QString("}");
}
