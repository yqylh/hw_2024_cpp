#ifndef __BERTH_H__
#define __BERTH_H__
#include "config.hpp"
#include "path.hpp"
#include <vector>

struct Berth {
    int id;
    Pos pos;
    std::vector<Pos> usePos; // 使用的位置
    int velocity; // 装载速度
    int disWithTimeBerth[MAX_Line_Length + 1][MAX_Col_Length + 1];
    //上面的是原始值，别改
    
    std::vector<int> shipId; // 表示当前泊位上的船的 id,可能有多个,用empty()判断是否有船
    int goodsNum; // 表示当前泊位上的货物数量
    int on_way_ship;
    int on_way_robot;
    int waitting_ship;
    int ship_wait_start_time;
    std::queue<int> item_value_queue;
    int sum_value;
    int total_value;
    int total_goods;
    Berth(int id, int x, int y, int velocity) : id(id), velocity(velocity) {
        this->pos = Pos(x, y);
        goodsNum = 0;
        on_way_ship = 0;
        on_way_robot = 0;
        waitting_ship = 0;
        ship_wait_start_time =0;
        this->shipId.clear();
        this->item_value_queue = std::queue<int>();
        this->sum_value = 0;
        this->total_value = 0;
        this->total_goods = 0;
    }
    void findUsePos() {
        usePos.clear();
        std::vector<std::pair<Pos, int>> arr;
        std::unordered_set<Pos> visited;
        std::queue<Pos> q;
        q.push(pos);
        while (!q.empty()) {
            auto top = q.front(); q.pop();
            int haveGround = 0;
            for (int d = 0; d <= 3; d++) {
                Pos next = top + dir[d];
                if (checkPos(next) == false) continue;
                if (checkRobotAble(next) && grids[next.x][next.y]->type != 3) haveGround++;
                if (visited.find(next) != visited.end()) continue;
                if (grids[next.x][next.y]->type == 3) {
                    q.push(next);
                    visited.insert(next);
                }
            }
            arr.emplace_back(top, haveGround);
        }
        std::sort(arr.begin(), arr.end(), [](const std::pair<Pos, int> &a, const std::pair<Pos, int> &b) {
            return a.second > b.second;
        });
        for (int i = 0; i < 2; i++) {
            usePos.push_back(arr[i].first);
        }
    }
    void recordBerth() {
        std::queue<Pos> q; q.push(pos);
        while (!q.empty()) {
            auto top = q.front(); q.pop();
            for (int d = 0; d <= 3; d++) {
                Pos next = top + dir[d];
                if (checkPos(next) && (grids[next.x][next.y]->type == 3 || grids[next.x][next.y]->type == 8)) {
                    if (grids[next.x][next.y]->berthId == -1) {
                        grids[next.x][next.y]->berthId = id;
                        q.push(next);
                    }
                }
            }
        }
    }
};

std::vector<Berth *> berths;
std::unordered_map<Pos, Berth*> pos2berth;

void solveBerth() {
    // 预处理每个泊位到每个虚拟点的时间
    for (int i = 0; i < MAX_Berth_Num; i++) {
        berths[i]->findUsePos();
        berths[i]->recordBerth();
        for (auto & pos : berths[i]->usePos) {
            pos2berth[pos] = berths[i];
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