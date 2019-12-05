#ifndef VARASTOROBODEVICE_H
#define VARASTOROBODEVICE_H

#include <QObject>
#include <QDebug>
#include "point.h"
#include "ipv4address.h"

class VarastoRoboDevice
{
public:
    VarastoRoboDevice();
    VarastoRoboDevice(quint8 type, quint8 id, Point point, Ipv4Address ipv4_address); // 1, 1, 1, 1, 4 bytes [7*8]);
    quint8 type;
    quint8 id;
    Point point;
    Ipv4Address ipv4_address;
    QString str;
};

#endif // VARASTOROBODEVICE_H
