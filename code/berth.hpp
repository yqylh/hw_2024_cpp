#ifndef __BERTH_H__
#define __BERTH_H__
#include "config.hpp"
#include "path.hpp"
#include "estimator.hpp"
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

    int robotNumLeft = 0;

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
        this->robotNumLeft = 0;
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


struct BuyRobotSorter {
    int berthId;
    double avgDistange; // second priority
    int buyId; // first priority
    bool operator < (const BuyRobotSorter &a) const {
        if (buyId != a.buyId) return buyId < a.buyId;
        return avgDistange < a.avgDistange;
    }
};
std::vector<BuyRobotSorter> _buyRobotQueue;

void initBerthEstimator(const int &totalSpawnPlace, const int &berthNumber) {
    berthEstimator = BerthEstimator(totalSpawnPlace, berthNumber);
    /*
    for (int i = 0; i < berthNumber; i++) {
        std::vector<Pos> beginPosList;
        for (auto & pos : berths[i]->usePos) {
            beginPosList.push_back(pos);
        }
        berthEstimator.checkBertehWithRobotNum(beginPosList, i, 3.0, 150);
    }
     */
    int avgRobotPerBerth = _maxRobotCnt / berthNumber;
    std::vector<std::vector<Pos>> beginPosList(berthNumber);
    std::vector<double> robotControlLengths(berthNumber, 9999.0);
    std::vector<double> controlNumber(berthNumber, avgRobotPerBerth);
    for (int i = 0; i < berthNumber; i++) {
        for (auto & pos : berths[i]->usePos) {
            beginPosList[i].push_back(pos);
        }
    }
    berthEstimator.checkBerthAll(beginPosList, controlNumber, robotControlLengths);
    std::vector<BuyRobotSorter> buyRobotSorter;

    for (int i = 0; i < berthNumber; i++) {
        auto avgNewItemPerPull = berthEstimator.berthState[i].avgNewItemPerPull(totalSpawnPlace);
        int robotNumber = int(avgNewItemPerPull + 0.5);
        for (int j = 1; j <= robotNumber; j++) {
            _buyRobotQueue.push_back({i, berthEstimator.berthState[i].avgDistanceToItem(), j});
        }
        estimatorLogger.log(0, "berthId={} totalGrid={} totalItemGrid={} avgDistanceToItem={}, avgNewItemPerPull={}", i, berthEstimator.berthState[i].totalGrid, berthEstimator.berthState[i].totalItemGrid, berthEstimator.berthState[i].avgDistanceToItem(), avgNewItemPerPull);
    }
    std::sort(_buyRobotQueue.begin(), _buyRobotQueue.end());

#ifdef DEBUG
    std::ofstream out("../log/berthbelong.txt");
    for (int i = 0; i < MAX_Line_Length; i++) {
        for (int j = 0; j < MAX_Col_Length; j++) {
            if (grids[i][j]->belongToBerth == -1)
                out << 0;
            else
                out << grids[i][j]->belongToBerth + 1;
        }
        out << std::endl;
    }
#endif
}

#endif