#ifndef __GRID_H__
#define __GRID_H__

#include "config.hpp"
#include "path.hpp"
struct Grid {
    Pos pos; // 位置
    int type; // 0->空地 1->海洋 2->障碍物 3->泊位
    Grid(){}
    Grid(int x, int y, int type) : type(type){
        this->pos = Pos(x, y);
    }
};
Grid *grids[MAX_Line_Length][MAX_Col_Length];

// 用于寻路
Path* findPath(Pos begin, Pos end) {
    Path *path = new Path();

    
    return path;
}

#endif