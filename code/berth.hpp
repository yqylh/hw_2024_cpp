#ifndef __BERTH_H__
#define __BERTH_H__
#include "config.hpp"
#include "path.hpp"
#include <vector>

struct Berth {
    int id;
    Pos pos;
    std::vector<Pos> usePos; // 使用的位置
    std::vector<Direction *> usePosDir; // 使用的位置 任何节点到这个位置的下一步方向
    int time; // 运输到虚拟点的时间  
    int velocity; // 装载速度
    int disWithTimeBerth[MAX_Line_Length + 1][MAX_Col_Length + 1];

    //上面的是原始值，别改
    
    std::vector<int> shipId; // 表示当前泊位上的船的 id,可能有多个,用empty()判断是否有船
    bool selected; // 表示当前泊位是否被选中
    int goodsNum; // 表示当前泊位上的货物数量
    int on_way_ship;
    int on_way_robot;
    int waitting_ship;
    int ship_wait_start_time;
    Berth(int id, int x, int y, int time, int velocity) : id(id), time(time), velocity(velocity) {
        this->pos = Pos(x, y);
        selected = false;
        goodsNum = 0;
        on_way_ship = 0;
        on_way_robot = 0;
        waitting_ship = 0;
        ship_wait_start_time =0;
    }
    void findUsePos() {
        usePos.clear();
        std::vector<std::pair<Pos, int>> arr;
        for (int i = 0; i <= 3; i++) {
            for (int j = 0; j <= 3; j++) {
                Pos temp = pos + Pos(i, j);
                int haveGround = 0;
                for (int d = 0; d <= 3; d++) {
                    Pos temp2 = temp + dir[d];
                    if (temp2.x < 0 || temp2.x >= MAX_Line_Length || temp2.y < 0 || temp2.y >= MAX_Col_Length) {
                        continue;
                    }
                    if (grids[temp2.x][temp2.y]->type == 0) {
                        haveGround++;
                    }
                }
                arr.emplace_back(temp, haveGround);
            }
        }
        std::sort(arr.begin(), arr.end(), [](const std::pair<Pos, int> &a, const std::pair<Pos, int> &b) {
            return a.second > b.second;
        });
        for (int i = 0; i < 2; i++) {
            usePos.push_back(arr[i].first);
        }
    }
};

Berth *berths[MAX_Berth_Num];
std::unordered_map<Pos, Berth*> pos2berth;

void solveBerth() {
    std::vector<std::pair<int, int>> arr;
    /**
     * 选择最优的泊位
     * 选择的标准是速度和时间的加权和
     * 不一定有用
    */
    for (int i = 0; i < MAX_Berth_Num; i++) {
        arr.emplace_back(i, berths[i]->velocity + berths[i]->time * 2);
        // arr.emplace_back(i, berths[i]->velocity * MAX_Capacity + berths[i]->time * 2);
    }
    std::sort(arr.begin(), arr.end(), [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
        return a.second < b.second;
    });
    // 只选择前 MAX_Ship_Num 个泊位
    for (int i = 0; i < MAX_Ship_Num; i++) {
        berths[arr[i].first]->selected = true;
    }
    // 预处理每个泊位到每个虚拟点的时间
    for (int i = 0; i < MAX_Berth_Num; i++) {
        berths[i]->findUsePos();
        for (auto & pos : berths[i]->usePos) {
            pos2berth[pos] = berths[i];
            berths[i]->usePosDir.push_back(sovleGrid(pos));
        }
        solveGridWithTime(berths[i]->pos, -1);
        for (int j = 0; j < MAX_Line_Length; j++) {
            for (int k = 0; k < MAX_Col_Length; k++) {
                berths[i]->disWithTimeBerth[j][k] = disWithTime[j][k];
            }
        }
    }
}

#endif