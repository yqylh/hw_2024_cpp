#ifndef __REBOT_H__
#define __REBOT_H__
#pragma GCC optimize(3)

#include "config.hpp"
#include "grid.hpp"
#include "item.hpp"
#include "ship.hpp"
#include "berth.hpp"

struct Robot{
    int id; // 机器人的 id
    Pos pos; // 机器人的位置
    int status; // 机器人的状态 0表示寄了 1 表示正常运行
    int bring; // 机器人带的货物 0 表示没有货物 1 表示有货物
    int bringTimeLimit; // 机器人到达带货物的时间
    Ship *toShip; // 机器人所在的船的 id
    Path *path; // 机器人的路径
    Direction *robotDir; // 机器人到达每个点的方向
    Robot() {
        this->id = -1;
    }
    Robot(int id, int x, int y) {
        this->id = id;
        this->pos = Pos(x, y);
        this->status = 1;
        this->bring = 0;
        this->path = nullptr;
        this->toShip = nullptr;
    }
    void move();
    void action();
    void checkCollision(std::unordered_map<Pos, Pos> &otherPos);
};

void Robot::move() {
    // 如果机器人被撞到了
    if (!status) return;
    // 如果没分配路径
    if (path == nullptr) return;
    // 如果路径走完了,但是因为一些原因没有到达目的地
    if (pos == path->end) return;
    printf("move %d %d\n", id , path->pathDir->getDir(pos.x, pos.y));
    flowLogger.log(nowTime, "move {0} {1}", id, path->pathDir->getDir(pos.x, pos.y));
}
void Robot::action() {
    flowLogger.log(nowTime, "action {0}", id);
    // 如果机器人被撞到了
    if (!status) return;
    if (path != nullptr) {
        if (!(pos == path->end)) return;
        // 机器人到达目的地
        if (bring == 0 && nowTime <= bringTimeLimit) {
            printf("get %d\n", id);
            flowLogger.log(nowTime, "get {0}", id);
            bring = 1;
        } else if (bring == 1 && grids[pos.x][pos.y]->type == 3) {
            printf("pull %d\n", id);
            flowLogger.log(nowTime, "pull {0}", id);
            bring = 0;
            if (toShip->berthId != -1) {
                toShip->capacity++;
                toShip->waitTime = nowTime + berths[toShip->berthId]->velocity;
                robotLogger.log(nowTime, "toShip->capacity {0}", toShip->capacity);
            }
        }
        delete path;
        path = nullptr;
    }
    if (bring == 0) {
        // 机器人没有货物
        for (auto i = unsolvedItems.begin(); i != unsolvedItems.end();) {
            if (i->checkDead()) {
                unsolvedItems.erase(i++);
                continue;
            }
            path = new Path(pos, i->pos, 0);
            
            if (grids[i->pos.x][i->pos.y]->gridDir == nullptr) grids[i->pos.x][i->pos.y]->gridDir = sovleGrid(i->pos);
            path->pathDir = grids[i->pos.x][i->pos.y]->gridDir;
            // 如果到哪里的时间超过了货物的时间 或者 不可达
            auto length = (std::abs(pos.x - i->pos.x) + std::abs(pos.y - i->pos.y));
            if (path->pathDir->isVisited(pos.x, pos.y) == false || length + nowTime + 3 > i->beginTime + Item_Continue_Time) {
                delete path;
                path = nullptr;
                i++;
                continue;
            }
            bringTimeLimit = i->beginTime + Item_Continue_Time;
            unsolvedItems.erase(i++);
            break;
        }
    } 
    if (bring == 1) {
        // 机器人有货物
        for (auto & ship : ships) {
            if (ship->id != id % 5 && ship->id != shipNum) {
                continue;
            }
            if (ship->status == 1 && ship->berthId != -1 && ship->capacity != MAX_Capacity) {
                path = new Path(pos, berths[ship->berthId]->usePos[ship->id % 2], 1);
                path->pathDir = berths[ship->berthId]->usePosDir[ship->id % 2];
                if (path->pathDir->isVisited(pos.x, pos.y) == false) {
                    delete path;
                    path = nullptr;
                    continue;
                }
                toShip = ship;
                flowLogger.log(nowTime, "toShip {0}", toShip->id);
                break;
            }
        }
    }
}
// first 表示机器人的目标位置, second 表示机器人原始位置
void Robot::checkCollision(std::unordered_map<Pos, Pos> &otherPos){
    robotLogger.log(nowTime, "robot{0} checkCollision", id);
    Pos nextTimePos;
    int nextDir = -1;
    // 首先预处理自己之后几帧的位置
    if (!status || path == nullptr || pos == path->end) {
        nextTimePos = pos;
    } else {
        nextDir = path->pathDir->getDir(pos.x, pos.y);
        nextTimePos = pos + dir[nextDir];
    }
    robotLogger.log(nowTime, "robot{0} pos{1},{2} nextTimePos{3},{4} nextDir{5}", id, pos.x, pos.y, nextTimePos.x, nextTimePos.y, nextDir);
    // 如果下一帧的位置有机器人 或者有两个机器人交换位置
    if (otherPos.find(nextTimePos) != otherPos.end() || (otherPos.find(pos) != otherPos.end() && otherPos.find(pos)->second == nextTimePos) ){
        robotLogger.log(nowTime, "robot{0} crash", id);
        // 不能继续走同样的方向,尽量不被追着揍
        std::vector<int> ableDir;
        if (nextDir == 0) ableDir = {2, 3, 1}; 
        if (nextDir == 1) ableDir = {2, 3, 0}; 
        if (nextDir == 2) ableDir = {0, 1, 3}; 
        if (nextDir == 3) ableDir = {0, 1, 2}; 
        for (auto & d : ableDir) { 
            auto nextPos = pos + dir[d];
            if (grids[nextPos.x][nextPos.y]->type == 1 || grids[nextPos.x][nextPos.y]->type == 2) continue;
            if (otherPos.find(nextPos) != otherPos.end() 
                || (otherPos.find(pos) != otherPos.end() && otherPos.find(pos)->second == nextPos) ) {
                continue;
            }
            printf("move %d %d\n", id, d);
            flowLogger.log(nowTime, "move {0} {1}", id, d);
            status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
            otherPos[nextPos] = pos;
            return;
        }
        // 如果可以停留在原地,有点弱智这里??这里应该是直接可以留下来
        if (otherPos.find(pos) == otherPos.end()) {
            status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
            otherPos[pos] = pos;
            return;
        }
    } else {
        otherPos[nextTimePos] = pos;
    }
    return;
}
Robot *robots[MAX_Robot_Num];

