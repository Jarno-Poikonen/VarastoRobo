#include "bitmap.h"

BitMap::BitMap(quint8 width, quint8 height) : width(width), height(height), length(width*height)
{
    if(length > 0)
        map = new bool[length];
    else
        map = nullptr;
}

BitMap::~BitMap()
{
    if(length > 0)
        delete[] map;
}

void BitMap::set_bit(quint8 x, quint8 y, bool value)
{

}

bool BitMap::get_bit(quint8 x, quint8 y)
{

}

bool& BitMap::operator[](quint16 i)
{
    return map[i];
}
