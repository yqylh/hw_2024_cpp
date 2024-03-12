#ifndef __INPUT_H__
#define __INPUT_H__
#include "config.hpp"
#include "robot.hpp"
#include "worktable.hpp"
#include "radar.hpp"
#include "grid.hpp"
#include <fstream>
#include <set>
#include <map>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>

void solveGraph() {
    std::unordered_map<int, double> buyDis456, buySellDis456, buySellDis7;
    for (auto & i : worktables) {
        if (i.type == 4 || i.type == 5 || i.type == 6) {
            double dis9 = 1e8;
            double dis1 = 1e8;
            double dis2 = 1e8;
            double dis3 = 1e8;
            for (auto & j : worktables) {
                if (j.type == 9 || j.type == 7) {
                    dis9 = std::min(dis9, WTtoWTwithItem[i.id][j.id]);
                }
                if (j.type == 1) {
                    dis1 = std::min(dis1, WTtoWTwithItem[i.id][j.id]);
                }
                if (j.type == 2) {
                    dis2 = std::min(dis2, WTtoWTwithItem[i.id][j.id]);
                }
                if (j.type == 3) {
                    dis3 = std::min(dis3, WTtoWTwithItem[i.id][j.id]);
                }
            }
            double dis = 0;
            if (i.type == 4) dis = dis1 + dis2;
            if (i.type == 5) dis = dis1 + dis3;
            if (i.type == 6) dis = dis2 + dis3;
            buyDis456.insert(std::make_pair(i.id, dis));
            buySellDis456.insert(std::make_pair(i.id, dis + dis9));
        }
    }
    for (auto & i : worktables) {
        if (i.type == 7) {
            double dis8 = 1e8;
            double dis4 = 1e8;
            double dis5 = 1e8;
            double dis6 = 1e8;
            for (auto & j : worktables) {
                if (j.type == 8 || j.type == 9) {
                    dis8 = std::min(dis8, WTtoWTwithItem[i.id][j.id]);
                }
                if (j.type == 4) {
                    dis4 = std::min(dis4, WTtoWTwithItem[i.id][j.id] + buyDis456[j.id]);
                }
                if (j.type == 5) {
                    dis5 = std::min(dis5, WTtoWTwithItem[i.id][j.id] + buyDis456[j.id]);
                }
                if (j.type == 6) {
                    dis6 = std::min(dis6, WTtoWTwithItem[i.id][j.id] + buyDis456[j.id]);
                }
            }
            buySellDis7.insert(std::make_pair(i.id, dis8 + dis4 + dis5 + dis6));
        }
    }
    if (buySellDis7.size() == 0) {
        // 没有7号工作台
        std::vector<std::pair<int, double> > near7;
        for (auto &_ : buySellDis456) {
            near7.push_back(std::make_pair(_.first, _.second));
        }
        std::sort(near7.begin(), near7.end(), [](const std::pair<int, double> & a, const std::pair<int, double> & b) {
            return a.second < b.second;
        }); // 按照距离排序
        int index = 0;
        for (auto & num7 : near7) {
            int id7 = num7.first;
            index++;
            // if (index <= near7.size() * 0.5) {
                worktables[id7].near7 = (cos(num7.second / near7.rbegin()->second * M_PI / 2) + 1.5) * 2;
                // worktables[id7].near7 = (cos(num7.second / near7.rbegin()->second * M_PI) + 2) * 2;
                // TESTOUTPUT(fout << "id " << id7 << " " << worktables[id7].near7 << std::endl;)
                if (worktables[id7].near7 < 1) worktables[id7].near7 = 1;
                // worktables[id7].near7 = 2;
            // } else {
            //    break;
            // }
            std::vector<std::pair<int, double> > near1, near2;
            for (auto & i : worktables) {
                if (i.type == 1 && EQUAL(i.near7, 1, 1e-3)) {
                    double dis = WTtoWTwithItem[i.id][id7];
                    near1.push_back(std::make_pair(i.id, dis));
                }
                if (i.type == 2 && EQUAL(i.near7, 1, 1e-3)) {
                    double dis = WTtoWTwithItem[i.id][id7];
                    near2.push_back(std::make_pair(i.id, dis));
                }
            }
            std::sort(near1.begin(), near1.end(), [](const std::pair<int, double> & a, const std::pair<int, double> & b) {
                return a.second < b.second;
            }); // 按照距离排序
            std::sort(near2.begin(), near2.end(), [](const std::pair<int, double> & a, const std::pair<int, double> & b) {
                return a.second < b.second;
            }); // 按照距离排序
                TESTOUTPUT(fout << "near with id=" << id7 << " is ";)
            if (near1.size() >= 1) {
                worktables[near1[0].first].near7 = 1.2;
                TESTOUTPUT(fout << near1[0].first << " ";)
            }
            if (near2.size() >= 1) {
                worktables[near2[0].first].near7 = 1.2;
                TESTOUTPUT(fout << near2[0].first << " ";)
            }
            TESTOUTPUT(fout << std::endl;)
        }
        // for (int id = 0; id < 8; id++) worktables[id].near7 = 0.01;
        // for (int id = worktableNum - 15 + 1; id <= worktableNum; id++) worktables[id].near7 = 0.01;
        return;
    }
    std::vector<std::pair<int, double> > near7;
    for (auto &_ : buySellDis7) {
        near7.push_back(std::make_pair(_.first, _.second));
    }
    // 有7号工作台  
    std::sort(near7.begin(), near7.end(), [](const std::pair<int, double> & a, const std::pair<int, double> & b) {
        return a.second < b.second;
    }); // 按照距离排序
    int index = 0;
    for (auto & num7 : near7) {
        int id7 = num7.first;
        if (index++ < near7.size() / 2 || near7.size() == 1) {
            worktables[id7].near7 = 4;
        } else {
            worktables[id7].near7 = 0.5;
            continue;
        }
        std::vector<std::pair<int, double> > near4, near5, near6;
        for (auto & i : worktables) {
            if (i.type == 4 && EQUAL(i.near7, 1, 1e-3)) {
                double dis = WTtoWTwithItem[i.id][id7];
                near4.push_back(std::make_pair(i.id, dis + buyDis456[i.id]));
            }
            if (i.type == 5 && EQUAL(i.near7, 1, 1e-3)) {
                double dis = WTtoWTwithItem[i.id][id7];
                near5.push_back(std::make_pair(i.id, dis + buyDis456[i.id]));
            }
            if (i.type == 6 && EQUAL(i.near7, 1, 1e-3)) {
                double dis = WTtoWTwithItem[i.id][id7];
                near6.push_back(std::make_pair(i.id, dis + buyDis456[i.id]));
            }
        }
        std::sort(near4.begin(), near4.end(), [](const std::pair<int, double> & a, const std::pair<int, double> & b) {
            return a.second < b.second;
        }); // 按照距离排序
        std::sort(near5.begin(), near5.end(), [](const std::pair<int, double> & a, const std::pair<int, double> & b) {
            return a.second < b.second;
        }); // 按照距离排序
        std::sort(near6.begin(), near6.end(), [](const std::pair<int, double> & a, const std::pair<int, double> & b) {
            return a.second < b.second;
        }); // 按照距离排序
        TESTOUTPUT(fout << "near with id=" << id7 << " is ";)
        if (near4.size() >= 1 ) {
            worktables[near4[0].first].near7 = 3;
            TESTOUTPUT(fout << near4[0].first << " ";)
        }
        if (near5.size() >= 1) {
            worktables[near5[0].first].near7 = 3;
            TESTOUTPUT(fout << near5[0].first << " ";)
        }
        if (near6.size() >= 1) {
            worktables[near6[0].first].near7 = 3;
            TESTOUTPUT(fout << near6[0].first << " ";)
        }
        TESTOUTPUT(fout << std::endl;)
    }
}

