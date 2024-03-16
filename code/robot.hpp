#ifndef __REBOT_H__
#define __REBOT_H__
#pragma GCC optimize(3)

#include "config.hpp"
#include "grid.hpp"
#include "item.hpp"
#include "ship.hpp"
#include "berth.hpp"
#include "berth_centre.hpp"
#include "path.hpp"

struct Robot{
    int id; // 机器人的 id
    Pos pos; // 机器人的位置
    int status; // 机器人的状态 0表示寄了 1 表示正常运行
    int bring; // 机器人带的货物 0 表示没有货物 1 表示有货物
    int bringTimeLimit; // 机器人到达带货物的时间
    int choosed_berth_id; // 机器人选择的泊位
    Direction *robotDir; // 机器人到达每个点的方向
    int lastWeak; // 上一次的弱智时间
    Pos lastWeakPos; // 上一次的弱智位置
    bool havePath;
    std::deque<Pos> wholePath; // 机器人的路径，front为当前位置
    Robot() {
        this->id = -1;
    }
    Robot(int id, int x, int y) {
        this->id = id;
        this->pos = Pos(x, y);
        this->status = 1;
        this->bring = 0;
        this->choosed_berth_id = -1;
        this->robotDir = nullptr;
        this->havePath = false;
        this->wholePath.clear();
        this->lastWeak = -1;
        this->lastWeakPos = Pos(-1, -1);
    }
    void action(); // 计算机器人的路径和生成行动
    void move(); // 生成机器人的移动指令
    void checkCollision(std::unordered_map<Pos, Pos> &otherPos);
};


