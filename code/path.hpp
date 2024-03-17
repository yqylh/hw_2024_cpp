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

void solveGridWithTime(Pos begin, int nowRobotId) {
    memset(disWithTime, 0x3f, sizeof(disWithTime));
    memset(preWithTime, 0, sizeof(preWithTime));
    
    for (int i = 0; i < fixPos.size(); i++) {
        if (fixPos[i].x != -1 and i != nowRobotId) {
            grids[fixPos[i].x][fixPos[i].y]->robotOnIt = true;
        }
    }

    int start = 0;
    int end = 0;
    _queueRobot[end++] = begin;
    disWithTime[begin.x][begin.y] = 0;

    int lastSeenDis = -1;

    while (start != end) {
        Pos now = _queueRobot[start++];
        if (start == 40010) start = 0;

        if (lastSeenDis != disWithTime[now.x][now.y]) {
            pathLogger.log(nowTime, "lastSeen={},nowDis={}", lastSeenDis, disWithTime[now.x][now.y]);
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
                for (Pos tar : allPath[disWithTime[now.x][now.y]]) {
                    if (tar.x == -1) continue;
                    grids[tar.x][tar.y]->robotOnIt = true;
                }
            }
            if (lastSeenDis + 1 < allPath.size()) {
                for (Pos tar : allPath[disWithTime[now.x][now.y] + 1]) {
                    if (tar.x == -1) continue;
                    grids[tar.x][tar.y]->robotOnIt = true;
                }
            }
        }


        for (int i = 0; i < 4; i++) {
            Pos next = now + dir[i];
            if (next.x < 0 || next.x >= MAX_Line_Length || next.y < 0 || next.y >= MAX_Col_Length) continue;
            if (disWithTime[next.x][next.y] <= disWithTime[now.x][now.y] + 1 || (grids[next.x][next.y]->type != 0 && grids[next.x][next.y]->type != 3)) continue;
            if (grids[next.x][next.y]->robotOnIt) continue;
            _queueRobot[end++] = next;
            if (end == 40010) end = 0;
            disWithTime[next.x][next.y] = disWithTime[now.x][now.y] + 1;
            preWithTime[next.x][next.y] = now;
        }


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
    /*
    auto now = end;
    while (!(now == begin)) {
        path->push_back(now);
        now = _pre[now.x][now.y];
    }
    std::reverse(path->begin(), path->end());
    */
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

std::deque<Pos> findPathWithTime(Pos begin, Pos end) {
    std::deque<Pos> path;
    auto now = end;
    while (!(now == begin)) {
        path.push_back(now);
        now = preWithTime[now.x][now.y];
    }
    path.push_back(begin);
    std::reverse(path.begin(), path.end());
    return path;
}

#endif