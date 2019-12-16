#include "bitmap.h"

BitMap::BitMap(quint8 width, quint8 height) : width(width), height(height), length(width*height)
{

}

BitMap::~BitMap()
{

}


bool& BitMap::operator[](quint16 i)
{
    return map[i];
}