void Robot::action() {
    flowLogger.log(nowTime, "action {0}", id);
    robotLogger.log(nowTime, "id={},status={},bring={},pos=({},{}),havePath={},pathSize={}", id, status, bring, pos.x, pos.y, havePath, wholePath.size());
    // 如果机器人被撞到了
    if (!status) {
        wholePath.clear();
        havePath = false;
        updateFixPos(pos, id);
        deletePathFromAllPath(id);
        return;
    }

    if (pos.x != wholePath.front().x || pos.y != wholePath.front().y) {
        // 如果机器人的路径首位不等于当前位置，则需要重新计算路径。
        havePath = false;
        wholePath.clear();
        deletePathFromAllPath(id);
    }

    // ==1 表示走到了，需要找新的路了
    if (wholePath.size() == 1) {
        robotLogger.log(nowTime, "reach id={},bring={},pos=({},{}),tarpos=({},{}),gridtype={}", id, bring, pos.x, pos.y, wholePath.front().x, wholePath.front().y, grids[wholePath.front().x][wholePath.front().y]->type);
        // 机器人到达目的地
        // 如果机器人没有货物，目的地应该是拿货物
        if (bring == 0 && nowTime <= bringTimeLimit) {
            printf("get %d\n", id);
            flowLogger.log(nowTime, "get {0}", id);
            bring = 1;
        // 送货物
        } else if (bring == 1 && grids[pos.x][pos.y]->type == 3) {
            printf("pull %d\n", id);
            flowLogger.log(nowTime, "pull {0}", id);
            bring = 0;
            if (choosed_berth_id != -1) {
                berth_center->declare_robot_pull_good(choosed_berth_id);
                // toShip->capacity++;
                // toShip->waitTime = nowTime + berths[toShip->berthId]->velocity;
                // robotLogger.log(nowTime, "toShip->capacity {0}", toShip->capacity);
            }
        }

        wholePath.clear();
        havePath = false;
    }


    // 没走到，还在走
    if (wholePath.size() > 1) return;

    // 找当前机器人在不碰撞前提下的所有路，路长存在disWithTime，前序点存在preWithTime
    auto beginSolveTime = high_resolution_clock::now();
    solveGridWithTime(pos, id);
    auto endSolveTime = high_resolution_clock::now();
    // pathLogger.log(nowTime, "rId={0}solveGridWithTime time={1}", id, duration_cast<microseconds>(endSolveTime - beginSolveTime).count());

    if (bring == 0) {
        // 机器人没有货物，找一个最近的可达的货物
        double value = 0;
        int minDis = 0x3f3f3f3f;
        auto targetItem = unsolvedItems.end();

        for (auto i = unsolvedItems.begin(); i != unsolvedItems.end();) {
            if (i->checkDead()) {
                unsolvedItems.erase(i++);
                continue;
            }

            // 判断是否可达
            if (disWithTime[i->pos.x][i->pos.y] == 0x3f3f3f3f) {
                i++;
                continue;
            }
            // 判断是否超时
            if (nowTime + disWithTime[i->pos.x][i->pos.y] + 3 > i->beginTime + Item_Continue_Time) {
                i++;
                continue;
            }
            auto tempValue = double(i->value) / disWithTime[i->pos.x][i->pos.y];
            if (tempValue > value){
                minDis = disWithTime[i->pos.x][i->pos.y];
                targetItem = i;
                value = tempValue;
            }
            i++;
        }


        if (minDis != 0x3f3f3f3f && targetItem != unsolvedItems.end()) {
            int targetItemIndex = std::distance(unsolvedItems.begin(), targetItem);
            flowLogger.log(nowTime, "rid={},toItem={}", id, targetItemIndex);

            wholePath = findPathWithTime(pos, targetItem->pos);
            bringTimeLimit = targetItem->beginTime + Item_Continue_Time;
            unsolvedItems.erase(targetItem);
            havePath = true;
            addPathToAllPath(wholePath, id);
        }
    } 
    if (bring == 1) {
        // 机器人有货物
        int minDis = 0x3f3f3f3f;
        Pos choosed_berth_pos = Pos(-1,-1);

        float* berth_level_list = berth_center->call_robot_choose_berth();

        for (int i = 0; i < MAX_Berth_Num;i++){
            Berth* now_berth = berths[i];
            for(Pos sub_berths : now_berth->usePos){
                berthLogger.log(nowTime, "rid={},bid={},sub_berths=({},{}),berth_level_list={},disWithTime={}", id, i, sub_berths.x, sub_berths.y, berth_level_list[i], disWithTime[sub_berths.x][sub_berths.y]);
                if(disWithTime[sub_berths.x][sub_berths.y] == 0x3f3f3f3f) continue;
                
                if (disWithTime[sub_berths.x][sub_berths.y] + berth_level_list[i]< minDis){
                    minDis = disWithTime[sub_berths.x][sub_berths.y];
                    choosed_berth_pos = sub_berths;
                    choosed_berth_id = i;
                }   
            }
        }
        berthLogger.log(nowTime, "find berth, rid={},choosed_berth_pos=({},{}),minDis={}", id, choosed_berth_pos.x, choosed_berth_pos.y, minDis);

        // auto targetShip = ships[0];

        // for (auto & ship : ships) {
        //     if (ship->id != id % 5 && ship->id != shipNum) {
        //         continue;
        //     }
        //     if (ship->status == 1 && ship->berthId != -1 && ship->capacity != MAX_Capacity) {
        //         // 机器人有货物，找一个最近的可达的泊位
        //         // Pos nowShipPos = berths[ship->berthId]->usePos[ship->id % 2];
                
        //         // 判断是否可达
        //         if (disWithTime[nowShipPos.x][nowShipPos.y] == 0x3f3f3f3f) {
        //             continue;
        //         }
        //         /*
        //         // 判断是否超时
        //         if (nowTime + disWithTime[nowShipPos.x][nowShipPos.y] + 3 > ship->waitTime) {
        //             continue;
        //         }
        //         */
        //         if (disWithTime[nowShipPos.x][nowShipPos.y] < minDis) {
        //             minDis = disWithTime[nowShipPos.x][nowShipPos.y];
        //             targetShip = ship;
        //         }

        //     }
        // }
        
        if (minDis != 0x3f3f3f3f) {
            // toShip = targetShip;
            
            // auto targetPos = berths[targetShip->berthId]->usePos[targetShip->id % 2];
            // flowLogger.log(nowTime, "rid={},toShip={},targetPos=({},{}),ship->berthId={}", id, toShip->id, choosed_berth_pos.x, choosed_berth_pos.y, toShip->berthId);
            wholePath = findPathWithTime(pos, choosed_berth_pos);
            havePath = true;
            berth_center->declare_robot_choose_berth(choosed_berth_id);
            addPathToAllPath(wholePath, id);
        }
    }

    if (wholePath.size() == 0) {
        robotLogger.log(nowTime, "noPath id={},bring={},havePath={},pathSize={}", id, bring, havePath, wholePath.size());
        havePath = false;
        updateFixPos(pos, id);
    }
}