void solveRobot() {
    // 预处理每个机器人初始点到每个虚拟点的时间, 也就是机器人能到达的点
    for (int i = 0; i < MAX_Robot_Num; i++) {
        robots[i]->robotDir = sovleGrid(robots[i]->pos);
        grids[robots[i]->pos.x][robots[i]->pos.y]->gridDir = robots[i]->robotDir;
    }
}

void solveCollision() {
    // 记录 friends
    std::vector<std::vector<int>> friends(MAX_Robot_Num + 1, std::vector<int>(0));
    std::unordered_set<Pos> robotPos;
    // 计算每个节点的 friend 的 id
    for (int i = 0; i <= robotNum; i++) {
        robotPos.insert(robots[i]->pos);
        for (int j = i + 1; j <= robotNum; j++) {
            if (robots[i]->pos.length(robots[j]->pos) <= 2) {
                friends[i].push_back(j);
                friends[j].push_back(i);
            }
        }
    }
    // 记录周围的空闲度
    int free[MAX_Robot_Num + 1] = {0};
    for (int i = 0; i <= robotNum; i++) {
        for (int j = 0; j < 4; j++) {
            auto nextPos = robots[i]->pos + dir[j];
            if (robotPos.find(nextPos) != robotPos.end()) continue;
            if (grids[nextPos.x][nextPos.y]->type == 1 || grids[nextPos.x][nextPos.y]->type == 2) continue;
            free[i]++;
        }
    }
    // 使用Lambda表达式作为比较函数
    auto compare = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        if (a.second == b.second) {
            int ida = a.first, idb = b.first;
            if (robots[ida]->pos.x == robots[idb]->pos.x) {
                return robots[ida]->pos.y < robots[idb]->pos.y;
            } else {
                return robots[ida]->pos.x < robots[idb]->pos.x;
            }
        } else {
            return a.second < b.second;
        }
    };
    // 初始化set，指定Lambda表达式作为比较器
    std::set<std::pair<int, int>, decltype(compare)> mySet(compare);
    // 按照 free 排序    id   free. 如果 free 相同按照 x y 排序
    for (int i = 0; i <= robotNum; i++) {
        mySet.insert(std::make_pair(i, free[i]));
    }
    /**
     * 1. 首先找到没计算的点中空闲度最小的点
     * 2. 然后找到它的邻居 优先距离 1,其次距离 2,然后将其删除
     * 3. 重复 1 2 步骤
    */
    std::unordered_map<int, std::vector<int>> group; group.clear();
    while (!mySet.empty()) {
        auto it = mySet.begin();
        int u = it->first;
        mySet.erase(it);
        group[u] = std::vector<int>();
        std::queue<int> q;
        while (!q.empty()) {
            auto top = q.front();
            q.pop();
            group[u].push_back(top);
            for (auto & item : mySet) if (item.first == top) {
                mySet.erase(item);
                break;
            }
            for (auto &frd : friends[top]) {
                if (robots[top]->pos.length(robots[frd]->pos) == 1) q.push(frd);
            }
            for (auto &frd : friends[top]) {
                if (robots[top]->pos.length(robots[frd]->pos) == 2) q.push(frd);
            }
        }
    }
    // 按组进行碰撞检测
    std::unordered_map<Pos, Pos> nextPos; nextPos.clear();
    for (auto & g : group) {
        for (auto & i : g.second) {
            robots[i]->checkCollision(nextPos);
        }
    }
    return;
}

#endif