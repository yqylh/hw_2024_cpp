#ifndef __GRID_H__
#define __GRID_H__

#include "config.hpp"
struct Direction {
    std::bitset<BitsetSize> visited; // 用于记录是否访问过
    std::bitset<BitsetSize * 2> dirNext; // 用于记录方向 00 表示右移一格 01 表示左移一格 10 表示上移一格 11 表示下移一格
    Direction(){
        visited.reset();
        dirNext.reset();
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
        if (d & 1) dirNext[index] = 1;
        if (d & 2) dirNext[index + 1] = 1;
    }
    int getDir(int x, int y) {
        int ans = 0;
        int index = getDirIndex(x, y);
        if (dirNext[index] == 1) ans += 1;
        if (dirNext[index + 1] == 1) ans += 2;
        return ans;
    }
};

struct TeDirection { //用于船只的四向导航
    int visited[4][BitsetSize]; // 用于记录是否访问过
    std::bitset<BitsetSize * 2> dirNext; // 用于记录方向 00 表示右移一格 01 表示左移一格 10 表示上移一格 11 表示下移一格
    TeDirection(){
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < BitsetSize; j++) {
                visited[i][j] = 0x3f3f3f3f;
            }
        }
        dirNext.reset();
    }
    int pos_to_dic(int pos) {
        if (pos == 0) return 0;
        if (pos == 1) return 1;
        if (pos == 10 or pos == 2) return 2;
        if (pos == 11 or pos == 3) return 3;
        return -1;
    }
    int getVisitedIndex(int x, int y) {
        return x * MAX_Col_Length + y;
    }
    int getDirIndex(int x, int y) {
        return x * MAX_Col_Length * 2 + y * 2;
    }
    void setVisited(int x, int y, int pos,int dis) {
        int dic = pos_to_dic(pos);
        visited[dic][getVisitedIndex(x,y)] = dis;
    }

    int isVisited(int x, int y,int pos) {
        int dic = pos_to_dic(pos);
        return visited[dic][getVisitedIndex(x, y)];
    }

    void setDir(int x, int y, int d) {
        int index = getDirIndex(x, y);
        if (d & 1) dirNext[index] = 1;
        if (d & 2) dirNext[index + 1] = 1;
    }
    int getDir(int x, int y) {
        int ans = 0;
        int index = getDirIndex(x, y);
        if (dirNext[index] == 1) ans += 1;
        if (dirNext[index + 1] == 1) ans += 2;
        return ans;
    }
};

class Grid {
public:
    Pos pos; // 位置
    int type;  
    /*
        -1  #保留(视作障碍物)
        0    #空地
        1    #海洋
        2    #障碍物
        3    #泊位
        4   #保留
        5    #陆地主干道
        6    #海洋主干道
        7    #机器人购买地块，同时该地块也是主干道
        8    #船舶购买地块，同时该地块也是主航道
        9    #靠泊区
        10   #海陆立体交通地块
        11   #海陆立体交通地块，同时为主干道和主航道
        12   #交货点
    */
   
    bool robotOnIt;
    
    Direction *gridDir; // 用来导航从起点到终点的路径
    Grid(){
        this->pos = Pos(-1, -1);
        this->type = -1;
        this->gridDir = nullptr;
        this->robotOnIt = false;
    }
    Grid(int x, int y, int type) : type(type){
        this->pos = Pos(x, y);
        this->gridDir = nullptr;
        this->robotOnIt = false;
    }
};

int gengerate_robot_type(int type);
int gengerate_boat_type(int type);

class RobotGrid:public Grid{
public:
    int type;
    /*
    -1  禁行
    0   通行
    1   主干道/主航道
    */
   Direction *gridDir_h, *gridDir_v;
   RobotGrid():Grid(){
        this->type = -1;
   }
   RobotGrid(int x, int y, int type):Grid(x,y,type){
        this->type = gengerate_robot_type(type);
   }
};

