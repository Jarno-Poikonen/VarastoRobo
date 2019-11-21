#ifndef POINT_H
#define POINT_H

#include <QObject>

class Point
{
public:
    Point();
    Point(quint8 x, quint8 y);
    quint8 x;
    quint8 y;
    QString str;
};

#endif // POINT_H
