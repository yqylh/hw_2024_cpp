#ifndef __SHIP_H__
#define __SHIP_H__
#include "config.hpp"
#include "berth.hpp"
struct Ship {
    int id;
    int status; // 0 移动(运输)中 1正常状态(即装货状态或运输完成状态) 2 泊位外等待状态
    int berthId; // 表示目标泊位，如果目标泊位是虚拟点，则为-1
    int capacity; // 货物数量
    int waitTime; // 等待时间
    bool is_last_round; // 是否是最后一轮(游戏结束)
    Ship(int id): id(id) {
        status = 1;
        berthId = -1;
        capacity = 0;
        waitTime = 0;
        is_last_round = false;
    }
    int leftCapacity() {
        return MAX_Capacity - capacity;
    }
    void go(int berthId){
        printf("go %d\n", id);
        shipLogger.log(nowTime, "ship{0} go", id);
    }
    void go_berth(int berthId){
        printf("ship %d %d\n", id, berthId);
        shipLogger.log(nowTime, "ship{0} move to berth{1} 移动时间 {2}", id, berthId, berths[berthId]->time);
    }
    void move_berth(int new_berthId){
        shipLogger.log(nowTime, "ship{0} move from berth{1} to berth{2} 移动时间 :500", id, berthId, new_berthId);
        printf("ship %d %d\n", id, new_berthId);
    }
};

Ship *ships[MAX_Ship_Num];

#endif