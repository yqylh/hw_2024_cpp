#ifndef __PATH_H__
#define __PATH_H__

#include "config.hpp"
#include "grid.hpp"
#include <algorithm>


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
            robot_grids[fixPos[i].x][fixPos[i].y]->robotOnIt = true  * (1-robot_grids[fixPos[i].x][fixPos[i].y]->type);
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
                    robot_grids[tar.x][tar.y]->robotOnIt = false;
                }
            }
            if (lastSeenDis != -1 and lastSeenDis + 1 < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis + 1]) {
                    if (tar.x == -1) continue;
                    robot_grids[tar.x][tar.y]->robotOnIt = false;
                }
            }
            
            lastSeenDis = disWithTime[now.x][now.y];

            // add new robot
            if (lastSeenDis < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis]) {
                    if (tar.x == -1) continue;
                    
                    robot_grids[tar.x][tar.y]->robotOnIt = true * (1-robot_grids[tar.x][tar.y]->type);
                }
            }
            if (lastSeenDis + 1 < allPath.size()) {
                for (Pos tar : allPath[lastSeenDis + 1]) {
                    if (tar.x == -1) continue;
                    robot_grids[tar.x][tar.y]->robotOnIt = true * (1-robot_grids[tar.x][tar.y]->type);
                }
            }
        }
        

        for (int i = 0; i < 4; i++) {
            Pos next = now + dir[i];
            if (next.x < 0 || next.x >= MAX_Line_Length || next.y < 0 || next.y >= MAX_Col_Length) continue;
            if (disWithTime[next.x][next.y] <= disWithTime[now.x][now.y] + 1 || (robot_grids[next.x][next.y]->type == -1)) continue;
            if (robot_grids[next.x][next.y]->robotOnIt) continue;
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
            robot_grids[tar.x][tar.y]->robotOnIt = false;
        }
    }
    if (lastSeenDis != -1 and lastSeenDis + 1 < allPath.size()) {
        for (Pos tar : allPath[lastSeenDis + 1]) {
            if (tar.x == -1) continue;
            robot_grids[tar.x][tar.y]->robotOnIt = false;
        }
    }
    
    for (int i = 0; i < fixPos.size(); i++) {
        if (fixPos[i].x != -1 and i != nowRobotId) {
            robot_grids[fixPos[i].x][fixPos[i].y]->robotOnIt = false;
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
        if (robot_grids[now.x][now.y]->type != -1) {
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
int _arr_pos[40010];
int cal_next_dis(int pos1,int pos2){
    if (std::abs(pos1 - pos2) == 2) return 2;
    return 1;
}
TeDirection * sovleShip(Pos origin,int pos) { 
    //用于船只的各向异性BFS
    //pos方向: 00 表示右移一格 01 表示左移一格 10 表示上移一格 11 表示下移一格 或 0 表示右移一格 1 表示左移一格 2 表示上移一格 3 表示下移一格
    shipLogger.log(nowTime, "sovleShip");
    TeDirection * result = new TeDirection;
    result->setVisited(origin.x, origin.y, pos, 0);
    int start = 0, end = 0;
    _arr_s[end++] = origin;
    _arr_pos[end++] = pos;
    while (start < end) {
        Pos &now = _arr_s[start++];
        int now_pos = _arr_pos[start - 1];
        for (int i = 0; i < 4; i++) {
            Pos next = now + dir[i]; // 下一个点 
            // int next_pos = i;
            if (boat_grids[next.x][next.y]->type == -1) continue;           // 不是船可以走的地方,跳过
            int now_dis = result->isVisited(now.x, now.y, now_pos);
            int next_dis = now_dis + cal_next_dis(i, now_pos);              //计算从 此 到 下一个点的对应方向 的距离
            if (result->isVisited(next.x, next.y, i) <= next_dis) continue; //如果已经记录过,并且距离更小,跳过
            result->setVisited(next.x, next.y, i, next_dis);                //记录 (可能是覆盖记录)
            result->setDir(next.x, next.y, ((i == 0 || i == 2) ? i + 1 : i - 1));
            _arr_s[end++] = next;
        }
    }
    shipLogger.log(nowTime, "sovleShip finish");
    return result;
}


#endif