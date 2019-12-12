#include "systembroadcastmessage.h"

SystemBroadcastMessage::SystemBroadcastMessage()
{

}

SystemBroadcastMessage::SystemBroadcastMessage(char* datagram, qint64 datagram_size) : datagram(datagram), datagram_size(datagram_size)
{
    
}

SystemBroadcastMessage::~SystemBroadcastMessage()
{

}

void SystemBroadcastMessage::parse_datagram()
{
    // System Broadcast Message parsing
    constant       = static_cast<quint16>(datagram[1]);
    constant     <<= 8;
    constant      |= static_cast<quint16>(datagram[0]);
    state          = static_cast<quint8>(datagram[2]);
    master_id      = static_cast<quint8>(datagram[3]);
    map_height     = static_cast<quint8>(datagram[4]);
    map_width      = static_cast<quint8>(datagram[5]);
    obstacle_count = static_cast<quint8>(datagram[6]);
    device_count   = static_cast<quint8>(datagram[7]);

    map_length = map_width * map_height;
    warehouse_bitmap = BitMap(map_width, map_height);

    map_str = "  warehouse bitmap: {\n";
    obstacles_str = "  obstacles: {\n";
    devices_str = "  devices: {\n";

    quint8 map_bytes = static_cast<quint8>(qCeil(map_length / 8.0));
    quint8 obstacle_bytes = obstacle_count*2;

    for(quint8 i = 0; i < map_length; ++i)
    {
        warehouse_bitmap[i] = static_cast<bool>(datagram[8 + i / 8] >> i % 8 & 1);
        QString x = QString::number(i % map_width);
        QString y = QString::number(i / map_width);
        map_str += "    {x: " + x + ", y: " + y + "} => ";
        map_str += warehouse_bitmap[i] ? "True" : "False";
        map_str += "\n";
    }

    for(quint8 i = 0; i < obstacle_count; ++i)
    {
        quint8 x = static_cast<quint8>(datagram[8 + map_bytes + i*2 + 0]);
        quint8 y = static_cast<quint8>(datagram[8 + map_bytes + i*2 + 1]);
        obstacle_list[i] = Point(x, y);
        obstacles_str += "    " + obstacle_list[i].str +"\n";
    }

    for(quint8 i = 0; i < device_count; ++i)
    {
        quint8 type = static_cast<quint8>(datagram[8 + map_bytes + obstacle_bytes + i*8 + 0]);
        quint8 id   = static_cast<quint8>(datagram[8 + map_bytes + obstacle_bytes + i*8 + 1]);
        quint8 x    = static_cast<quint8>(datagram[8 + map_bytes + obstacle_bytes + i*8 + 2]);
        quint8 y    = static_cast<quint8>(datagram[8 + map_bytes + obstacle_bytes + i*8 + 3]);
        quint8 a    = static_cast<quint8>(datagram[8 + map_bytes + obstacle_bytes + i*8 + 7]);
        quint8 b    = static_cast<quint8>(datagram[8 + map_bytes + obstacle_bytes + i*8 + 6]);
        quint8 c    = static_cast<quint8>(datagram[8 + map_bytes + obstacle_bytes + i*8 + 5]);
        quint8 d    = static_cast<quint8>(datagram[8 + map_bytes + obstacle_bytes + i*8 + 4]);
        Ipv4Address ipv4_address(a, b, c, d);
        device_list[i] = VarastoRoboDevice(type, id, Point(x, y), ipv4_address);
        devices_str += device_list[i].str;
    }

    map_str += "  }\n";
    obstacles_str += "  }\n";
    devices_str += "  }\n";

    // save parsed datagram as a string for gui use
    str += "RECEIVED: (" + QString::number(datagram_size) + QString(" bytes)\n");
    str += "SMB: {\n";
    str += "  datagram_size: "     + QString::number(datagram_size)  + "\n";
    str += "  constant: "          + QString::number(constant)       + "\n";
    str += "  state: "             + QString::number(state)          + "\n";
    str += "  master_id: "         + QString::number(master_id)      + "\n";
    str += "  map_height: "        + QString::number(map_height)     + "\n";
    str += "  map_width: "         + QString::number(map_width)      + "\n";
    str += "  obstacle_count: "    + QString::number(obstacle_count) + "\n";
    str += "  device_count: "      + QString::number(device_count)   + "\n";
    str += map_str;
    str += obstacles_str;
    str += devices_str;
}

void SystemBroadcastMessage::reset()
{
    str = "";
    obstacle_count = 0;
    device_count = 0;
    state = 3;
}
