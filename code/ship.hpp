#ifndef __SHIP_H__
#define __SHIP_H__
#include "config.hpp"
#include "berth.hpp"

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
    Direction *path; // 船的方希
    int isLastRound; // 是否是最后一轮
    Pos targetPos; // 目标位置
    Ship(int id): id(id) {
        status = 1;
        capacity = 0;
        isLastRound = 0;
        berthId = -2;
        path = nullptr;

    }
    inline int leftCapacity() {return MAX_Capacity - capacity;}
    // 靠泊命令
    void berth() {
        printf("berth %d\n", id);
        status = 1;
    }
    // 离开命令
    void dept() {
        printf("dept %d\n", id);
        status = 1;
    }
    // 去卖
    void goSell(Pos delivery) {
        dept();
        this->berthId = -1;
        // find path
        path = sovleShip(delivery);
        targetPos = delivery;
    }
    // 去另一个港口
    void moveToBerth(int _berthId, Pos to) {
        if (berthId != -2) dept();
        this->berthId = _berthId;
        // find path
        path = sovleShip(to);
        targetPos = to;
    }
    void move() {
        if (status != 0) return;
        if (path == nullptr) return;
        if (targetPos == pos) return;
        quickFix(1);
        int nextDir = path->getDir(pos.x, pos.y);
        if (nextDir != direction) {
            printf("rot %d %d\n", id, 1);
            return;
        } else {
            printf("ship %d\n", id);
        }
    }
};

std::vector<Ship *> ships;
void newShip(int x, int y) {
    shipLogger.log(nowTime, "新建船只！");
    printf("lboat %d %d\n", x, y);
    ships.push_back(new Ship(MAX_Ship_Num++));
}
#endif