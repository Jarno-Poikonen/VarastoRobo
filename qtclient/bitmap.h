#ifndef BITMAP_H
#define BITMAP_H

#include <QObject>

class BitMap
{
public:
    BitMap(quint8 width = 0, quint8 height = 0);
    ~BitMap();
    void set_bit(quint8 x, quint8 y, bool value);
    bool get_bit(quint8 x, quint8 y);
    bool& operator[](quint16 i);

private:
    bool* map;
    quint8 width;
    quint8 height;
    quint16 length;
};

#endif // BITMAP_H