class BoatGrid:public Grid{
public:
    int type;
    int reduced_type[4]; //约化地图
    /*
    -1  禁行
    0   通行
    1   主干道/主航道
    */
   BoatGrid():Grid(){
        this->type = -1;
        this->reduced_type[0] = 0;
        this->reduced_type[1] = 0;
        this->reduced_type[2] = 0;
        this->reduced_type[3] = 0;
   }
   BoatGrid(int x, int y, int type):Grid(x,y,type){
        this->type = gengerate_boat_type(type);
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

// Grid *grids[MAX_Line_Length + 1][MAX_Col_Length + 1];
RobotGrid *robot_grids[MAX_Line_Length + 1][MAX_Col_Length + 1];
BoatGrid *boat_grids[MAX_Line_Length + 1][MAX_Col_Length + 1];

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
            if (pre.find(next) != pre.end() || (robot_grids[next.x][next.y]->type == -1)) continue;
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
Pos _arr[40010];
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
            if ((robot_grids[next.x][next.y]->type == -1) || result->isVisited(next.x, next.y)) continue; //不是空地或者泊位,或者已经记录过前序, 跳过
            result->setVisited(next.x, next.y);
            result->setDir(next.x, next.y, ((i == 0 || i == 2) ? i + 1 : i - 1));
            _arr[end++] = next;
        }
    }
    return result;
}

void solveAllGrid() {
    for (int i = 0; i < MAX_Line_Length; i++) {
        for (int j = 0; j < MAX_Col_Length; j++) {
            auto stop = high_resolution_clock::now();
            auto usedTime = duration_cast<milliseconds>(stop - programStart).count();
            if (usedTime > 3500) return;
            if ((robot_grids[i][j]->type != -1) && robot_grids[i][j]->gridDir == nullptr) {
                robot_grids[i][j]->gridDir = sovleGrid(Pos(i, j));
            }
        }
    }
    flowLogger.log(nowTime, "main thread Finish");
    return;
}

void solveBoatGrid(){
    /*
    boat shifts →
        0 (右): (0,0)(0,1)(0,2)(1,0)(1,1)(1,2)
        1 (左): (0,0)(-1,0)(-2,0)(0,-1)(-1,-1)(-2,-1)
        2 (上): (0,0)(0,1)(-1,0)(-1,1)(-2,0)(-2,1)
        3 (下): (0,0)(1,0)(2,0)(0,-1)(1,-1)(2,-1)
    */ 
   Pos shifts[4][6] = {
       {Pos(0,0),Pos(0,1),Pos(0,2),Pos(1,0),Pos(1,1),Pos(1,2)},
       {Pos(0,0),Pos(-1,0),Pos(-2,0),Pos(0,-1),Pos(-1,-1),Pos(-2,-1)},
       {Pos(0,0),Pos(0,1),Pos(-1,0),Pos(-1,1),Pos(-2,0),Pos(-2,1)},
       {Pos(0,0),Pos(1,0),Pos(2,0),Pos(0,-1),Pos(1,-1),Pos(2,-1)}};

    for (int i = 0; i < MAX_Line_Length; i++) {     //行
        for (int j = 0; j < MAX_Col_Length; j++) {  //列
        if (boat_grids[i][j]->type == -1) continue; //障碍物,跳过
            for (int k = 0; k < 4; k++) {           //方向
                for (int l = 0; l < 6; l++) {       //船占的六格
                    Pos next = Pos(i, j) + shifts[k][l];
                    if (next.x < 0 || next.x >= MAX_Line_Length || next.y < 0 || next.y >= MAX_Col_Length) continue;
                    if (boat_grids[next.x][next.y]->type == -1) {
                        boat_grids[i][j]->reduced_type[k] = -1; // 撞了
                        break;
                    }
                    if (boat_grids[i][j]->type == 1) {
                        boat_grids[i][j]->reduced_type[k] = 1;  // 在主航道
                        break;
                    }
                }
            }
        }
    }
}

int gengerate_robot_type(int type){
    switch(type){
        case 0: return 0;
        case 1: return -1;
        case 2: return -1;
        case 3: return 1;
        case 5: return 1;
        case 6: return -1;
        case 7: return 1;
        case 8: return -1;
        case 9: return -1;
        case 10: return 0;
        case 11: return 1;
        case 12: return -1;
    }
    return -1;
}

int gengerate_boat_type(int type){
        switch(type){
            case 0: return -1;
            case 1: return 0;
            case 2: return -1;
            case 3: return 1;
            case 5: return -1;
            case 6: return 1;
            case 7: return -1;
            case 8: return 1;
            case 9: return 1;
            case 10: return 0;
            case 11: return 1;
            case 12: return 1;
        }
    return -1;
}

#endif