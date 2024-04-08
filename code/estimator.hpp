#ifndef __ESTIMATOR_H__
#define __ESTIMATOR_H__

#include "config.hpp"
#include "grid.hpp"

struct BerthEstState {
    Pos pos;
    int dis;
    int belongTo;
    BerthEstState(Pos pos, int dis) : pos(pos), dis(dis) {}
    BerthEstState(Pos pos, int dis, int belongTo) : pos(pos), dis(dis), belongTo(belongTo) {}
    BerthEstState() {}

};

class BerthState {
public:
    double totItemDistance;
    int totalGrid;
    int totalItemGrid;
    int realBerthItem;
    BerthState() {
        totItemDistance = 0;
        totalGrid = 0;
        totalItemGrid = 0;
        realBerthItem = 0;
    }

    inline double avgDistanceToItem() {
        if (totalItemGrid == 0) return 0;
        return totItemDistance / (double)totalItemGrid;
    }

    inline double avgNewItemPerPull(const int &totalSpawnGrid) {
        if (totalItemGrid == 0) return 0;
        auto avgDistance = this->avgDistanceToItem();
        double spawnRatePerFrame = 4900.0 / 15000.0;
        double spwanRatePerFrameInArea = spawnRatePerFrame * (double)totalItemGrid / totalSpawnGrid;
        double avgPullTime = avgDistance * 2.0;
        return spwanRatePerFrameInArea * avgPullTime;
    }
};

std::vector<bool> _berthDone;
BerthEstState _estQueue[400010];

class BerthEstimator {
public:
    std::vector<BerthState> berthState;
    int totalSpawnPlace;
    BerthEstimator() { }
    BerthEstimator(int totalSpawnPlace, int berthNumber) {
        this->totalSpawnPlace = totalSpawnPlace;
        berthState = std::vector<BerthState>(berthNumber);
    }
    
    void checkBertehWithRobotNum(const std::vector<Pos> &beginPosList, const int &berthId, const double &controlNumber, const double &robotControlLength) {
        
        int start = 0;
        int end = 0;
        for (auto beginPos : beginPosList) {
            _estQueue[end++] = BerthEstState(beginPos, 0);
            // _estimatorDistance[beginPos.x][beginPos.y] = 0;
        }
        estimatorLogger.log(0, "berthId={} robotNum={}, totalSpawnPlace={}", berthId, beginPosList.size(), totalSpawnPlace);
        int lastDis = 0;

        while (start != end) {
            auto nowS = _estQueue[start++];
            auto now = nowS.pos;
            auto nowDis = nowS.dis;
            if (start == 400010) start = 0;

            if (grids[now.x][now.y]->belongToBerth != -1) continue;
            if (nowDis > robotControlLength) break;


            berthState[berthId].totalGrid += 1;
            if (grids[now.x][now.y]->type == 0) { // 可刷新物品的格子
                berthState[berthId].totalItemGrid += 1;
                berthState[berthId].totItemDistance += nowDis;
            }

            grids[now.x][now.y]->belongToBerth = berthId;

            auto avgNewItemPerPull = berthState[berthId].avgNewItemPerPull(totalSpawnPlace);
            if (nowDis != lastDis) {
                lastDis = nowDis;
                estimatorLogger.log(0, "nowDis={} avgNewItemPerPull={}, totalItemGrid={}, totalItemDistance={}", nowDis, avgNewItemPerPull, berthState[berthId].totalItemGrid, berthState[berthId].totItemDistance);
            }
            
            if (avgNewItemPerPull > controlNumber) {
                break;
            }

            for (int d = 0; d <= 3; d++) {
                Pos next = now + dir[d];
                if (NOT_VALID_GRID(next.x, next.y) or 
                    (not IS_ROBOT_ABLE(grids[next.x][next.y]->bit_type)) or
                    (grids[next.x][next.y]->belongToBerth != -1)) 
                    continue;
                _estQueue[end++] = BerthEstState(next, nowDis + 1);
                if (end == 400010) end = 0;
            }
        }
        estimatorLogger.log(0, "berthId={} totalGrid={} totalItemGrid={} avgDistanceToItem={}", berthId, berthState[berthId].totalGrid, berthState[berthId].totalItemGrid, berthState[berthId].avgDistanceToItem());
        estimatorLogger.log(0, "avgNewItemPerPull={}", berthState[berthId].avgNewItemPerPull(totalSpawnPlace));
    }

    void checkBerthAll(const std::vector<std::vector<Pos>> &beginPosLists, const std::vector<double> &controlNumbers, const std::vector<double> &robotControlLengths) {
        _berthDone = std::vector<bool>(beginPosLists.size(), false);
        
        int start = 0;
        int end = 0;

        int maxPosCnt = 0;
        for (int betrhId = 0; betrhId < beginPosLists.size(); betrhId++) {
            maxPosCnt = std::max(maxPosCnt, (int)beginPosLists[betrhId].size());
        }
        for (int i = 0; i < maxPosCnt; i++) {
            for (int betrhId = 0; betrhId < beginPosLists.size(); betrhId++) {
                if (i < beginPosLists[betrhId].size()) {
                    _estQueue[end++] = BerthEstState(beginPosLists[betrhId][i], 0, betrhId);
                }
            }
        }

        while (start != end) {
            auto nowS = _estQueue[start++];
            auto now = nowS.pos;
            auto nowDis = nowS.dis;
            auto berthId = nowS.belongTo;
            if (start == 400010) start = 0;

            if (grids[now.x][now.y]->belongToBerth != -1) continue;
            if (_berthDone[berthId]) continue;
            if (nowDis > robotControlLengths[berthId]) {
                _berthDone[berthId] = true;
                continue;
            }


            berthState[berthId].totalGrid += 1;
            if (grids[now.x][now.y]->type == 0) { // 可刷新物品的格子
                berthState[berthId].totalItemGrid += 1;
                berthState[berthId].totItemDistance += nowDis;
            }

            grids[now.x][now.y]->belongToBerth = berthId;

            auto avgNewItemPerPull = berthState[berthId].avgNewItemPerPull(totalSpawnPlace);
            
            if (avgNewItemPerPull > controlNumbers[berthId]) {
                _berthDone[berthId] = true;
            }

            for (int d = 0; d <= 3; d++) {
                Pos next = now + dir[d];
                if (NOT_VALID_GRID(next.x, next.y) or 
                    (not IS_ROBOT_ABLE(grids[next.x][next.y]->bit_type)) or
                    (grids[next.x][next.y]->belongToBerth != -1)) 
                    continue;
                _estQueue[end++] = BerthEstState(next, nowDis + 1, berthId);
                if (end == 400010) end = 0;
            }
        }
    }
};


BerthEstimator berthEstimator;

#endif