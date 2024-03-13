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
    path->begin = begin;
    path->end = end;
    
    std::queue<Pos> q;
    std::unordered_map<Pos, Pos> pre;
    q.push(begin);
    pre[begin] = begin;
    while (!q.empty()) {
        Pos now = q.front();
        q.pop();
        if (now == end) break;
        for (int i = 0; i < 4; i++) {
            Pos next = now + dir[i];
            if (next.x < 0 || next.x >= MAX_Line_Length || next.y < 0 || next.y >= MAX_Col_Length) continue;
            if (pre.find(next) != pre.end() || (grids[next.x][next.y]->type != 0 && grids[next.x][next.y]->type != 3)) continue;
            q.push(next);
            pre[next] = now;
        }
    }
    // 可能无法到达 需要判断
    if (pre.find(end) == pre.end()) return nullptr;
    auto now = end;
    while (!(now == begin)) {
        path->path.push_back(now);
        now = pre[now];
    }
    std::reverse(path->path.begin(), path->path.end());
    return path;
}

#endif