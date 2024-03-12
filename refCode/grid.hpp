#ifndef __GRID_H__
#define __GRID_H__

#include "vector.hpp"
#include <vector>
#include <map>

struct Grid {
    Vector2D index;
    int type; // 0->空地 1->障碍物
    std::pair<int, int> *visited; // robotId, time
    std::vector<Vector2D> obstacles; // 障碍物的四个坐标
    int foeTime = 0; // 被敌人占据的次数
    int foeEndTime = 0; // 被敌人占据的结束时间
    Grid(){
        visited = nullptr;
        obstacles = std::vector<Vector2D>();
    }
    Grid(Vector2D index, int type) : index(index), type(type){
        visited = nullptr;
        obstacles = std::vector<Vector2D>();
    }
    void solveEndTime(int nowTime) {
        if (nowTime - foeEndTime > 200) foeTime = 1;
        if (foeTime > 30) foeTime = 30;
        foeEndTime = nowTime;
        foeEndTime += (foeTime / 5 + 2) * 5;
        TESTOUTPUT(fout << "(" << index.x << "," << index.y << ") " << "foeEndTime: " << foeEndTime << std::endl;)
    }
    void setFoe(int noeTime) {
        foeTime++;
        type = 1;
        solveEndTime(noeTime);
    }
};
std::map<Vector2D, Grid *> grids;

/**
 * 预处理
 * 1. 增加边界
 * 2. 计算每个网格附近 3*3 的障碍物
*/
void detectionObstacle() {
    // 增加两行两列的边界
    for (double x = 0.25; x < 49.75; x += 0.5) {
        grids[Vector2D(x, -0.25)] = new Grid(Vector2D(x, -0.25), 1);
        grids[Vector2D(x, 50.25)] = new Grid(Vector2D(x, 50.25), 1);
        grids[Vector2D(-0.25, x)] = new Grid(Vector2D(-0.25, x), 1);
        grids[Vector2D(50.25, x)] = new Grid(Vector2D(50.25, x), 1);
    }
    grids[Vector2D(-0.25, -0.25)] = new Grid(Vector2D(-0.25, -0.25), 1);
    grids[Vector2D(-0.25, 50.25)] = new Grid(Vector2D(-0.25, 50.25), 1);
    grids[Vector2D(50.25, -0.25)] = new Grid(Vector2D(50.25, -0.25), 1);
    grids[Vector2D(50.25, 50.25)] = new Grid(Vector2D(50.25, 50.25), 1);
    // 对所有的网格计算 3*3 内的所有障碍物的四个坐标
    for (auto & i : grids) {
        double addx[] = {0, 0.5, -0.5};
        double addy[] = {0, 0.5, -0.5};
        std::set<int> visited;
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                Vector2D index = i.second->index + Vector2D(addx[j], addy[k]);
                if (grids.find(index) != grids.end()) {
                    if (grids[index]->type == 1) {
                        // 把浮点数坐标映射到整数区间
                        int numInt = round((index.x + 0.25) * 1000000) + round((index.y + 0.25)*100);
                        if (visited.find(numInt) == visited.end()) {
                            visited.insert(numInt);
                            i.second->obstacles.emplace_back(index.x + 0.25, index.y + 0.25);
                        }
                        numInt = round((index.x + 0.25) * 1000000) + round((index.y - 0.25)*100);
                        if (visited.find(numInt) == visited.end()) {
                            visited.insert(numInt);
                            i.second->obstacles.emplace_back(index.x + 0.25, index.y - 0.25);
                        }
                        numInt = round((index.x - 0.25) * 1000000) + round((index.y + 0.25)*100);
                        if (visited.find(numInt) == visited.end()) {
                            visited.insert(numInt);
                            i.second->obstacles.emplace_back(index.x - 0.25, index.y + 0.25);
                        }
                        numInt = round((index.x - 0.25) * 1000000) + round((index.y - 0.25)*100);
                        if (visited.find(numInt) == visited.end()) {
                            visited.insert(numInt);
                            i.second->obstacles.emplace_back(index.x - 0.25, index.y - 0.25);
                        }
                    }
                }
            }
        }
    }
}

#endif