#ifndef __BERTH_H__
#define __BERTH_H__
#include "config.hpp"
struct Berth {
    int id;
    Pos pos;
    int time; // 运输到虚拟点的时间  
    int velocity; // 装载速度
    int shipId; // 表示当前泊位上的船的 id 如果没有则为-1
    Berth(int id, int x, int y, int time, int velocity) : id(id), time(time), velocity(velocity) {
        this->pos = Pos(x, y);
        shipId = -1;
    }
};

Berth *berths[MAX_Berth_Num];

#endif