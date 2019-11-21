#include "ipv4address.h"

Ipv4Address::Ipv4Address() : a(0), b(0), c(0), d(0)
{
    str = QString::number(a) + QString(".") +
          QString::number(b) + QString(".") +
          QString::number(c) + QString(".") +
          QString::number(d);
}

Ipv4Address::Ipv4Address(quint8 a, quint8 b, quint8 c, quint8 d) : a(a), b(b), c(c), d(d)
{
    str = QString::number(a) + QString(".") +
          QString::number(b) + QString(".") +
          QString::number(c) + QString(".") +
          QString::number(d);
}
