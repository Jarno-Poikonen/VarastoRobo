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
    void parse_datagram();
    void gui_print();
    void debug_print();
    void gui_and_debug_print();
    QString str;
    quint8 obstacle_count;              // max 54
    quint8 device_count;                // 4x GoPiGo, UR5, Drone, QtClient = 7 (master not included here)
    Point obstacle_list[54];            // x, y // 1, 1 [54*2]
    VarastoRoboDevice device_list[32];  // type, id, x, y, ipv4 // 1, 1, 1, 1, 4 bytes [7*8]

private:
    char* datagram;
    qint64 datagram_size;
    quint16 map_length;

    // sbm
    quint16 constant;
    quint8 state;
    quint8 master_id;
    quint8 map_height;      // 0-5 (6)
    quint8 map_width;       // 0-8 (9)

    BitMap warehouse_bitmap; // (0, 0), (1, 0) .. (8, 0), (0, 1) [54]
    QString map_str;
    QString obstacles_str;
    QString devices_str;
};

#endif // SYSTEMBROADCASTMESSAGE_H