void Robot::move() {
    // 如果机器人被撞到了
    if (!status) return;
    // 只要路径小于2，就说明无论如何不用走
    if (wholePath.size() < 2) return;
    // robotLogger.log(nowTime, "id={},bring={},havePath={},pathSize={},from=({},{}),to=({},{}),status={}", id, bring, havePath, wholePath.size(), wholePath.front().x, wholePath.front().y, wholePath.back().x, wholePath.back().y, status);
    auto nowPos = wholePath.front();
    auto nextPos = wholePath.at(1);
    int nextDir = getDirWithPath(nowPos, nextPos);
    // 把当前位置弹出
    wholePath.pop_front();
    printf("move %d %d\n", id , nextDir);

    flowLogger.log(nowTime, "move {0} {1}", id, nextDir);
}


// first 表示机器人的目标位置, second 表示机器人原始位置
void Robot::checkCollision(std::unordered_map<Pos, Pos> &otherPos){
    // Pos nextTimePos;
    // int nextDir = -1;
    // // 首先预处理自己之后几帧的位置
    // if (!status || path == nullptr || pos == path->end) {
    //     nextTimePos = pos;
    // } else {
    //     nextDir = path->pathDir->getDir(pos.x, pos.y);
    //     nextTimePos = pos + dir[nextDir];
    // }
    // robotLogger.log(nowTime, "robot{0} pos{1},{2} nextTimePos{3},{4} nextDir{5}", id, pos.x, pos.y, nextTimePos.x, nextTimePos.y, nextDir);
    // // 如果下一帧的位置有机器人 或者有两个机器人交换位置
    // if (otherPos.find(nextTimePos) != otherPos.end() || (otherPos.find(pos) != otherPos.end() && otherPos.find(pos)->second == nextTimePos) ){
    //     robotLogger.log(nowTime, "robot{0} crash", id);
    //     // 不能继续走同样的方向,尽量不被追着揍
    //     std::vector<int> ableDir;
    //     if (nextDir == 0) ableDir = {2, 3, 1}; 
    //     if (nextDir == 1) ableDir = {2, 3, 0}; 
    //     if (nextDir == 2) ableDir = {0, 1, 3}; 
    //     if (nextDir == 3) ableDir = {0, 1, 2}; 
    //     if (nextDir == -1) ableDir = {0, 1, 2, 3};
    //     while (rand() % 2) std::swap(ableDir[rand() % ableDir.size()], ableDir[rand() % ableDir.size()]);
    //     if (otherPos.find(nextTimePos) != otherPos.end() && otherPos.find(nextTimePos)->first == otherPos.find(nextTimePos)->second) {
    //         // 这是一个弱智的情况,有机器人停下来了,那我需要下一帧继续绕开走
    //         lastWeak = nowTime;
    //         lastWeakPos = pos;
    //     }
    //     for (auto & d : ableDir) { 
    //         auto nextPos = pos + dir[d];
    //         if (nextPos.x < 0 || nextPos.x >= MAX_Line_Length || nextPos.y < 0 || nextPos.y >= MAX_Col_Length) {
    //             continue;
    //         }
    //         if (grids[nextPos.x][nextPos.y]->type == 1 || grids[nextPos.x][nextPos.y]->type == 2) continue;
    //         if (otherPos.find(nextPos) != otherPos.end() 
    //             || (otherPos.find(pos) != otherPos.end() && otherPos.find(pos)->second == nextPos) ) {
    //             continue;
    //         }
    //         printf("move %d %d\n", id, d);
    //         flowLogger.log(nowTime, "move {0} {1}", id, d);
    //         status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
    //         otherPos[nextPos] = pos;
    //         return;
    //     }
    //     // 如果可以停留在原地,有点弱智这里??这里应该是直接可以留下来
    //     if (otherPos.find(pos) == otherPos.end()) {
    //         status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
    //         otherPos[pos] = pos;
    //         return;
    //     }
    // } else { // 如果下一帧的位置没有机器人
    //     // 特殊情况:如果上一帧遇到了弱智,而且这一帧的下一步是回到弱智面前,就不要走了,换一个方向.
    //     if (!status || path == nullptr || pos == path->end) {}
    //     else if (lastWeak == nowTime - 1 && pos+dir[path->pathDir->getDir(pos.x, pos.y)] == lastWeakPos) {
    //         auto nextDir = path->pathDir->getDir(pos.x, pos.y);
    //         int minLength = 1e9; int minDir = -1;
    //         // 遍历其他三个方向,选择一个最近的方向
    //         for (int d = 0; d < 4; d++) if (d != nextDir) {
    //             auto nextPos = pos + dir[d];
    //             auto length = path->end.length(nextPos);
    //             // 判断越界
    //             if (nextPos.x < 0 || nextPos.x >= MAX_Line_Length || nextPos.y < 0 || nextPos.y >= MAX_Col_Length) continue;
    //             // 判断是否是障碍物
    //             if (grids[nextPos.x][nextPos.y]->type == 1 || grids[nextPos.x][nextPos.y]->type == 2) continue;
    //             // 判断是否有机器人
    //             if (otherPos.find(nextPos) != otherPos.end()) continue;
    //             // 判断是否重叠
    //             if (otherPos.find(pos) != otherPos.end() && otherPos.find(pos)->second == nextPos) continue;
    //             if (length < minLength) {
    //                 minLength = length;
    //                 minDir = d;
    //             } else if (length == minLength) {
    //                 if (rand() % 2 == 0) minDir = d;
    //             }
    //         }
    //         if (minDir != -1) {
    //             printf("move %d %d\n", id, minDir);
    //             status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
    //             otherPos[pos + dir[minDir]] = pos;
    //             flowLogger.log(nowTime, "move {0} {1}", id, minDir);
    //             return;
    //         }
    //     }
    //     otherPos[nextTimePos] = pos;
    // }
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
            if (nextPos.x < 0 || nextPos.x >= MAX_Line_Length || nextPos.y < 0 || nextPos.y >= MAX_Col_Length) {
                continue;
            }
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
    bool visited[MAX_Robot_Num + 1] = {0};
    while (!mySet.empty()) {
        auto it = mySet.begin();
        int u = it->first;
        mySet.erase(it);
        group[u] = std::vector<int>();
        std::queue<int> q;
        q.push(u);
        visited[u] = true;
        while (!q.empty()) {
            auto top = q.front();
            q.pop();
            group[u].push_back(top);
            for (auto & item : mySet) if (item.first == top) {
                mySet.erase(item);
                break;
            }
            Pos topPos = robots[top]->pos;
            for (auto &frd : friends[top]) {
                if (visited[frd] == false && topPos.length(robots[frd]->pos) == 1) {
                    q.push(frd);
                    visited[frd] = true;
                }
            }
            for (auto &frd : friends[top]) {
                if (visited[frd] == false && topPos.length(robots[frd]->pos) == 2) {
                    q.push(frd);
                    visited[frd] = true;
                }
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