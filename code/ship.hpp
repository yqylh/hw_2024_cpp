#ifndef __SHIP_H__
#define __SHIP_H__
#include "config.hpp"
#include "berth.hpp"
#include <chrono>

int tmpTotalGoods = 0;

struct Ship {
    int id;
    int capacity; // 货物数量
    Pos pos; // 船的核心点的位置(左后方)
    int status; // 0 正常行驶状态 1恢复状态 2装载状态
    int direction; // 船的朝向 0 到 3 分别对应右、左、上、下。（和机器人移动的方向表示一致）
    /**
     * 船所在的泊位 -1表示销售点 -2表示船刚卖完,或者刚出生 没有目标
     * 如果 status 是 2 则表示船所在的泊位. 如果 status 是 1 则表示船即将到达的泊位(正在恢复状态) 如果 status 是 0 则表示船想去的泊位.
    */
    int berthId;
    std::deque<int> *path; // 船的路径, 0 表示顺时针, 1 表示逆时针 2 表示直线
    int isLastRound; // 是否是最后一轮
    Pos targetPos; // 目标位置
    Ship(int id): id(id) {
        status = 1;
        capacity = 0;
        isLastRound = 0;
        berthId = -2;
        path = nullptr;

    }
    void output() {
        shipLogger.log(nowTime, "ship{} pos{},{} capacity{} status{} direction{} berthId{}  targetPos{},{}", id, pos.x, pos.y, capacity, status, direction, berthId, targetPos.x, targetPos.y);
    }
    inline int leftCapacity() {return MAX_Capacity - capacity;}
    // 靠泊命令
    void berth() {
        printf("berth %d\n", id);
        flowLogger.log(nowTime, "ship{} berth", id);
        status = 1;
    }
    // 离开命令
    void dept() {
        printf("dept %d\n", id);
        flowLogger.log(nowTime, "ship{} dept", id);
        status = 1;
    }
    // 去卖
    void goSell(Pos delivery) {
        dept();
        this->berthId = -1;
        // 这里不需要找路径,因为要先移动到主航道上,然后状态从 2 变成 0. path 设置成空
        path = nullptr;
        targetPos = delivery;
    }
    // 去另一个港口
    void moveToBerth(int _berthId, Pos to) {
        if (berthId >= 0) dept();
        this->berthId = _berthId;
        // 这里不需要找路径,因为要先移动到主航道上,然后状态从 2 变成 0. path 设置成空
        path = nullptr;
        targetPos = to;
    }
    void move() {
        if (status != 0) return;
        if (targetPos == pos) return;
        // 没有到达目标位置 如果路径为空,则找路径
        if (path == nullptr) {
            path = sovleShip( pos, direction, targetPos, true);
            // path = Timer::measure("sovleShip_ori",sovleShip_ori, pos, direction, targetPos, true);
        }
        if (path->empty()) {
            path = nullptr;
            return;
        }
        int nextDir = path->front(); path->pop_front();
        if (nextDir ==4) {
            nextDir = path->front();path->pop_front();
            }

        if (nextDir == 2) {
            printf("ship %d\n", id);
            flowLogger.log(nowTime, "ship{} move", id);
        } else{ 
            printf("rot %d %d\n", id, nextDir);
            flowLogger.log(nowTime, "ship{} rot{}", id, nextDir);
        }
        
    }
};

std::vector<Ship *> ships;
void newShip(int x, int y) {
    printf("lboat %d %d\n", x, y);
    ships.push_back(new Ship(MAX_Ship_Num++));
    money -= 8000;
}


#endif