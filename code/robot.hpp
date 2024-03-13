#ifndef __REBOT_H__
#define __REBOT_H__
#pragma GCC optimize(3)

#include "config.hpp"
#include "grid.hpp"
#include "item.hpp"
#include "path.hpp"
#include "ship.hpp"
struct Robot{
    int id; // 机器人的 id
    Pos pos; // 机器人的位置
    int status; // 机器人的状态 0表示寄了 1 表示正常运行
    int bring; // 机器人带的货物 0 表示没有货物 1 表示有货物
    int bringTimeLimit; // 机器人到达带货物的时间
    Ship *toShip; // 机器人所在的船的 id
    Path *path; // 机器人的路径
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
    void move() {
        // 如果机器人被撞到了
        if (!status) return;
        // 如果没分配路径
        if (path == nullptr) return;
        // 如果路径走完了,但是因为一些原因没有到达目的地
        if (path->path.empty()) return;
        auto next = path->path.front();
        path->path.pop_front();
        auto nextPos = next - pos;
        auto nextDir = Pos2move[nextPos];
        printf("move %d %d\n", id , nextDir);
        TESTOUTPUT(fout << "move " << id << " " << nextDir << std::endl;)
        TESTOUTPUT(fout << "pos " << pos.x << " " << pos.y << std::endl;)
        TESTOUTPUT(fout << "next " << next.x << " " << next.y << std::endl;)
    }

    void action() {
        TESTOUTPUT(fout << "action " << id << std::endl;)
        // 如果机器人被撞到了
        if (!status) return;
        if (path != nullptr) {
            // 机器人有路径
            if (!path->path.empty()) return;
            // 机器人到达目的地
            if (bring == 0 && nowTime <= bringTimeLimit) {
                printf("get %d\n", id);
                TESTOUTPUT(fout << "get " << id << std::endl;)
                bring = 1;
            } else if (bring == 1 && grids[pos.x][pos.y]->type == 3) {
                printf("pull %d\n", id);
                TESTOUTPUT(fout << "pull " << id << std::endl;)
                bring = 0;
                if (toShip->berthId != -1) {
                    toShip->capacity++;
                    toShip->waitTime = nowTime + berths[toShip->berthId]->velocity;
                    TESTOUTPUT(fout << "toShip->capacity " << toShip->capacity << std::endl;)
                }
            }
            delete path;
            path = nullptr;
        }
        if (bring == 0) {
            // 机器人没有货物
            TESTOUTPUT(fout << "unsolvedItems.size() " << unsolvedItems.size() << std::endl;)
            for (auto i = unsolvedItems.begin(); i != unsolvedItems.end();) {
                if (i->checkDead()) {
                    unsolvedItems.erase(i++);
                    continue;
                }
                path = findPath(pos, i->pos);
                // error
                if (path == nullptr) {
                    return;
                }
                // 如果到哪里的时间超过了货物的时间
                if (path->path.size() + nowTime + 10> i->beginTime + Item_Continue_Time) {
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
                    path = findPath(pos, berths[ship->berthId]->pos + Pos(3, 3));
                    if (path == nullptr) {
                        continue;
                    }
                    toShip = ship;
                    TESTOUTPUT(fout << "toShip " << toShip->id << std::endl;)
                    break;
                }
            }
        }
    }
};

Robot *robots[MAX_Robot_Num];


#endif