double bfs(Vector2D from, Vector2D to, int bring) {
        std::vector<Vector2D> path;
        std::map<Vector2D, Vector2D> fromWhere;
        std::queue<Vector2D> q;
        q.push(from);
        fromWhere.insert(std::make_pair(from, from));
        bool find = false;
        while (!q.empty() && find == false) {
            // 当前的位置
            Vector2D now = q.front();
            q.pop();
            // 八个方向的移动
            std::vector<std::pair<double, double>> adds = {{0, 0.5}, {0.5, 0}, {0, -0.5}, {-0.5, 0}, {0.5, 0.5}, {-0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}, {0, 0}};
            for (auto &add : adds) {
                // 移动后的位置
                Vector2D next = now + Vector2D(add.first, add.second);
                if (next.x <= 0.25 || next.x >= 49.75 || next.y <= 0.25 || next.y >= 49.75) continue;
                // 没访问过
                if (fromWhere.find(next) != fromWhere.end()) continue;
                // 是墙
                if (grids[next]->type == 1) continue;
                if (bring == 0) {
                    // 不携带物品
                    // 可以碰两个角
                    int num = 0;
                    std::vector<Vector2D> obstacles;
                    if (!(next == to))for (auto & item : grids[next]->obstacles) {
                        if ((next - item).length() < 0.45) {
                            num++;
                            obstacles.push_back(item);
                        }
                    }
                    int nowNum = 0;
                    for (auto & item : grids[now]->obstacles) {
                        if ((now - item).length() < 0.45) {
                            nowNum++;
                        }
                    }
                    if (nowNum >= 3) {
                        if (num > 1) continue; 
                    } else 
                    if (num > 2) continue;
                    // 被两边的卡死
                    if (num == 2 && (obstacles[0]-obstacles[1]).length() > 0.51) continue;
                    // 在墙角斜着走
                    if (num == 2 && nowNum == 2 && add.first != 0 && add.second != 0) continue;
                } else {
                    // 携带物品
                    int num = 0;
                    Vector2D obstacles;
                    if (!(next == to))for (auto & item : grids[next]->obstacles) {
                        if ((next - item).length() < 0.53) {
                            num++;
                            obstacles = item;
                        }
                    }
                    // 碰到了至少两个角,一定不能去
                    if (num > 1) continue;
                    // 只碰到了一个角,可能可以去
                    if (num == 1) {
                        Vector2D deltaNextToObstacles = obstacles - next;
                        Vector2D deltaNowToNext = next - now;
                        // 左上或者右下
                        if (deltaNextToObstacles.x == deltaNextToObstacles.y) {
                            if ((deltaNowToNext ^ deltaNextToObstacles) < 0) {
                                // 顺时针转
                                if (grids[next + Vector2D(-deltaNextToObstacles.x * 4, 0)]->type == 1) continue;
                                if (grids[next + Vector2D(0, -deltaNextToObstacles.y * 4)]->type == 1) continue;
                                if (grids[next + Vector2D(-deltaNextToObstacles.x * 4, 2 * deltaNextToObstacles.y)]->type == 1) continue;
                                if (grids[next + Vector2D(-deltaNextToObstacles.x * 4, 4 * deltaNextToObstacles.y)]->type == 1) continue;
                            } else if ((deltaNowToNext ^ deltaNextToObstacles) > 0)  {
                                if (grids[next + Vector2D(0, -deltaNextToObstacles.y * 4)]->type == 1) continue;
                                if (grids[next + Vector2D(-deltaNextToObstacles.x * 4, 0)]->type == 1) continue;
                                if (grids[next + Vector2D(2 * deltaNextToObstacles.x, -deltaNextToObstacles.y * 4)]->type == 1) continue;
                                if (grids[next + Vector2D(4 * deltaNextToObstacles.x, -deltaNextToObstacles.y * 4)]->type == 1) continue;
                            } else {
                                continue;
                            }
                        } else {
                            // 右下或者左上
                            if ((deltaNowToNext ^ deltaNextToObstacles) > 0) {
                                // 顺时针转
                                if (grids[next + Vector2D(-deltaNextToObstacles.x * 4, 0)]->type == 1) continue;
                                if (grids[next + Vector2D(0, -deltaNextToObstacles.y * 4)]->type == 1) continue;
                                if (grids[next + Vector2D(-deltaNextToObstacles.x * 4, 2 * deltaNextToObstacles.y)]->type == 1) continue;
                                if (grids[next + Vector2D(-deltaNextToObstacles.x * 4, 4 * deltaNextToObstacles.y)]->type == 1) continue;
                            } else if ((deltaNowToNext ^ deltaNextToObstacles) < 0)  {
                                if (grids[next + Vector2D(0, -deltaNextToObstacles.y * 4)]->type == 1) continue;
                                if (grids[next + Vector2D(-deltaNextToObstacles.x * 4, 0)]->type == 1) continue;
                                if (grids[next + Vector2D(2 * deltaNextToObstacles.x, -deltaNextToObstacles.y * 4)]->type == 1) continue;
                                if (grids[next + Vector2D(4 * deltaNextToObstacles.x, -deltaNextToObstacles.y * 4)]->type == 1) continue;
                            } else {
                                continue;
                            }
                        }
                    }   
                }
                fromWhere.insert(std::make_pair(next, now));
                q.push(next);
                if (next == to){
                    find = true;
                }
            }
        }
        while ( 1 ) {
            path.push_back(to);
            if (to == fromWhere[to]) break;
            to = fromWhere[to];
        }
        if (find == false) return 1e8;
        else 
        return path.size();
}

