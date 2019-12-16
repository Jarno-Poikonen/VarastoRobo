#ifndef SYSTEMBROADCASTMESSAGE_H
#define SYSTEMBROADCASTMESSAGE_H

#include <QtMath>
#include <QDebug>
#include "bitmap.h"
#include "point.h"
#include "varastorobodevice.h"

class SystemBroadcastMessage
{
public:
    SystemBroadcastMessage();
    SystemBroadcastMessage(char* datagram, qint64 datagram_size);
    ~SystemBroadcastMessage();

    Point* get_obstacle_list();
    VarastoRoboDevice* get_device_list();

    quint8 get_state();
    quint8 get_device_count();
    quint8 get_obstacle_count();

    QString get_str();

    void reset();

private:
    void parse_datagram();

    Point obstacle_list[60];
    VarastoRoboDevice device_list[32];
    BitMap warehouse_bitmap;

    char* datagram;

    qint64 datagram_size;

    quint16 constant;
    quint16 map_length;

    quint8 device_count;
    quint8 obstacle_count;
    quint8 state;
    quint8 map_height;
    quint8 map_width;
    quint8 master_id;

    QString devices_str;
    QString map_str;
    QString obstacles_str;
    QString str;

};

#endif // SYSTEMBROADCASTMESSAGE_H
