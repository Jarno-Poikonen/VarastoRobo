#include "varastorobodevice.h"

VarastoRoboDevice::VarastoRoboDevice()
{

}

 // 1, 1, 1, 1, 4 bytes [7*8]);
VarastoRoboDevice::VarastoRoboDevice(quint8 type, quint8 id, Point point, Ipv4Address ipv4_address) :
    type(type), id(id), point(point), ipv4_address(ipv4_address)
{
    str = QString("\nVarastoRoboDevice: {\n") +
            QString("  type: ")         + QString::number(type) + QString("\n") +
            QString("  id: ")           + QString::number(id)   + QString("\n") +
            QString("  point: ")        + point.str             + QString("\n") +
            QString("  ipv4 address: ") + ipv4_address.str      + QString("\n") +
            QString("}");
}
