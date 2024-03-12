#ifndef __SHIP_H__
#define __SHIP_H__
#include "config.hpp"
class Ship {
    int id;
    int x, y; // 坐标
    Ship(int id, int x, int y): id(id), x(x), y(y) {}
};

Ship *ships[MAX_Ship_Num];

#endif