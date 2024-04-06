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
TPos _queueRobot[40010];

void solveGridWithTime(Pos beginPos, int nowRobotId, int beginFrame=0) {
    memset(disWithTime, 0x3f, sizeof(disWithTime));
    
    int start = 0, end = 0;
    _queueRobot[end++] = TPos(beginPos, beginFrame);
    disWithTime[beginPos.x][beginPos.y] = beginFrame;

    while (start != end) {
        TPos now_s = _queueRobot[start++];
        auto now = now_s.pos;
        auto now_dis = now_s.dis;
        if (start == 40010) start = 0;        

        for (int i = 0; i < 4; i++) {
            Pos next = now + dir[i];
            if (NOT_VALID_GRID(next.x, next.y) or 
                not IS_ROBOT_ABLE(grids[next.x][next.y]->bit_type) or 
                disWithTime[next.x][next.y] <= now_dis + 1) continue;
            // never reached before or reached with longer time
            _queueRobot[end++] = TPos(next, now_dis + 1);
            if (end == 40010) end = 0;
            disWithTime[next.x][next.y] = now_dis + 1;
        }
    }
    return;
}

#endif