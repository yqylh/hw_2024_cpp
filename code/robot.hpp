#ifndef __REBOT_H__
#define __REBOT_H__
#pragma GCC optimize(3)

#include "config.hpp"
#include "grid.hpp"
struct Robot{
    int id; // 机器人的 id
    int x; // 机器人的 x 坐标
    int y; // 机器人的 y 坐标
    Robot() {
        this->id = -1;
        this->x = -1;
        this->y = -1;
    }
    Robot(int id, int x, int y) {
        this->id = id;
        this->x = x;
        this->y = y;
    }
    void outputTest() {
        TESTOUTPUT(fout << "Robot id: " << id << std::endl;)
    }
};

Robot *robots[MAX_Robot_Num];


#endif