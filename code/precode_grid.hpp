#ifndef __P_GRID__
#define __P_GRID__

#include "config.hpp"
#include "grid.hpp"
#include "berth.hpp"
#include "robot.hpp"

Grid *reachable_grids[MAX_Line_Length][MAX_Col_Length];
int min_dis_way[MAX_Line_Length][MAX_Col_Length][MAX_Berth_Num];
int min_dis[MAX_Line_Length][MAX_Col_Length][MAX_Berth_Num];

int sovle_grid(){
    // 初始化reachable_grids
    for (int i = 0; i < MAX_Line_Length; i++){
        for (int j = 0; j < MAX_Col_Length; j++){
            reachable_grids[i][j] = new Grid(i, j, 0);
        }
    }

    // 深度遍历, 从泊位出发, 标记可达的点(非零即可达)
    for (int i = 0 ; i < MAX_Berth_Num; i++){
        if (reachable_grids[berths[i]->pos.x][berths[i]->pos.y]->type != 0) continue; // 已经被标记过, 跳过
        // reachable_grids[berths[i]->pos.x][berths[i]->pos.y] = new Grid(berths[i]->pos.x, berths[i]->pos.y, i + 1); // 标记泊位 作为起点 !!起点不能在这里被标记
        Pos begin = berths[i]->pos;
        std::queue<Pos> q;
        std::unordered_map<Pos, Pos> pre;
        q.push(begin); // 将起点加入队列
        pre[begin] = begin;
        while (!q.empty()) {
            Pos now = q.front();
            q.pop();
            for (int j = 0; j < 4; j++) {
                if (grids[now.x][now.y]->type == 0 || grids[now.x][now.y]->type == 3) {
                    // 如果这个点是可达的, 就标记它, 并且加入队列
                    Pos next = now + dir[j]; // 下一个点
                    reachable_grids[now.x][now.y]->type = i + 1;
                    if (next.x < 0 || next.x >= MAX_Line_Length || next.y < 0 || next.y >= MAX_Col_Length) continue; // 越界
                    if ((grids[next.x][next.y]->type != 0 && grids[next.x][next.y]->type != 3) || reachable_grids[next.x][next.y]->type != 0 || pre.find(next) != pre.end()) continue; //不是空地或者泊位,或者已经被标记过, 跳过
                    q.push(next);
                    pre[next] = now;
                    
                }
            }
        }
    }

    // 计算距离需要重新跑一次循环,把每个点到每个泊位的最短距离都算出来
    for (int i = 0 ; i < MAX_Berth_Num; i++){
        Pos begin = berths[i]->pos;
        min_dis[begin.x][begin.y][i] = 0; 
        std::queue<Pos> q;
        std::unordered_map<Pos, Pos> pre;
        q.push(begin); // 将起点加入队列
        pre[begin] = begin;
        while (!q.empty()) {
            Pos now = q.front();
            q.pop();
            for (int j = 0; j < 4; j++) {
                if (grids[now.x][now.y]->type == 0 || grids[now.x][now.y]->type == 3) {
                    Pos next = now + dir[j]; // 下一个点
                    if (next.x < 0 || next.x >= MAX_Line_Length || next.y < 0 || next.y >= MAX_Col_Length) continue; // 越界
                    if ((grids[next.x][next.y]->type != 0 && grids[next.x][next.y]->type != 3) || min_dis[next.x][next.y][i] != 0 || pre.find(next) != pre.end()) continue; //不是空地或者泊位,或者已经被标记过, 跳过
                    min_dis_way[next.x][next.y][i] = j; // 记录下一个点的方向到数组
                    min_dis[next.x][next.y][i] = min_dis[now.x][now.y][i] + 1; // 记录下一个点的距离到数组
                    q.push(next);
                    pre[next] = now;
                    
                }
            }
        }
    }
    return 0;
}

#endif