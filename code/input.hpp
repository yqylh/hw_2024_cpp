#ifndef __INPUT_H__
#define __INPUT_H__
#include "config.hpp"
#include "grid.hpp"
#include "robot.hpp"
#include "ship.hpp"
#include "berth.hpp"
#include "item.hpp"
#include "logger.hpp"
#include "berth_centre.hpp"

void inputMap(){
    for (int i = 0; i < MAX_Line_Length; i++) {
        std::string line;
        getline(std::cin, line);
    
        for (int j = 0; j < MAX_Col_Length; j++) {
            if (line[j] == '.') {
                grids[i][j] = new Grid(i, j, 0);
            } else if (line[j] == '*') {
                grids[i][j] = new Grid(i, j, 1);
            } else if (line[j] == '#') {
                grids[i][j] = new Grid(i, j, 2);
            } else if (line[j] == 'A') {
                robotNum++;
                robots[robotNum] = new Robot(robotNum, i, j);
                grids[i][j] = new Grid(i, j, 0);
            } else if (line[j] == 'B') {
                grids[i][j] = new Grid(i, j, 3);
            } else {
                throw;
            }
        }
    }
    for (int i = 0; i < MAX_Berth_Num; i++) {
        int id, x, y, time, velocity;
        scanf("%d%d%d%d%d", &id, &x, &y, &time, &velocity);
        berths[i] = new Berth(id, x, y, time, velocity);
        berthLogger.log(nowTime, "Berth{0} :id={1} x={2} y={3} time={4} v={5}", i, id, x, y, time, velocity);
    }
    std::cin >> MAX_Capacity;
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    for (int i = 0; i < MAX_Ship_Num; i++) ships[i] = new Ship(i);
    shipNum = MAX_Ship_Num - 1;
    solveBerth();
    solveRobot();
    // solveAllGrid();
    berth_center->do_first_frame();
    { // 因为要实现 header_only 的特性, center 和 robot不能互相引用, 所以只能在这里初始化
        std::vector<Pos> robot_pos;
        for (int i = 0; i <= robotNum; i++) robot_pos.push_back(robots[i]->pos);
        berth_center->solve_robot_berth(robot_pos);
    }
    srand(time(0));
    puts("OK");
    fflush(stdout);
}


bool inputFrame() {
    if (scanf("%d%d",&nowTime, &money ) == EOF) {
        inputFlag = false;
        return false;
    }
    int K;
    scanf("%d", &K);
    for (int i = 1; i <= K; i++) {
        int x, y, value;
        scanf("%d%d%d", &x, &y, &value);
        unsolvedItems.emplace_back(x, y, value);
        unsolvedItems.back().beginTime = nowTime;
    }
    for (int i = 0; i <= robotNum; i++) {
        scanf("%d%d%d%d", &robots[i]->bring, &robots[i]->pos.x, &robots[i]->pos.y, &robots[i]->status);
    }
    for (int i = 0; i <= shipNum; i++) {
        scanf("%d%d", &ships[i]->status, &ships[i]->berthId);
    }
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    return true;
}

void solveFrame() {
    flowLogger.log(nowTime, "当前帧数={0}", nowTime);

    berth_center->call_ship_and_berth_check();
    
    bcenterlogger.log(nowTime, "ship_and_berth_check_ok");

    for (int i = 0; i <= robotNum; i++) robots[i]->action();
    // 碰撞检测
    solveCollision();
    // 移动
    for (int i = 0; i <= robotNum; i++) robots[i]->move();
    pathLogger.log(nowTime, "allPath.size()={0}", allPath.size());
    // 时间向前推进
    if (allPath.size() > 0) allPath.pop_front();
    
    puts("OK");
    fflush(stdout);
}
#endif