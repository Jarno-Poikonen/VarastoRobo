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
    ~SystemBroadcastMessage();
    void parse_datagram_string(char* datagram_string, qint64 datagram_size);
    QString str;

private:
    // needed for memory deallocation
    quint16 map_length;

    // smb
    quint16 constant;
    quint8 state;
    quint8 master_id;
    quint8 map_height;      // 0-5 (6)
    quint8 map_width;       // 0-8 (9)
    quint8 obstacle_count;  // max 54
    quint8 device_count;    // 4x GoPiGo, UR5, Drone, QtClient = 7 (master not included here)

    BitMap warehouse_bitmap; // (0, 0), (1, 0) .. (8, 0), (0, 1) [54]
    Point* obstacle_list;    // x, y // 1, 1 [54*2]
    VarastoRoboDevice* device_list;     // type, id, x, y, ipv4 // 1, 1, 1, 1, 4 bytes [7*8]
};

#endif // SYSTEMBROADCASTMESSAGE_H
