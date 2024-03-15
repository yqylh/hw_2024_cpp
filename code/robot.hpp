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
    void checkCollision(std::unordered_set<Pos> &collisions);
};

void Robot::move() {
    // 如果机器人被撞到了
    if (!status) return;
    // 如果没分配路径
    if (path == nullptr) return;
    // 如果路径走完了,但是因为一些原因没有到达目的地
    if (pos == path->end) return;
    printf("move %d %d\n", id , path->pathDir->getDir(pos.x, pos.y));
    TEST(fout << "move " << id << " " << path->pathDir->getDir(pos.x, pos.y) << std::endl;)
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
                TEST(fout << "toShip " << toShip->id << std::endl;)
                break;
            }
        }
    }
}
void Robot::checkCollision(std::unordered_set<Pos> &collisions){
    TEST(fout << "checkCollision " << id << "status " << status << std::endl;)
    Pos nextTimePos;
    int nextDir = -1;
    // 首先预处理自己之后几帧的位置
    if (!status || path == nullptr || pos == path->end) {
        nextTimePos = pos;
    } else {
        nextDir = path->pathDir->getDir(pos.x, pos.y);
        nextTimePos = pos + dir[nextDir];
    }
    TEST(fout << "robot " << id << " pos " << pos.x << " " << pos.y << " nextTimePos " << nextTimePos.x << " " << nextTimePos.y << " nextDir " << nextDir << std::endl;)
    // 如果下一帧的位置有机器人 或者有机器人走到我的位置
    if (collisions.find(nextTimePos) != collisions.end() || collisions.find(pos) != collisions.end() ){
        TEST(fout << "crash " << id << std::endl;)
        // 不能继续走同样的方向,尽量不被追着揍
        std::vector<int> ableDir;
        if (nextDir == 0) ableDir = {2, 3, 1}; 
        if (nextDir == 1) ableDir = {2, 3, 0}; 
        if (nextDir == 2) ableDir = {0, 1, 3}; 
        if (nextDir == 3) ableDir = {0, 1, 2}; 
        for (auto & d : ableDir) { 
            auto nextPos = pos + dir[d];
            if (grids[nextPos.x][nextPos.y]->type == 1 || grids[nextPos.x][nextPos.y]->type == 2) continue;
            if (collisions.find(nextPos) == collisions.end()) {
                printf("move %d %d\n", id, d);
                TEST(fout << "move " << id << " " << d << std::endl;)
                status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
                collisions.insert(nextPos);
                return;
            }
        }
        // 如果可以停留在原地,有点弱智这里
        if (collisions.find(pos) == collisions.end()) {
            status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
            collisions.insert(pos);
            return;
        }
    } else {
        collisions.insert(nextTimePos);
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

#endif