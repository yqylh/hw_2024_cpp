#ifndef __REBOT_H__
#define __REBOT_H__
#pragma GCC optimize(3)

#include "config.hpp"
#include "grid.hpp"
#include "item.hpp"
#include "path.hpp"
struct Robot{
    int id; // 机器人的 id
    Pos pos; // 机器人的位置
    int status; // 机器人的状态
    int bring; // 机器人带的货物 0 表示没有货物 1 表示有货物
    Robot() {
        this->id = -1;
    }
    Robot(int id, int x, int y) {
        this->id = id;
        this->pos = Pos(x, y);
    }
    void outputTest() {
        TESTOUTPUT(fout << "Robot id: " << id << std::endl;)
    }
    void move() {
        printf("move %d %d\n", id , 1);
    }
    void action() {
        
    }
    void load() {}
};

Robot *robots[MAX_Robot_Num];


#endif