void solveWorktableToWorktable() {
    for (int i = 0; i <= worktableNum; i++) {
        for (int j = 0; j <= worktableNum; j++) {
            WTtoWT[i][j] = -1;
            WTtoWTwithItem[i][j] = -1;
        }
    }
    for (int i = 0; i <= worktableNum; i++) {
        if (worktables[i].near7 == 0) continue;
        for (int j = 0; j <= worktableNum; j++) {
            if (worktables[j].near7 == 0) continue;
            if (i == j) {
                WTtoWT[i][j] = 0;
                WTtoWTwithItem[i][j] = 0;
                WTtoWT[j][i] = 0;
                WTtoWTwithItem[j][i] = 0;
                continue;
            }
            if (WTtoWT[i][j] != -1) continue;
            double distance = bfs(Vector2D(worktables[i].x, worktables[i].y), Vector2D(worktables[j].x, worktables[j].y), 0);
            WTtoWT[i][j] = distance;
            WTtoWT[j][i] = distance;
            distance = bfs(Vector2D(worktables[i].x, worktables[i].y), Vector2D(worktables[j].x, worktables[j].y), 1);
            WTtoWTwithItem[i][j] = distance;
            WTtoWTwithItem[j][i] = distance;
        }
    }
    TESTOUTPUT(fout << "worktable to worktable 不带物品" << std::endl);
    for (int i = 0; i <= worktableNum; i++) {
        TESTOUTPUT(fout << i << "-->");
        for (int j = 0; j <= worktableNum; j++) {
            if (WTtoWT[i][j] == -1) WTtoWT[i][j] = 1e8;
            TESTOUTPUT(fout << WTtoWT[i][j] << " ");
        }
        TESTOUTPUT(fout << std::endl);
    }
    TESTOUTPUT(fout << "worktable to worktable 带物品" << std::endl);
    for (int i = 0; i <= worktableNum; i++) {
        TESTOUTPUT(fout << i << "-->");
        for (int j = 0; j <= worktableNum; j++) {
            if (WTtoWTwithItem[i][j] == -1) WTtoWTwithItem[i][j] = 1e8;
            TESTOUTPUT(fout << WTtoWTwithItem[i][j] << " ");
        }
        TESTOUTPUT(fout << std::endl);
    }
}
void solveRobotToWorktable(){
    for (int i = 0; i <= robotNum; i++) {
        for (int j = 0; j <= worktableNum; j++) {
            RobotToWT[i][j] = -1;
        }
    }
    for (int i = 0; i <= robotNum; i++) {
        for (int j = 0; j <= worktableNum; j++) {
            if (EQUAL(worktables[j].near7, 0, 1e-3)) continue;
            double distance = bfs(Vector2D(robots[i].x, robots[i].y), Vector2D(worktables[j].x, worktables[j].y), 0);
            if (distance < 1e7) {
                robots[i].couldReach.push_back(j);
            }
            RobotToWT[i][j] = distance;
        }
    }
    TESTOUTPUT(fout << "robot to worktable" << std::endl);
    for (int i = 0; i <= robotNum; i++) {
        TESTOUTPUT(fout << i << "-->");
        for (int j = 0; j <= worktableNum; j++) {
            TESTOUTPUT(fout << RobotToWT[i][j] << " ");
        }
        TESTOUTPUT(fout << std::endl);
    }
}
int couldReachZero = 0;
void solveMapNum() {
    int numWT[MAX_Worktable_Type_Num + 1] = {0};
    int numWTFoe[MAX_Worktable_Type_Num + 1] = {0};
    for (int i = 0; i <= worktableNum; i++) {
        numWT[worktables[i].type]++;
    }
    for (int i = 0; i <= worktableNumFoe; i++) {
        numWTFoe[worktablesFoe[i].type]++;
    }
    for (int i = 0; i <= robotNum; i++) {
        if (robots[i].couldReach.size() == 0) {
            couldReachZero++;
        }
    }
    // 有可能图四红方
    // if (couldReachZero == 0 && worktableNumFoe > 0) {
        // 可怜的三号
        // if (numWT[9] < 3) {
            // if (RoB == RED) {
            // if (RoB == BLUE)
                robots[3].isGankRobot = true;
            // } else {
            //     robots[2].isGankRobot = true;
            //     robots[3].isGankRobot = true;
            // }
        // }
    // }
    // if (couldReachZero == 1) {
    //     // 1:3的情况
    //     for (int i = 0; i <= robotNum; i++) {
    //         if (robots[i].couldReach.size() == 0) {
    //             robots[i].isGankRobot = true;
    //         }
    //     }
    //     flag2 = true;
    // }
    // if (couldReachZero == 4) {
    //     for (int i = 0; i <= robotNum; i++) {
    //         robots[i].isGankRobot = true;
    //     }
    // }
    if (couldReachZero == 0) couldReachZero++;
    for (int i = 0; i <= worktableNumFoe; i++) {
        if (worktablesFoe[i].type > 3 && worktablesFoe[i].type < 8) {
            gankFoeWT.push_back(worktablesFoe[i].id);
        }
    }
}
void inputMap(){
    std::string line;
    getline(std::cin, line);
    if (line == std::string("BLUE")) {
        RoB = BLUE;
        MAX_SPEED = 6;
        TOL_Collision = 4;
    } else {
        RoB = RED;
        MAX_SPEED = 7;
        TOL_Collision = 6;
    }
    for (int i = 0; i < MAP_Line_Length; i++) {
        std::string line;
        getline(std::cin, line);
        for (int j = 0; j < MAP_Col_Length; j++) {
            double x = 0.5*j+0.25;
            double y = 50 - 0.25 - 0.5*i;
            if (line[j] == '.') {
                grids[Vector2D(x,y)] = new Grid(Vector2D(x,y), 0);
                continue;
            }
            if (line[j] == '#') {
                grids[Vector2D(x,y)] = new Grid(Vector2D(x,y), 1);
                // 墙壁
                continue;
            }
            grids[Vector2D(x,y)] = new Grid(Vector2D(x,y), 0);
            if (RoB == BLUE) {
                // 蓝方
                if (line[j] == 'A') {
                    // A 表示自己的机器人 蓝色
                    robotNum++;
                    robots[robotNum] = Robot(robotNum, x,y);
                } else if (line[j] == 'B') {
                    // B 表示对方的机器人 红色
                    // todo
                    robotNumFoe++;
                    robotsFoe[robotNumFoe] = Robot(robotNumFoe, x,y);
                } else if (line[j] >= 'a' && line[j] <= 'i') {
                    // 对方工作台 红色
                    worktableNumFoe++;
                    worktablesFoe[worktableNumFoe] = Worktable(worktableNumFoe, x,y, char(line[j]) - 'a' + 1);
                } else if (line[j] >= '0' && line[j] <= '9') {
                    // 自己的工作台 蓝色
                    worktableNum++;
                    worktables[worktableNum] = Worktable(worktableNum, x,y, char(line[j]) - '0');
                } else throw;
            } else {
                // 红方
                if (line[j] == 'A') {
                    // A 表示对方的机器人 蓝色
                    // todo
                    robotNumFoe++;
                    robotsFoe[robotNumFoe] = Robot(robotNumFoe, x,y);
                } else if (line[j] == 'B') {
                    // B 表示自己的机器人 红色
                    robotNum++;
                    robots[robotNum] = Robot(robotNum, x,y);
                } else if (line[j] >= 'a' && line[j] <= 'i') {
                    // 自己的工作台 蓝色
                    worktableNum++;
                    worktables[worktableNum] = Worktable(worktableNum, x,y, char(line[j]) - 'a' + 1);
                } else if (line[j] >= '0' && line[j] <= '9') {
                    // 对方工作台 红色
                    worktableNumFoe++;
                    worktablesFoe[worktableNumFoe] = Worktable(worktableNumFoe, x,y, char(line[j]) - '0');
                } else throw;
            }
        }
    }
    detectionObstacle();
    while(getline(std::cin, line) && line != "OK");
    solveWorktableToWorktable();
    solveGraph();
    solveRobotToWorktable();
    solveMapNum();
    for (auto & robot : robotsFoe) {
        TESTOUTPUT(fout << "robot " << robot.id << " (" << robot.x << "," << robot.y << ") " << std::endl);
    }
    for (auto & wt : worktablesFoe) {
        TESTOUTPUT(fout << "worktable " << wt.id << " (" << wt.x << "," << wt.y << ") " << " type=" << wt.type << std::endl);
    }
    puts("OK");
    fflush(stdout);
}


