#ifndef __GRID_H__
#define __GRID_H__

#include "config.hpp"

struct Grid {
    int x, y; // 坐标
    Vector2D index;
    int type; // 0->空地 1->障碍物
    std::pair<int, int> *visited; // robotId, time
    Grid(){
        visited = nullptr;
        obstacles = std::vector<Vector2D>();
    }
    Grid(Vector2D index, int type) : index(index), type(type){
        visited = nullptr;
        obstacles = std::vector<Vector2D>();
    }
};
Grid grids[MAP_Line_Length][MAP_Col_Length];


#endif