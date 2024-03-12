#ifndef __BERTH_H__
#define __BERTH_H__
#include "config.hpp"
struct Berth {
    int id;
    Pos pos;
    int time; // 运输到虚拟点的时间  
    int velocity; // 装载速度
    Berth(int id, int x, int y, int time, int velocity) : id(id), time(time), velocity(velocity) {
        this->pos = Pos(x, y);
    }
};

Berth *berths[MAX_Berth_Num];

#endif