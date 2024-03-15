#ifndef __REBOT_H__
#define __REBOT_H__
#pragma GCC optimize(3)

#include "config.hpp"
#include "grid.hpp"
#include "item.hpp"
#include "ship.hpp"
#include "berth.hpp"
#include "path.hpp"

struct Robot{
    int id; // 机器人的 id
    Pos pos; // 机器人的位置
    int status; // 机器人的状态 0表示寄了 1 表示正常运行
    int bring; // 机器人带的货物 0 表示没有货物 1 表示有货物
    int bringTimeLimit; // 机器人到达带货物的时间
    Ship *toShip; // 机器人所在的船的 id
    Direction *robotDir; // 机器人到达每个点的方向
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
        this->toShip = nullptr;
        this->robotDir = nullptr;
        this->havePath = false;
        this->wholePath.clear();
    }
    void action(); // 计算机器人的路径和生成行动
    void move(); // 生成机器人的移动指令
    void checkCollision(std::unordered_map<Pos, Pos> &otherPos);
};

void Robot::move() {
    // 如果机器人被撞到了
    if (!status) return;
    // 只要路径小于2，就说明无论如何不用走
    if (wholePath.size() < 2) return;
    robotLogger.log(nowTime, "id={},bring={},havePath={},pathSize={},from=({},{}),to=({},{}),status={}", id, bring, havePath, wholePath.size(), wholePath.front().x, wholePath.front().y, wholePath.back().x, wholePath.back().y, status);
    auto nowPos = wholePath.front();
    auto nextPos = wholePath.at(1);
    int nextDir = getDirWithPath(nowPos, nextPos);
    // 把当前位置弹出
    wholePath.pop_front();
    printf("move %d %d\n", id , nextDir);

    flowLogger.log(nowTime, "move {0} {1}", id, nextDir);
}

void Robot::action() {
    flowLogger.log(nowTime, "action {0}", id);
    robotLogger.log(nowTime, "ac id={},bring={},havePath={},pathSize={}", id, bring, havePath, wholePath.size());

    // 如果机器人被撞到了
    if (!status) return;    
    // 没走到，还在走
    if (wholePath.size() > 1) return;

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
            if (toShip->berthId != -1) {
                toShip->capacity++;
                toShip->waitTime = nowTime + berths[toShip->berthId]->velocity;
                robotLogger.log(nowTime, "toShip->capacity {0}", toShip->capacity);
            }
        }

        wholePath.clear();
        havePath = false;
    }
    // 机器人现在应该是没事干
    // 找当前机器人在不碰撞前提下的所有路，路长存在disWithTime，前序点存在preWithTime
    auto beginSolveTime = high_resolution_clock::now();
    solveGridWithTime(pos);
    auto endSolveTime = high_resolution_clock::now();
    pathLogger.log(nowTime, "rId={0}solveGridWithTime time={1}", id, duration_cast<microseconds>(endSolveTime - beginSolveTime).count());

    if (bring == 0) {
        // 机器人没有货物，找一个最近的可达的货物
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
            if (disWithTime[i->pos.x][i->pos.y] < minDis) {
                minDis = disWithTime[i->pos.x][i->pos.y];
                targetItem = i;
            }
            i++;

            /*
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
            */
        }


        if (minDis != 0x3f3f3f3f && targetItem != unsolvedItems.end()) {
            int targetItemIndex = std::distance(unsolvedItems.begin(), targetItem);
            flowLogger.log(nowTime, "toItem {0}", targetItemIndex);

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
        auto targetShip = ships[0];

        for (auto & ship : ships) {
            if (ship->id != id % 5 && ship->id != shipNum) {
                continue;
            }
            if (ship->status == 1 && ship->berthId != -1 && ship->capacity != MAX_Capacity) {
                // 机器人有货物，找一个最近的可达的泊位
                Pos nowShipPos = berths[ship->berthId]->usePos[ship->id % 2];
                // 判断是否可达
                if (disWithTime[nowShipPos.x][nowShipPos.y] == 0x3f3f3f3f) {
                    continue;
                }
                /*
                // 判断是否超时
                if (nowTime + disWithTime[nowShipPos.x][nowShipPos.y] + 3 > ship->waitTime) {
                    continue;
                }
                */
                if (disWithTime[nowShipPos.x][nowShipPos.y] < minDis) {
                    minDis = disWithTime[nowShipPos.x][nowShipPos.y];
                    targetShip = ship;
                }

                /*
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
                */
            }
        }
        
        if (minDis != 0x3f3f3f3f) {
            toShip = targetShip;
            
            auto targetPos = berths[targetShip->berthId]->usePos[targetShip->id % 2];
            flowLogger.log(nowTime, "rid={},toShip={},targetPos=({},{}),ship->berthId={}", id, toShip->id, targetPos.x, targetPos.y, toShip->berthId);
            wholePath = findPathWithTime(pos, targetPos);
            havePath = true;
            addPathToAllPath(wholePath, id);
        }
    }
}
// first 表示机器人的目标位置, second 表示机器人原始位置
void Robot::checkCollision(std::unordered_map<Pos, Pos> &otherPos){
}
Robot *robots[MAX_Robot_Num];

void solveRobot() {
    // 预处理每个机器人初始点到每个虚拟点的时间, 也就是机器人能到达的点
    for (int i = 0; i < MAX_Robot_Num; i++) {
        robots[i]->robotDir = sovleGrid(robots[i]->pos);
        grids[robots[i]->pos.x][robots[i]->pos.y]->gridDir = robots[i]->robotDir;
    }
}

#endif