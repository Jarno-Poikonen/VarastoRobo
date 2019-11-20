#include "systembroadcastmessage.h"

SystemBroadcastMessage::SystemBroadcastMessage()
{

}

SystemBroadcastMessage::~SystemBroadcastMessage()
{
    if(obstacle_count > 0)
    {
        delete[] obstacle_list;
    }

    if(device_count > 0)
    {
        delete[] device_list;
    }
}

void SystemBroadcastMessage::parse_datagram_string(char* datagram_string, qint64 datagram_size)
{
    // System Broadcast Message parsing
    constant       = static_cast<quint16>(datagram_string[1]);
    constant     <<= 8;
    constant      |= static_cast<quint16>(datagram_string[0]);
    state          = static_cast<quint8>(datagram_string[2]);
    master_id      = static_cast<quint8>(datagram_string[3]);
    map_height     = static_cast<quint8>(datagram_string[4]);
    map_width      = static_cast<quint8>(datagram_string[5]);
    obstacle_count = static_cast<quint8>(datagram_string[6]);
    device_count   = static_cast<quint8>(datagram_string[7]);

    map_length = map_width * map_height;

    if(map_length > 0)
        warehouse_bitmap = BitMap(map_width, map_height);
    else
        qDebug() << "BAD BITMAP LENGTH! \n";

    if(obstacle_count > 0)
        obstacle_list = new Point[obstacle_count];
    else
        qDebug() << "BAD OBSTACLE COUNT! \n";

    if(device_count > 0)
        device_list = new VarastoRoboDevice[device_count];
//    else
//        qDebug() << "BAD DEVICE COUNT! \n";


    QString map_str;
    for(quint8 i = 0; i < map_length; ++i)
    {
        warehouse_bitmap[i] = static_cast<bool>(datagram_string[8 + i / 8] >> i % 8 & 1);
        map_str += QString("(x: ") + QString::number(i % map_width) + QString(", y: ") + QString::number(i / map_width);
        map_str += warehouse_bitmap[i] ? QString("): True\n") : QString("): False\n");
    }

    QString obstacles_str;
    quint8 map_bytes = static_cast<quint8>(qCeil(map_length / 8.0));
    for(quint8 i = 0; i < obstacle_count; ++i)
    {
        quint8 x = static_cast<quint8>(datagram_string[8 + map_bytes + i*2 + 0]);
        quint8 y = static_cast<quint8>(datagram_string[8 + map_bytes + i*2 + 1]);
        obstacle_list[i] = Point(x, y);
        obstacles_str += obstacle_list[i].str + QString("\n");
    }

    QString devices_str;
    quint16 obstacle_bytes = obstacle_count*2;
    for(quint8 i = 0; i < device_count; ++i)
    {
        quint8 type = static_cast<quint8>(datagram_string[8 + map_bytes + obstacle_bytes + i*8 + 0]);
        quint8 id   = static_cast<quint8>(datagram_string[8 + map_bytes + obstacle_bytes + i*8 + 1]);
        quint8 x    = static_cast<quint8>(datagram_string[8 + map_bytes + obstacle_bytes + i*8 + 2]);
        quint8 y    = static_cast<quint8>(datagram_string[8 + map_bytes + obstacle_bytes + i*8 + 3]);
        quint8 a    = static_cast<quint8>(datagram_string[8 + map_bytes + obstacle_bytes + i*8 + 7]);
        quint8 b    = static_cast<quint8>(datagram_string[8 + map_bytes + obstacle_bytes + i*8 + 6]);
        quint8 c    = static_cast<quint8>(datagram_string[8 + map_bytes + obstacle_bytes + i*8 + 5]);
        quint8 d    = static_cast<quint8>(datagram_string[8 + map_bytes + obstacle_bytes + i*8 + 4]);
        Ipv4Address ipv4_address(a, b, c, d);
        device_list[i] = VarastoRoboDevice(type, id, Point(x, y), ipv4_address);
        devices_str += device_list[i].str;
    }



    // save parsed datagram as a string for gui use
    str =   QString("datagram_size: ")     + QString::number(datagram_size)  + QString("\n") +
                            QString("constant: ")          + QString::number(constant)       + QString("\n") +
                            QString("state: ")             + QString::number(state)          + QString("\n") +
                            QString("master_id: ")         + QString::number(master_id)      + QString("\n") +
                            QString("map_height: ")        + QString::number(map_height)     + QString("\n") +
                            QString("map_width: ")         + QString::number(map_width)      + QString("\n") +
                            QString("obstacle_count: ")    + QString::number(obstacle_count) + QString("\n") +
                            QString("device_count: ")      + QString::number(device_count)   + QString("\n") +
                            QString("\nwarehouse_map: \n") + map_str +
                            QString("\nobstacles:\n")      + obstacles_str +
                            QString("\ndevices:")          + devices_str;
}

