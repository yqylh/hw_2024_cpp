#ifndef __PATH_H__
#define __PATH_H__

#include "config.hpp"
#include "grid.hpp"


struct TPos {
    Pos pos;
    int dis;
    TPos(Pos pos, int dis) : pos(pos), dis(dis) {}
    TPos() {}
    bool operator < (const TPos &a) const {
        return dis < a.dis;
    }
};

int disWithTime[MAX_Line_Length + 1][MAX_Col_Length + 1];
Pos preWithTime[MAX_Line_Length + 1][MAX_Col_Length + 1];
int delayWithTime[MAX_Line_Length + 1][MAX_Col_Length + 1];
std::deque<std::vector<Pos>> allPath;
std::vector<Pos> fixPos(MAX_Robot_Num, Pos(-1, -1));
TPos _queueRobot[40010];

void solveGridWithTime(Pos beginPos, int nowRobotId, int beginFrame=0) {
    memset(disWithTime, 0x3f, sizeof(disWithTime));
    memset(preWithTime, 0xff, sizeof(preWithTime));
    memset(delayWithTime, 0, sizeof(delayWithTime));
    
    for (int i = 0; i < fixPos.size(); i++) {
        if (i != nowRobotId and fixPos[i].x != -1 and not IS_ROBOT_NOCOL(grids[fixPos[i].x][fixPos[i].y]->bit_type)) {
            grids[fixPos[i].x][fixPos[i].y]->robotOnIt = true;
        }
    }

    int start = 0;
    int end = 0;
    _queueRobot[end++] = TPos(beginPos, beginFrame);
    disWithTime[beginPos.x][beginPos.y] = beginFrame;

    int lastSeenDis = -1;

    while (start != end) {
        TPos now_s = _queueRobot[start++];
        auto now = now_s.pos;
        auto now_dis = now_s.dis;
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

        
        if (lastSeenDis != now_dis) {
            // pathLogger.log(nowTime, "lastSeen={},nowDis={}", lastSeenDis, disWithTime[now.x][now.y]);
            // remove last robot
            if (lastSeenDis != -1 and lastSeenDis < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis]) {
                    if (tar.x == -1 or NOT_VALID_GRID(tar.x, tar.y) or IS_ROBOT_NOCOL(grids[tar.x][tar.y]->bit_type)) continue;
                    grids[tar.x][tar.y]->robotOnIt = false;
                }
            }
            if (lastSeenDis != -1 and lastSeenDis + 1 < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis + 1]) {
                    if (tar.x == -1 or NOT_VALID_GRID(tar.x, tar.y) or IS_ROBOT_NOCOL(grids[tar.x][tar.y]->bit_type)) continue;
                    grids[tar.x][tar.y]->robotOnIt = false;
                }
            }
            
            lastSeenDis = now_dis;

            // add new robot
            if (lastSeenDis < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis]) {
                    if (tar.x == -1 or NOT_VALID_GRID(tar.x, tar.y) or IS_ROBOT_NOCOL(grids[tar.x][tar.y]->bit_type)) continue;
                    grids[tar.x][tar.y]->robotOnIt = true;
                }
            }
            if (lastSeenDis + 1 < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis + 1]) {
                    if (tar.x == -1 or NOT_VALID_GRID(tar.x, tar.y) or IS_ROBOT_NOCOL(grids[tar.x][tar.y]->bit_type)) continue;
                    grids[tar.x][tar.y]->robotOnIt = true;
                }
            }
        }

        if (grids[now.x][now.y]->robotOnIt) continue;
        

        for (int i = 0; i < 4; i++) {
            Pos next = now + dir[i];
            // if (checkRobotAble(next) == false) continue;
            if (NOT_VALID_GRID(next.x, next.y) or 
                not IS_ROBOT_ABLE(grids[next.x][next.y]->bit_type) or 
                disWithTime[next.x][next.y] <= now_dis + 1 or 
                grids[next.x][next.y]->robotOnIt) continue;
            // never reached before or reached with longer time
            _queueRobot[end++] = TPos(next, now_dis + 1);
            if (end == 40010) end = 0;
            disWithTime[next.x][next.y] = now_dis + 1;
            preWithTime[next.x][next.y] = now;
            // could reach through delay
            // delay at now position
            delayWithTime[next.x][next.y] = now_dis - disWithTime[now.x][now.y];
        }

        /*
        if (now_dis - disWithTime[now.x][now.y] <= 5) {
            _queueRobot[end++] = TPos(now, now_dis + 1);
            if (end == 40010) end = 0;
        }
        */
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
            if (tar.x == -1 or NOT_VALID_GRID(tar.x, tar.y) or IS_ROBOT_NOCOL(grids[tar.x][tar.y]->bit_type)) continue;
            grids[tar.x][tar.y]->robotOnIt = false;
        }
    }
    if (lastSeenDis != -1 and lastSeenDis + 1 < allPath.size()) {
        for (Pos tar : allPath[lastSeenDis + 1]) {
            if (tar.x == -1 or NOT_VALID_GRID(tar.x, tar.y) or IS_ROBOT_NOCOL(grids[tar.x][tar.y]->bit_type)) continue;
            grids[tar.x][tar.y]->robotOnIt = false;
        }
    }
    
    for (int i = 0; i < fixPos.size(); i++) {
        if (i != nowRobotId and fixPos[i].x != -1 and not IS_ROBOT_NOCOL(grids[fixPos[i].x][fixPos[i].y]->bit_type)) {
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
    path.push_back(now);
    while (!(now == beginPos)) {
        if (NOT_VALID_GRID(now.x, now.y) or not IS_ROBOT_ABLE(grids[now.x][now.y]->bit_type)) {
            path.clear();
            return path;
        }

        
        while (delayWithTime[now.x][now.y] > 0) {
            path.push_back(preWithTime[now.x][now.y]);
            delayWithTime[now.x][now.y]--;
            pathLogger.log(nowTime, "delayWithTime={}, nowx={}, nowy={}, disWithTime={}", delayWithTime[now.x][now.y], now.x, now.y, disWithTime[now.x][now.y]);
        }

        path.push_back(preWithTime[now.x][now.y]);
        now = preWithTime[now.x][now.y];
    }
    // path.push_back(beginPos);
    std::reverse(path.begin(), path.end());
    return path;
}

#endif