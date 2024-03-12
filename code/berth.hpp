#ifndef __BERTH_H__
#define __BERTH_H__
#include "config.hpp"
struct Berth {
    int id;
    int x, y; // 坐标
    int time; // 运输到虚拟点的时间  
    int velocity; // 装载速度
    Berth(int id, int x, int y, int time, int velocity) : id(id), x(x), y(y), time(time), velocity(velocity) {}
};

Berth *berths[MAX_Berth_Num];

#endif