void solveWTblocked(std::vector<std::vector<double>> enemyWT) {
    std::vector<std::pair<double, double>> adds = {{0, 0.5}, {0.5, 0}, {0, -0.5}, {-0.5, 0}, {0.5, 0.5}, {-0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}, {0, 0}};
    for (int i = 0; i <= worktableNum; i++) {
        worktables[i].blocked = false;
        for (auto & point : enemyWT) {
            double nowx = int(point[0] / 0.5) * 0.5 + 0.25;
            double nowy = int(point[1] / 0.5) * 0.5 + 0.25;
            for (auto & add : adds) {
                if (Vector2D(point[0] + add.first, point[1] + add.second) == Vector2D(worktables[i].x, worktables[i].y)) {
                    worktables[i].blocked = true;
                    break;
                }
            }
        }
        if (worktables[i].blocked == false) continue;
        TESTOUTPUT(fout << "there" << std::endl;)
        TESTOUTPUT(fout << worktables[i].x << " " << worktables[i].y << " been blocked" << std::endl;)
        for (int j = 0; j <= robotNum; j++) if (robots[j].path != nullptr) {
            // 正在去卖
            if (robots[j].path->sellWorktableId == robots[j].worktableTogo) {
                // 卖给的是这个工作台
                if (robots[j].worktableTogo == i) {
                    refindsell(i, j);
                }
            }
            // 正在去买 也就是 还没买
            else if (robots[j].path->buyWorktableId == robots[j].worktableTogo) {
                // 买的是这个工作台 或者 卖的是这个工作台
                if (robots[j].worktableTogo == i || robots[j].path->sellWorktableId == i) {
                    // 重新规划一个买卖阶段
                    robots[j].path = nullptr;
                }
            }
        }
    }
}
void solveFoeRobotPosition(std::vector<std::vector<double>> enemyRobotPoint, std::vector<std::vector<double>> enemyRobotVelocity) {
    std::vector<std::pair<double, double>> adds = {{0, 0.5}, {0.5, 0}, {0, -0.5}, {-0.5, 0}, {0.5, 0.5}, {-0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}, {0, 0}};
    for (auto pos = FoeBlockPos.begin(); pos != FoeBlockPos.end();) {
        if (grids[*pos]->foeEndTime <= nowTime) {
            grids[*pos]->type = 0;
            pos = FoeBlockPos.erase(pos);
        } else {
            pos++;
        }
    }
    std::vector<std::vector<double>> enemyWT;
    // 删除靠墙的虚假机器人
    std::vector<std::vector<double>> enemyRobotPoint2;
    int index = -1;
    TESTOUTPUT(fout << "enemyRobotPoint.size() = " << enemyRobotPoint.size() << std::endl;)
    TESTOUTPUT(fout << "enemyRobotVelocity.size() = " << enemyRobotVelocity.size() << std::endl;)
    for (auto & point : enemyRobotPoint) {
        index++;
        if (Vector2D(enemyRobotVelocity[index][0], enemyRobotVelocity[index][1]).length() * 50 < 1) {
            enemyWT.push_back(point);
        }
        if (Vector2D(enemyRobotVelocity[index][0], enemyRobotVelocity[index][1]).length() * 50 > 3) continue;
        if (point[0] < 0.5 || point[0] > 49.5 || point[1] < 0.5 || point[1] > 49.5) continue;
        enemyRobotPoint2.push_back(point);
    }
    enemyRobotPoint = enemyRobotPoint2;
    // 覆盖障碍物
    for (int i = 0; i < enemyRobotPoint.size(); i++) {
        double nowx = int(enemyRobotPoint[i][0] / 0.5) * 0.5 + 0.25;
        double nowy = int(enemyRobotPoint[i][1] / 0.5) * 0.5 + 0.25;
        Vector2D now(nowx, nowy);
        for (auto & add : adds) {
            Vector2D pos = now + Vector2D(add.first, add.second);
            if (grids.find(pos) == grids.end()) continue;
            if (grids[pos]->type == 0 || grids[pos]->foeTime > 0) {
                grids[pos]->setFoe(nowTime);
                FoeBlockPos.insert(pos);
            }
        }
    }
    solveWTblocked(enemyWT);
}

