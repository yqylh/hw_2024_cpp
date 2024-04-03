#ifndef __SHIP_H__
#define __SHIP_H__
#include "config.hpp"
#include "berth.hpp"

int tmpTotalGoods = 0;

struct Ship {
    int id;
    int capacity; // 货物数量
    Pos pos; // 船的核心点的位置
    int status; // 0 正常行驶状态 1恢复状态 2装载状态
    int direction; // 0 到 3 分别对应右、左、上、下。（和机器人移动的方向表示一致）
    
    int berthId; // 表示目标泊位，如果目标泊位是虚拟点，则为-1
    int isLastRound; // 是否是最后一轮
    Ship(int id): id(id) {
        status = 1;
        berthId = -1;
        capacity = 0;
        isLastRound = 0;
    }
    int leftCapacity() {
        return MAX_Capacity - capacity;
    }
    void go(){
        printf("go %d\n", id);
        status = 0;
        shipLogger.log(nowTime, "ship{0} go", id);
    }
    void go_berth(int berthId){
        printf("ship %d %d\n", id, berthId);
        status = 0;
        shipLogger.log(nowTime, "ship{0} move to berth{1} 移动时间 {2}", id, berthId, berths[berthId]->time);
    }
    void move_berth(int new_berthId){
        shipLogger.log(nowTime, "ship{0} move from berth{1} to berth{2} 移动时间 :500", id, berthId, new_berthId);
        status = 0;
        printf("ship %d %d\n", id, new_berthId);
    }
};

std::vector<Ship *> ships;
void newShip(int x, int y) {
    printf("lship %d %d\n", x, y);
    ships.push_back(new Ship(MAX_Ship_Num++));
}
#endif