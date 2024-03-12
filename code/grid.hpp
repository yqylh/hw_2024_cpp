#ifndef __GRID_H__
#define __GRID_H__

#include "config.hpp"

struct Grid {
    int x, y; // 坐标
    int type; // 0->空地 1->障碍物 2->海洋 3->泊位
    Grid(){}
    Grid(int x, int y, int type) : type(type){
        this->x = x;
        this->y = y;
    }
};
Grid grids[MAP_Line_Length][MAP_Col_Length];


#endif