bool isSameRobot(const double & x1, const double & y1, const double & x2, const double & y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) < 0.4 * 0.4;
}

std::vector<std::vector<double>> lastEnemyRobotPoint;
std::vector<int> lastEnemyRobotCarry;
std::vector<std::vector<double>> enemyRobotPointAll;
std::vector<int> enemyRobotCarryAll;
std::vector<std::vector<double>> enemyRobotVelocityAll;
bool inputFrame() {
    if (scanf("%d%d",&nowTime, &money ) == EOF) {
        inputFlag = false;
        return false;
    }
    int tableNum;
    scanf("%d", &tableNum);
    // 处理工作台的信息
    for (int i = 0; i < tableNum; i++) {
        int type; // 工作台类型
        double x, y; // 工作台坐标
        int inputStatus; // 原材料状态
        int outputStatus; // 输出状态
        scanf("%d%lf%lf%d%d%d", &type, &x, &y, &worktables[i].remainTime, &inputStatus, &outputStatus);

        worktables[i].output = outputStatus;
        // 处理原材料对应的信息
        for (int bitnum = 1; bitnum <= MAX_Item_Type_Num; bitnum++) {
            if (inputStatus & (1 << bitnum)) {
                worktables[i].inputId[bitnum] = 1;
            } else {
                worktables[i].inputId[bitnum] = 0;
            }
        }
    }
    // 处理机器人的信息
    for (int i = 0; i <= robotNum; i++) {
        scanf("%d%d%lf%lf%lf%lf%lf%lf%lf%lf", &robots[i].worktableId, &robots[i].bringId, &robots[i].timeCoef, &robots[i].crashCoef, &robots[i].angularSeppd, &robots[i].linearSpeedX, &robots[i].linearSpeedY, &robots[i].direction, &robots[i].x, &robots[i].y);
    }

    std::vector<std::vector<double>> enemyRobotPoint;
    std::vector<int> enemyRobotCarry;
    std::vector<std::vector<double>> enemyRobotVelocity;
    std::vector<std::vector<std::vector<double>>> visibleRobotPoint;
    std::vector<std::vector<std::vector<double>>> visibleRobotVelocity;
    std::vector<std::vector<int>> visibleRobotCarry;

    for (int i = 0; i <= robotNum; i++) {
        std::vector<double> laser;
        laser.clear();
        for (int j = 0; j < 360; j++) {
            scanf("%lf", &robots[i].lasers[j]);
            laser.push_back(robots[i].lasers[j]);
        }
        visibleRobotPoint.push_back(std::vector<std::vector<double>>());

        double nowRobotX = robots[i].x;
        double nowRobotY = robots[i].y;
        double nowRobotDirection = robots[i].direction;
        std::vector<std::vector<double>> robotPoint;
        std::vector<int> robotCarry;
        robotPoint.clear();
        robotCarry.clear();
        Radar radar(nowRobotX, nowRobotY, nowRobotDirection, laser);
        radar.searchRobot(robotPoint, robotCarry);

        if (robotPoint.size() > 0) {
            // LOGGER_RADAR("nowTime=" << nowTime << "robotId=" << i);
            int robotPointCnt = 0;
            for (auto &point : robotPoint) {
                robotPointCnt++;
                // LOGGER_RADAR(point[0] << " " << point[1]);
                bool skip = false;
                if (point[0] < 0.5 || point[0] > 49.5 || point[1] < 0.5 || point[1] > 49.5) skip = true;
                for (int j = 0; j <= robotNum; j++) {
                    if (j == i) continue;
                    double deltaX = robots[j].x - point[0];
                    double deltaY = robots[j].y - point[1];
                    if (deltaX * deltaX + deltaY * deltaY < 0.4 * 0.4) {
                        skip = true; // is our robots
                        break;
                    }
                }
                if (skip) continue;
                for (auto & enemyPoints : enemyRobotPoint) {
                    double deltaX = enemyPoints[0] - point[0];
                    double deltaY = enemyPoints[1] - point[1];
                    if (deltaX * deltaX + deltaY * deltaY < 0.4 * 0.4) {
                        skip = true; // have been calculated
                        break;
                    }
                }
                if (skip) continue;
                enemyRobotPoint.push_back(point);
                enemyRobotCarry.push_back(robotCarry[robotPointCnt - 1]);
            }
        }

        for (int j = 0; j < enemyRobotPoint.size(); j++) {
            for (auto & point : robotPoint) {
                double deltaX = enemyRobotPoint[j][0] - point[0];
                double deltaY = enemyRobotPoint[j][1] - point[1];
                if (deltaX * deltaX + deltaY * deltaY < 0.4 * 0.4) {
                    visibleRobotPoint[i].push_back(point);
                }
            }
        }
    }

    LOGGER_RADAR("nowTime=" << nowTime << " enemyRobotPoint.size()=" << enemyRobotPoint.size());
    for (int i = 0; i < enemyRobotPoint.size(); i++) {
        LOGGER_RADAR("enemyRobotPoint[" << i << "]=" << enemyRobotPoint[i][0] << " " << enemyRobotPoint[i][1] << " " << enemyRobotCarry[i]);
    }

    for (int i = 0; i < enemyRobotPoint.size(); i++) {
        bool foundInLastFrame = false;
        for (int j = 0; j < lastEnemyRobotPoint.size(); j++) {
            double deltaX = lastEnemyRobotPoint[j][0] - enemyRobotPoint[i][0];
            double deltaY = lastEnemyRobotPoint[j][1] - enemyRobotPoint[i][1];
            if (deltaX * deltaX + deltaY * deltaY < 0.4 * 0.4) {
                // same enemy robot
                foundInLastFrame = true;
                double velocityX = enemyRobotPoint[i][0] - lastEnemyRobotPoint[j][0];
                double velocityY = enemyRobotPoint[i][1] - lastEnemyRobotPoint[j][1];
                enemyRobotVelocity.push_back({velocityX, velocityY});
                break;
            }
        }
        if (! foundInLastFrame) {
            enemyRobotVelocity.push_back({0, 0});
        }
    }


    for (int i = 0; i <= robotNum; i++) {
        visibleRobotVelocity.push_back(std::vector<std::vector<double>>());
        for (int visibleRobotId = 0; visibleRobotId < visibleRobotPoint[i].size(); visibleRobotId++) {
            bool foundInLastFrame = false;
            for (int j = 0; j < lastEnemyRobotPoint.size(); j++) {
                double deltaX = lastEnemyRobotPoint[j][0] - visibleRobotPoint[i][visibleRobotId][0];
                double deltaY = lastEnemyRobotPoint[j][1] - visibleRobotPoint[i][visibleRobotId][1];
                if (deltaX * deltaX + deltaY * deltaY < 0.4 * 0.4) {
                    // same enemy robot
                    foundInLastFrame = true;
                    double velocityX = visibleRobotPoint[i][visibleRobotId][0] - lastEnemyRobotPoint[j][0];
                    double velocityY = visibleRobotPoint[i][visibleRobotId][1] - lastEnemyRobotPoint[j][1];
                    visibleRobotVelocity[i].push_back({velocityX, velocityY});
                    break;
                }
            }
            if (! foundInLastFrame) {
                visibleRobotVelocity[i].push_back({0, 0});
            }
        }
        visibleRobotCarry.push_back(std::vector<int>());
        for (int visibleRobotId = 0; visibleRobotId < visibleRobotPoint[i].size(); visibleRobotId++) {
            for (int j = 0; j < enemyRobotPoint.size(); j++) {
                double deltaX = enemyRobotPoint[j][0] - visibleRobotPoint[i][visibleRobotId][0];
                double deltaY = enemyRobotPoint[j][1] - visibleRobotPoint[i][visibleRobotId][1];
                if (deltaX * deltaX + deltaY * deltaY < 0.4 * 0.4) {
                    visibleRobotCarry[i].push_back(enemyRobotCarry[j]);
                    break;
                }
            }
        }
    }


    lastEnemyRobotPoint = enemyRobotPoint;
    lastEnemyRobotCarry = enemyRobotCarry;

    // visibleRobotPoint
    // visibleRobotVelocity
    // std::vector<std::vector<int>> visibleRobotCarry;
    for (int i = 0; i <= robotNum; i++) {
        robots[i].visibleRobotPoint = visibleRobotPoint[i];
        robots[i].visibleRobotVelocity = visibleRobotVelocity[i];
        robots[i].visibleRobotCarry = visibleRobotCarry[i];
    }
    
    solveFoeRobotPosition(enemyRobotPoint,enemyRobotVelocity);
    enemyRobotPointAll = enemyRobotPoint;
    enemyRobotCarryAll = enemyRobotCarry;
    enemyRobotVelocityAll = enemyRobotVelocity;
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    return true;
}

