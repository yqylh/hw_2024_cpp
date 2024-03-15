#ifndef __GRID_H__
#define __GRID_H__

#include "config.hpp"
struct Direction {
    std::bitset<BitsetSize> visited; // 用于记录是否访问过
    std::bitset<BitsetSize * 2> dir; // 用于记录方向 00 表示右移一格 01 表示左移一格 10 表示上移一格 11 表示下移一格
    Direction(){
        visited.reset();
        dir.reset();
    }
    int getVisitedIndex(int x, int y) {
        return x * MAX_Col_Length + y;
    }
    int getDirIndex(int x, int y) {
        return x * MAX_Col_Length * 2 + y * 2;
    }
    void setVisited(int x, int y) {
        visited[getVisitedIndex(x, y)] = 1;
    }
    bool isVisited(int x, int y) {
        return visited[getVisitedIndex(x, y)];
    }
    void setDir(int x, int y, int d) {
        int index = getDirIndex(x, y);
        if (d & 1) dir[index] = 1;
        if (d & 2) dir[index + 1] = 1;
    }
    int getDir(int x, int y) {
        int ans = 0;
        int index = getDirIndex(x, y);
        if (dir[index] == 1) ans += 1;
        if (dir[index + 1] == 1) ans += 2;
        return ans;
    }
};

struct Grid {
    Pos pos; // 位置
    int type; // 0->空地 1->海洋 2->障碍物 3->泊位
    Direction *gridDir; // 用来导航从起点到终点的路径
    Grid(){
        this->pos = Pos(-1, -1);
        this->type = -1;
        this->gridDir = nullptr;
    }
    Grid(int x, int y, int type) : type(type){
        this->pos = Pos(x, y);
        this->gridDir = nullptr;
    }
};

struct Path {
    Pos begin;
    Pos end;
    bool getOrPull; // 0 表示 get 1 表示 pull. 0 需要 reverse 1 不需要
    Direction *pathDir; // 用来导航从起点到终点的路径
    Path() {
        this->begin = Pos(-1, -1);
        this->end = Pos(-1, -1);
    }
    Path(Pos begin, Pos end, bool getOrPull) {
        this->begin = begin;
        this->end = end;
        this->getOrPull = getOrPull;
    }
    ~Path() {}
};

Grid *grids[MAX_Line_Length + 1][MAX_Col_Length + 1];

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

Direction * sovleGrid(Pos origin) {
    Direction * result = new Direction;
    result->setVisited(origin.x, origin.y);
    Pos _arr[40010];
    int start = 0, end = 0;
    _arr[end++] = origin;
    while (start < end) {
        Pos &now = _arr[start++];
        for (int i = 0; i < 4; i++) {
            Pos next = now + dir[i]; // 下一个点
            if (next.x < 0 || next.x >= MAX_Line_Length || next.y < 0 || next.y >= MAX_Col_Length) continue; // 越界
            if ((grids[next.x][next.y]->type != 0 && grids[next.x][next.y]->type != 3) || result->isVisited(next.x, next.y)) continue; //不是空地或者泊位,或者已经记录过前序, 跳过
            result->setVisited(next.x, next.y);
            result->setDir(next.x, next.y, ((i == 0 || i == 2) ? i + 1 : i - 1));
            _arr[end++] = next;
        }
    }
    return result;
}

void solveAllGrid() {
    // begin time
    std::atomic<int> threadLineMain = 0;
    std::atomic<int> threadLineT1 = 199;
    // 创建一个新的线程 thread
    std::thread t1([&](){
        for (int i = MAX_Line_Length - 1; i >= 0; i--) {
            threadLineT1 = i;
            if (i == threadLineMain) break;
            for (int j = 0; j < MAX_Col_Length; j++) {
                if ((grids[i][j]->type == 0 || grids[i][j]->type == 3) && grids[i][j]->gridDir == nullptr) {
                    grids[i][j]->gridDir = sovleGrid(Pos(i, j));
                }
            }
        }
        flowLogger.log(nowTime, "thread Finish");
        threadFinish = true;
    });
    // 这个线程要后台运行
    t1.detach();
    for (int i = 0; i < MAX_Line_Length; i++) {
        threadLineMain = i;
        if (i == threadLineT1) break;
        for (int j = 0; j < MAX_Col_Length; j++) {
            auto stop = high_resolution_clock::now();
            auto usedTime = duration_cast<milliseconds>(stop - programStart).count();
            if (usedTime > 3500) return;
            if ((grids[i][j]->type == 0 || grids[i][j]->type == 3) && grids[i][j]->gridDir == nullptr) {
                grids[i][j]->gridDir = sovleGrid(Pos(i, j));
            }
        }
    }
    flowLogger.log(nowTime, "main thread Finish");
    return;
}

#endif