#include "varastorobodevice.h"

VarastoRoboDevice::VarastoRoboDevice()
{

}

 // 1, 1, 1, 1, 4 bytes [7*8]);
VarastoRoboDevice::VarastoRoboDevice(quint8 type, quint8 id, Point point, Ipv4Address ipv4_address) :
    type(type), id(id), point(point), ipv4_address(ipv4_address)
{
    QString textual;
    switch(type)
    {
        case 0: textual = "(Master)";       break;
        case 1: textual = "(Qt Client)";    break;
        case 2: textual = "(GoPiGo)";       break;
        case 3: textual = "(UR5)";          break;
        case 4: textual = "(Drone)";        break;
        case 255: textual = "(Unknown)";    break;
    }
    str += "    VarastoRoboDevice: {\n";
    str += "      type: "         + QString::number(type) + " " + textual + "\n";
    str += "      id: "           + QString::number(id)   + "\n";
    str += "      point: "        + point.str             + "\n";
    str += "      ipv4 address: " + ipv4_address.str      + "\n";
    str += "    }\n";
}