std::vector<std::pair<int, int>> getRobotPriority() {
    std::vector<std::pair<int, int>> robotPriority;
    for (int i = 0; i <= robotNum; i++) {
        auto gotoTable = robots[i].worktableTogo;
        if (gotoTable <= 0) {
            robotPriority.push_back(std::make_pair(i, 7));
            continue;
        }
        if (gotoTable > worktableNum) {
            robotPriority.push_back(std::make_pair(i, 7));
            continue;
        }
        if (robots[i].path == nullptr) {
            robotPriority.push_back(std::make_pair(i, 7));
            continue;
        }
        auto type = worktables[gotoTable].type;
        if (robots[i].bringId == 0) {
            if (type == 7) robotPriority.push_back(std::make_pair(i, 3));
            if (type == 4 || type == 5 || type == 6) {
                if (worktables[robots[i].path->sellWorktableId].type == 7 && worktables[robots[i].path->sellWorktableId].waitPriority == 5) {
                    robotPriority.push_back(std::make_pair(i, 3));
                } else robotPriority.push_back(std::make_pair(i, 4));
            }
            if (type == 1 || type == 2 || type == 3) {
                if (worktables[robots[i].path->sellWorktableId].waitPriority == 4) {
                    robotPriority.push_back(std::make_pair(i, 5));
                } else robotPriority.push_back(std::make_pair(i, 6));
            }
        } else {
            if (robots[i].bringId == 7) robotPriority.push_back(std::make_pair(i, 1));
            if (robots[i].bringId == 4 || robots[i].bringId == 5 || robots[i].bringId == 6) {
                if (worktables[robots[i].path->sellWorktableId].type == 7 && worktables[robots[i].path->sellWorktableId].waitPriority == 5) {
                    robotPriority.push_back(std::make_pair(i, 1));
                } else robotPriority.push_back(std::make_pair(i, 2));  
            } 
            if (robots[i].bringId == 1 || robots[i].bringId == 2 || robots[i].bringId == 3) {
                if (worktables[robots[i].path->sellWorktableId].waitPriority == 4) {
                    robotPriority.push_back(std::make_pair(i, 4));
                } else robotPriority.push_back(std::make_pair(i, 5));
            }
        }
        // if (robots[i].bringId == 0) {
        //     if (type == 7) {
        //         robotPriority.push_back(std::make_pair(i, INT_MAX));
        //     } else {
        //         robotPriority.push_back(std::make_pair(i, nowTime - robots[i].runTime));
        //         if (type == 4 || type == 5 || type == 6) {
        //             robotPriority.back().second *= 1.2;
        //         }
        //     }
        // } else {
        //     if (robots[i].bringId == 7) {
        //         robotPriority.push_back(std::make_pair(i, INT_MAX));
        //     } else {
        //         robotPriority.push_back(std::make_pair(i, nowTime - robots[i].runTime));
        //         if (robots[i].bringId == 4 || robots[i].bringId == 5 || robots[i].bringId == 6) {
        //             robotPriority.back().second *= 1.4;
        //         }
        //     }
        // }
    }
    std::sort(robotPriority.begin(), robotPriority.end(), [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
        if (a.second == b.second) {
            return a.first < b.first;
        } else {
            return a.second < b.second;
        }
    });
    return robotPriority;
}
void solveFrame() {
    printf("%d\n", nowTime);
    TESTOUTPUT(fout << nowTime << std::endl;)
    for (int i = 0; i <= robotNum; i++) checkDestory(i);
    std::fill(canBuy, canBuy + MAX_Item_Type_Num + 1, 0);
    for (int i = 0; i <= worktableNum; i++) {
        worktables[i].checkWait();
        worktables[i].checkCanBuy();
        // TESTOUTPUT(worktables[i].outputTest();)
    }
    for (int i = 0; i <= robotNum; i++) {
        robots[i].checkCanBuy();
    }
    for (int i = 0; i <= robotNum; i++) {
        if (robots[i].isGankRobot) {
            { // 撤销
                for (auto pos = FoeBlockPos.begin(); pos != FoeBlockPos.end(); pos++)  grids[*pos]->type = 0;
            }
            robots[i].gankPoint = Vector2D(0,0);
            // 激光雷达识别到了一个地方机器人
            if (flag2) {
                if (robots[i].visibleRobotPoint.size() != 0) {
                    std::vector<int> robotFoeIndex;
                    robotFoeIndex.clear();
                    for (int j = 0; j < robots[i].visibleRobotPoint.size(); j++) robotFoeIndex.push_back(j);
                    std::sort(robotFoeIndex.begin(), robotFoeIndex.end(), [&](const int &a, const int &b) {
                        return (Vector2D(robots[i].x, robots[i].y) - Vector2D(robots[i].visibleRobotPoint[a][0], robots[i].visibleRobotPoint[a][1])).length() < (Vector2D(robots[i].x, robots[i].y) - Vector2D(robots[i].visibleRobotPoint[b][0], robots[i].visibleRobotPoint[b][1])).length();
                    });
                    for (auto & foeIndex : robotFoeIndex) {
                        auto & robotFoe = robots[i].visibleRobotPoint[foeIndex];
                        // 距离太远
                        if ((Vector2D(robots[i].x, robots[i].y) - Vector2D(robotFoe[0], robotFoe[1])).length() > 7.5) {
                            continue;
                        }
                        // 站着不动
                        if (Vector2D(robots[i].visibleRobotVelocity[foeIndex][0], robots[i].visibleRobotVelocity[foeIndex][1]).length() * 50 < 0.1 && (Vector2D(robots[i].visibleRobotPoint[foeIndex][0], robots[i].visibleRobotPoint[foeIndex][1]) - Vector2D(robots[i].x, robots[i].y)).length() > 1.5) {
                            continue;
                        }
                        robots[i].gankPoint = Vector2D(robotFoe[0] + robots[i].visibleRobotVelocity[foeIndex][0], robotFoe[1] + robots[i].visibleRobotVelocity[foeIndex][1]);
                        robots[i].gankPoint.x = int(robots[i].gankPoint.x / 0.5) * 0.5 + 0.25;
                        robots[i].gankPoint.y = int(robots[i].gankPoint.y / 0.5) * 0.5 + 0.25;
                        robots[i].moveToPoint(robots[i].gankPoint);
                        TESTOUTPUT(fout << "robot " << i << " gank " << robotFoe[0] << "," << robotFoe[1] << std::endl;)
                        robots[i].isGanking = true;
                        break;
                    }
                }
            } else {
                if (enemyRobotPointAll.size() != 0) {
                    std::vector<int> robotFoeIndex;
                    robotFoeIndex.clear();
                    for (int j = 0; j < enemyRobotPointAll.size(); j++) robotFoeIndex.push_back(j);
                    std::sort(robotFoeIndex.begin(), robotFoeIndex.end(), [&](const int &a, const int &b) {
                        return (Vector2D(robots[i].x, robots[i].y) - Vector2D(enemyRobotPointAll[a][0], enemyRobotPointAll[a][1])).length() - enemyRobotCarryAll[a] < (Vector2D(robots[i].x, robots[i].y) - Vector2D(enemyRobotPointAll[b][0], enemyRobotPointAll[b][1])).length() - enemyRobotCarryAll[b];
                    });
                    for (auto & foeIndex : robotFoeIndex) {
                        auto & robotFoe = enemyRobotPointAll[foeIndex];
                        // 站着不动
                        if (Vector2D(enemyRobotVelocityAll[foeIndex][0], enemyRobotVelocityAll[foeIndex][1]).length() * 50 < 0.1 && (Vector2D(enemyRobotPointAll[foeIndex][0], enemyRobotPointAll[foeIndex][1]) - Vector2D(robots[i].x, robots[i].y)).length() > 1.5) {
                            continue;
                        }
                        // 检查是否有人在攻击
                        bool isAttacking = false;
                        for (int j = 0; j < i; j++) {
                            if (robots[j].isGanking) {
                                if (robots[i].gankPoint == Vector2D(0,0)) continue;
                                if (isSameRobot(robots[j].gankPoint.x, robots[j].gankPoint.y, robotFoe[0], robotFoe[1])) {
                                    isAttacking = true;
                                    break;
                                }
                            }
                        }
                        if (isAttacking) continue;
                        // 没有人在攻击,就去攻击
                        robots[i].gankPoint = Vector2D(robotFoe[0], robotFoe[1]);
                        robots[i].gankPoint.x = int(robots[i].gankPoint.x / 0.5) * 0.5 + 0.25;
                        robots[i].gankPoint.y = int(robots[i].gankPoint.y / 0.5) * 0.5 + 0.25;
                        robots[i].moveToPoint(robots[i].gankPoint);
                        TESTOUTPUT(fout << "robot " << i << " gank " << robotFoe[0] << "," << robotFoe[1] << std::endl;)
                        robots[i].isGanking = true;
                        break;
                    }
                }
            }
            if (robots[i].gankPoint == Vector2D(0,0) && robots[i].isGanking == true && robots[i].pathPoints.size() != 0 && (robots[i].pathPoints.back() - Vector2D(robots[i].x, robots[i].y)).length() > 1.2) {
                if (robots[i].pathPoints.back() == Vector2D(0,0)) {}
                else robots[i].gankPoint = robots[i].pathPoints.back();
            }
            if (robots[i].gankPoint == Vector2D(0,0)){
                int &patrolNum = robots[i].patrolNum;
                Vector2D togo(worktablesFoe[gankFoeWT[patrolNum]].x, worktablesFoe[gankFoeWT[patrolNum]].y);
                // 没有识别到,去地方工作台巡逻
                if ((Vector2D(robots[i].x, robots[i].y)-togo).length() < 0.4) {
                    patrolNum = (patrolNum + couldReachZero) % gankFoeWT.size();
                }
                togo = Vector2D(worktablesFoe[gankFoeWT[patrolNum]].x, worktablesFoe[gankFoeWT[patrolNum]].y);
                TESTOUTPUT(fout << "robot " << i << " on" << robots[i].x << "," << robots[i].y  << " is going to " << togo.x << "," << togo.y << " length=" << (Vector2D(robots[i].x, robots[i].y)-togo).length() << std::endl;)
                robots[i].moveToPoint(togo);
                robots[i].isGanking = false;
            }
            { // 撤销
                for (auto pos = FoeBlockPos.begin(); pos != FoeBlockPos.end(); pos++)  grids[*pos]->type = 1;
            }
            continue;
        }
        robots[i].action();
        // TESTOUTPUT(robots[i].outputTest();) 
    }
    TESTOUTPUT(fout << "碰撞检测" << std::endl;)
    auto robotPriority = getRobotPriority();
    for (int robot2 = 1; robot2 <= robotNum; robot2++) {
        std::set<Vector2D> *robot1PathPoints = new std::set<Vector2D>();
        std::set<Vector2D> *robot1Points = new std::set<Vector2D>();        
        for (int robot1 = 0; robot1 < robot2; robot1++) {
            if (robots[robotPriority[robot1].first].isGanking) continue;
            if (robots[robotPriority[robot2].first].isGanking) continue;
            DetecteCollision(robotPriority[robot1].first, robotPriority[robot2].first, robot1PathPoints, robot1Points);
        }
    }
    TESTOUTPUT(fout << "开始移动" << std::endl;)
    for (int i = 0; i <= robotNum; i++) {
        robots[i].Move();
    }

    puts("OK");
    TESTOUTPUT(fout << "OK" << std::endl;)
    fflush(stdout);
}
#endif