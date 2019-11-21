#ifndef IPV4ADDRESS_H
#define IPV4ADDRESS_H

#include <QObject>

class Ipv4Address
{
public:
    Ipv4Address();
    Ipv4Address(quint8 a, quint8 b, quint8 c, quint8 d);
    quint8 a;
    quint8 b;
    quint8 c;
    quint8 d;
    QString str;
};

#endif // IPV4ADDRESS_H
