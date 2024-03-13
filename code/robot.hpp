#ifndef __REBOT_H__
#define __REBOT_H__
#pragma GCC optimize(3)

#include "config.hpp"
#include "grid.hpp"
#include "item.hpp"
#include "ship.hpp"
#include "berth.hpp"

struct Collision {
    Pos pos;
    int time;
    Collision(Pos pos, int time) {
        this->pos = pos;
        this->time = time;
    }
    Collision(){}
};
struct Robot{
    int id; // 机器人的 id
    Pos pos; // 机器人的位置
    int status; // 机器人的状态 0表示寄了 1 表示正常运行
    int bring; // 机器人带的货物 0 表示没有货物 1 表示有货物
    int bringTimeLimit; // 机器人到达带货物的时间
    Ship *toShip; // 机器人所在的船的 id
    Path *path; // 机器人的路径
    Direction **robotDir; // 机器人到达每个点的方向
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
    void checkCollision(std::vector<Collision> collisions);
};

void Robot::move() {
    // 如果机器人被撞到了
    if (!status) return;
    // 如果没分配路径
    if (path == nullptr) return;
    // 如果路径走完了,但是因为一些原因没有到达目的地
    if (pos == path->end) return;

    if (path->getOrPull == 1) {
        printf("move %d %d\n", id , path->pathDir[pos.x][pos.y].dir);
    } else {
        // reverse 这是get的路径,从机器人初始点或者从港口去找货物的路径
        TEST(fout << "reverse" << id << std::endl;)
        // TEST(fout << "nowPos " << pos.x << " " << pos.y << std::endl;)
        // TEST(fout << "end " << path->end.x << " " << path->end.y << std::endl;)
        auto nowPos = path->end;
        // TEST(fout << "reverse nowPos " << nowPos.x << " " << nowPos.y << std::endl;)
        while (true) {
            auto nextDir = path->pathDir[nowPos.x][nowPos.y].dir;
            nowPos = nowPos + dir[nextDir];
            // TEST(fout << "reverse nowPos " << nowPos.x << " " << nowPos.y << std::endl;)
            if (nowPos == pos) {
                if (nextDir == 0 || nextDir == 2) nextDir++; else nextDir--;
                printf("move %d %d\n", id , nextDir);
                break;
            }
        }
        // TEST(fout << "reverse end" << std::endl;)
    }
}
void Robot::action() {
    TEST(fout << "action " << id << std::endl;)
    // 如果机器人被撞到了
    if (!status) return;
    if (path != nullptr) {
        if (!(pos == path->end)) return;
        // 机器人到达目的地
        if (bring == 0 && nowTime <= bringTimeLimit) {
            printf("get %d\n", id);
            TEST(fout << "get " << id << std::endl;)
            bring = 1;
        } else if (bring == 1 && grids[pos.x][pos.y]->type == 3) {
            printf("pull %d\n", id);
            TEST(fout << "pull " << id << std::endl;)
            bring = 0;
            if (toShip->berthId != -1) {
                toShip->capacity++;
                toShip->waitTime = nowTime + berths[toShip->berthId]->velocity;
                TEST(fout << "toShip->capacity " << toShip->capacity << std::endl;)
            }
        }
        delete path;
        path = nullptr;
    }
    if (bring == 0) {
        // 机器人没有货物
        TEST(fout << "unsolvedItems.size() " << unsolvedItems.size() << std::endl;)
        for (auto i = unsolvedItems.begin(); i != unsolvedItems.end();) {
            if (i->checkDead()) {
                unsolvedItems.erase(i++);
                continue;
            }
            path = new Path(pos, i->pos, 0);
            auto where = pos2berth.find(pos);
            if (where != pos2berth.end()) {
                auto berth = berths[where->second->id];
                if (berth->usePos[0] == pos) {
                    path->pathDir = berth->usePosDir[0];
                } else {
                    path->pathDir = berth->usePosDir[1];
                }
            } else {
                path->pathDir = robotDir;
            }
            // 如果到哪里的时间超过了货物的时间 或者 不可达
            if (path->pathDir[i->pos.x][i->pos.y].dir == -1 || path->pathDir[i->pos.x][i->pos.y].length + nowTime + 3 > i->beginTime + Item_Continue_Time) {
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
        TEST(fout << "begin" << std::endl;)
        // 机器人有货物
        for (auto & ship : ships) {
            if (ship->id != id % 5 && ship->id != shipNum) {
                continue;
            }
            if (ship->status == 1 && ship->berthId != -1 && ship->capacity != MAX_Capacity) {
                TEST(fout << berths[ship->berthId]->usePos.size() << std::endl;)
                path = new Path(pos, berths[ship->berthId]->usePos[ship->id % 2], 1);
                path->pathDir = berths[ship->berthId]->usePosDir[ship->id % 2];
                if (path->pathDir[pos.x][pos.y].dir == -1) {
                    delete path;
                    path = nullptr;
                    continue;
                }
                toShip = ship;
                TEST(fout << "toShip " << toShip->id << std::endl;)
                break;
            }
        }
        TEST(fout << "嗯对" << std::endl;)
    }
}
void Robot::checkCollision(std::vector<Collision> collisions){

}
Robot *robots[MAX_Robot_Num];

void solveRobot() {
    // 预处理每个机器人初始点到每个虚拟点的时间, 也就是机器人能到达的点
    for (int i = 0; i < MAX_Robot_Num; i++) {
        robots[i]->robotDir = sovleGrid(robots[i]->pos);
    }
}

#endif