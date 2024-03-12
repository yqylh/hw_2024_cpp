#ifndef __SHIP_H__
#define __SHIP_H__
#include "config.hpp"
struct Ship {
    int id;
    int x, y; // 坐标
    int status; // 0 移动(运输)中 1正常状态(即装货状态或运输完成状态) 2 泊位外等待状态
    int berthId; // 表示目标泊位，如果目标泊位是虚拟点，则为-1
    Ship(int id, int x, int y): id(id), x(x), y(y) {}
};

Ship *ships[MAX_Ship_Num];

#endif