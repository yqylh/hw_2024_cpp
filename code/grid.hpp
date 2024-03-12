#ifndef __GRID_H__
#define __GRID_H__

#include "config.hpp"

struct Grid {
    int x, y; // 坐标
    int type; // 0->空地 1->海洋 2->障碍物 3->泊位
    Grid(){}
    Grid(int x, int y, int type) : type(type){
        this->x = x;
        this->y = y;
    }
};
Grid *grids[MAX_Line_Length][MAX_Col_Length];


#endif