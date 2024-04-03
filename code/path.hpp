#ifndef __PATH_H__
#define __PATH_H__

#include "config.hpp"
#include "grid.hpp"


struct TPos {
    Pos pos;
    int dis;
    TPos(Pos pos, int dis) : pos(pos), dis(dis) {}
    bool operator < (const TPos &a) const {
        return dis < a.dis;
    }
};

int disWithTime[MAX_Line_Length + 1][MAX_Col_Length + 1];
Pos preWithTime[MAX_Line_Length + 1][MAX_Col_Length + 1];
std::deque<std::vector<Pos>> allPath;
std::vector<Pos> fixPos(MAX_Robot_Num, Pos(-1, -1));
Pos _queueRobot[40010];

void solveGridWithTime(Pos beginPos, int nowRobotId, int beginFrame=0) {
    memset(disWithTime, 0x3f, sizeof(disWithTime));
    memset(preWithTime, 0xff, sizeof(preWithTime));
    
    for (int i = 0; i < fixPos.size(); i++) {
        if (fixPos[i].x != -1 and i != nowRobotId) {
            if (checkRobotNoColl(fixPos[i])) continue;
            grids[fixPos[i].x][fixPos[i].y]->robotOnIt = true;
        }
    }

    int start = 0;
    int end = 0;
    _queueRobot[end++] = beginPos;
    disWithTime[beginPos.x][beginPos.y] = beginFrame;

    int lastSeenDis = -1;

    while (start != end) {
        Pos now = _queueRobot[start++];
        if (start == 40010) start = 0;

        /*
        if (disWithTime[now.x][now.y] < allPath.size()) {
            for (Pos tar : allPath[disWithTime[now.x][now.y]]) {
                if (tar.x == -1) continue;
                grids[tar.x][tar.y]->robotOnIt = true;
            }
        }
        if (disWithTime[now.x][now.y] + 1 < allPath.size()) {
            for (Pos tar : allPath[disWithTime[now.x][now.y] + 1]) {
                if (tar.x == -1) continue;
                grids[tar.x][tar.y]->robotOnIt = true;
            }
        }
        */

        
        if (lastSeenDis != disWithTime[now.x][now.y]) {
            // pathLogger.log(nowTime, "lastSeen={},nowDis={}", lastSeenDis, disWithTime[now.x][now.y]);
            // remove last robot
            if (lastSeenDis != -1 and lastSeenDis < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis]) {
                    if (tar.x == -1) continue;
                    grids[tar.x][tar.y]->robotOnIt = false;
                }
            }
            if (lastSeenDis != -1 and lastSeenDis + 1 < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis + 1]) {
                    if (tar.x == -1) continue;
                    grids[tar.x][tar.y]->robotOnIt = false;
                }
            }
            
            lastSeenDis = disWithTime[now.x][now.y];

            // add new robot
            if (lastSeenDis < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis]) {
                    if (tar.x == -1) continue;
                    if (checkRobotNoColl(tar)) continue;
                    grids[tar.x][tar.y]->robotOnIt = true;
                }
            }
            if (lastSeenDis + 1 < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis + 1]) {
                    if (tar.x == -1) continue;
                    if (checkRobotNoColl(tar)) continue;
                    grids[tar.x][tar.y]->robotOnIt = true;
                }
            }
        }
        

        for (int i = 0; i < 4; i++) {
            Pos next = now + dir[i];
            if (checkRobotAble(next) == false) continue;
            if (disWithTime[next.x][next.y] <= disWithTime[now.x][now.y] + 1) continue;
            if (grids[next.x][next.y]->robotOnIt) continue;
            _queueRobot[end++] = next;
            if (end == 40010) end = 0;
            disWithTime[next.x][next.y] = disWithTime[now.x][now.y] + 1;
            preWithTime[next.x][next.y] = now;
        }
        /*
        if (disWithTime[now.x][now.y] < allPath.size()) {
            for (Pos tar : allPath[disWithTime[now.x][now.y]]) {
                if (tar.x == -1) continue;
                grids[tar.x][tar.y]->robotOnIt = false;
            }
        }
        if (disWithTime[now.x][now.y] + 1 < allPath.size()) {
            for (Pos tar : allPath[disWithTime[now.x][now.y] + 1]) {
                if (tar.x == -1) continue;
                grids[tar.x][tar.y]->robotOnIt = false;
            }
        }
        */
    }
    
    if (lastSeenDis != -1 and lastSeenDis < allPath.size()) {
        for (Pos tar : allPath[lastSeenDis]) {
            if (tar.x == -1) continue;
            grids[tar.x][tar.y]->robotOnIt = false;
        }
    }
    if (lastSeenDis != -1 and lastSeenDis + 1 < allPath.size()) {
        for (Pos tar : allPath[lastSeenDis + 1]) {
            if (tar.x == -1) continue;
            grids[tar.x][tar.y]->robotOnIt = false;
        }
    }
    
    for (int i = 0; i < fixPos.size(); i++) {
        if (fixPos[i].x != -1 and i != nowRobotId) {
            grids[fixPos[i].x][fixPos[i].y]->robotOnIt = false;
        }
    }

    return;
}

void addPathToAllPath(std::deque<Pos> path, int nowRobotId) {
    for (int nowFrame = 0; nowFrame < path.size(); nowFrame++) {
        if (nowFrame >= allPath.size()) {
            // vector<Pos> size is equal to robotNum
            allPath.push_back(std::vector<Pos> (MAX_Robot_Num));
        }
        allPath[nowFrame][nowRobotId] = path[nowFrame];
    }
    for (int nowFrame = path.size(); nowFrame < allPath.size(); nowFrame++) {
        allPath[nowFrame][nowRobotId] = Pos(-1, -1);
    }
    fixPos[nowRobotId] = Pos(-1, -1);
}

void deletePathFromAllPath(int nowRobotId) {
    for (int nowFrame = 0; nowFrame < allPath.size(); nowFrame++) {
        allPath[nowFrame][nowRobotId] = Pos(-1, -1);
    }
}

void updateFixPos(Pos pos, int nowRobotId) {
    fixPos[nowRobotId] = pos;
}

int getDirWithPath(Pos now, Pos next) {
    if (now.x == next.x) {
        if (now.y < next.y) return 0;
        else return 1;
    } else {
        if (now.x < next.x) return 3;
        else return 2;
    }
}

std::deque<Pos> findPathWithTime(Pos beginPos, Pos endPos) {
    std::deque<Pos> path;
    auto now = endPos;
    while (!(now == beginPos)) {
        if (checkRobotAble(now) == false) {
            path.clear();
            return path;
        }
        path.push_back(now);
        now = preWithTime[now.x][now.y];
    }
    path.push_back(beginPos);
    std::reverse(path.begin(), path.end());
    return path;
}

Pos _arr_s[40010];
Direction * sovleShip(Pos origin) {
    Direction * result = new Direction;
    result->setVisited(origin.x, origin.y);
    int start = 0, end = 0;
    _arr_s[end++] = origin;
    while (start < end) {
        Pos &now = _arr_s[start++];
        for (int i = 0; i < 4; i++) {
            Pos next = now + dir[i]; // 下一个点
            if (checkShipAble(next) == false) continue; // 不是机器人可以走的地方
            if (result->isVisited(next.x, next.y)) continue; //记录过前序, 跳过
            result->setVisited(next.x, next.y);
            result->setDir(next.x, next.y, ((i == 0 || i == 2) ? i + 1 : i - 1));
            _arr_s[end++] = next;
        }
    }
    return result;
}


#endif