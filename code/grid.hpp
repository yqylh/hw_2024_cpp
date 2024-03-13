#ifndef __GRID_H__
#define __GRID_H__

#include "config.hpp"
struct Grid {
    Pos pos; // 位置
    int type; // 0->空地 1->海洋 2->障碍物 3->泊位
    Grid(){}
    Grid(int x, int y, int type) : type(type){
        this->pos = Pos(x, y);
    }
};
struct Direction {
    int dir; // // 0 表示右移一格 1 表示左移一格 2 表示上移一格 3 表示下移一格, -1 表示不可达
    int length; // 从当前位置到目标位置的长度
    Direction(){ dir = -1;}
};
struct Path {
    Pos begin;
    Pos end;
    bool getOrPull; // 0 表示 get 1 表示 pull. 0 需要 reverse 1 不需要
    Direction **pathDir; // 用来导航从起点到终点的路径
    Path() {
        this->begin = Pos(-1, -1);
        this->end = Pos(-1, -1);
    }
    Path(Pos begin, Pos end, bool getOrPull) {
        this->begin = begin;
        this->end = end;
        this->getOrPull = getOrPull;
    }
};

Grid *grids[MAX_Line_Length][MAX_Col_Length];

// BFS用于寻路
std::list<Pos> *findPath(Pos begin, Pos end) {
    std::list<Pos> *path = new std::list<Pos>();
    
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
        path->push_back(now);
        now = pre[now];
    }
    std::reverse(path->begin(), path->end());
    return path;
}


Direction ** sovleGrid(Pos start) {
    Direction ** result = new Direction*[MAX_Line_Length];
    for (int i = 0; i < MAX_Line_Length; i++) result[i] = new Direction[MAX_Col_Length];
    result[start.x][start.y].length = 0;
    std::queue<Pos> q;
    std::unordered_map<Pos, Pos> pre;
    q.push(start); // 将起点加入队列
    pre[start] = start;
    while (!q.empty()) {
        Pos now = q.front();
        q.pop();
        for (int i = 0; i < 4; i++) {
            if (grids[now.x][now.y]->type == 0 || grids[now.x][now.y]->type == 3) {
                Pos next = now + dir[i]; // 下一个点
                if (next.x < 0 || next.x >= MAX_Line_Length || next.y < 0 || next.y >= MAX_Col_Length) continue; // 越界
                if ((grids[next.x][next.y]->type != 0 && grids[next.x][next.y]->type != 3) || pre.find(next) != pre.end()) continue; //不是空地或者泊位,或者已经记录过前序, 跳过
                result[next.x][next.y].length = result[now.x][now.y].length + 1;
                // 记录方向 但是要反过来记录方向 如果 i 是 0 或这 2 那么 next 的方向是 i + 1, 否则是 i - 1
                result[next.x][next.y].dir = ((i == 0 || i == 2) ? i + 1 : i - 1);
                q.push(next);
                pre[next] = now;
            }
        }
    }
    return result;
}
#endif