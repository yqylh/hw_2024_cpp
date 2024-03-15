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
    Ship(int id): id(id) {
        status = 1;
        berthId = -1;
        capacity = 0;
        waitTime = 0;
    }
    void action() {
        TEST(fout << "ship的状态" << id << " " << status << " " << berthId << " " << capacity << " " << waitTime << " max" << MAX_Capacity << std::endl;)
        if ((capacity == MAX_Capacity && nowTime >= waitTime) 
            || (capacity > MAX_Capacity) 
            || (berthId != -1 && nowTime + berths[berthId]->time + 10 >= MAX_TIME)) {
            if (status == 0) return;
            status = 0;
            capacity = 0;
            berthId = -1;
            printf("go %d\n", id);
            TEST(fout << "go " << id << std::endl;)
            return;
        }
        if (status == 1 && berthId == -1) {
            for (int i = 0; i < MAX_Berth_Num; i++) {
                if (berths[i]->shipId == -1) {
                    berthId = i;
                    berths[i]->shipId = id;
                    printf("ship %d %d\n", id, i);
                    TEST(fout << "ship " << id << " " << i << std::endl;)
                    TEST(fout << "ship移动的时间=" << berths[i]->time << std::endl;)
                    break;
                }
            }
        }
    }
    void go(int berthId){
        printf("go %d\n", id);
    }
};

Ship *ships[MAX_Ship_Num];

#endif