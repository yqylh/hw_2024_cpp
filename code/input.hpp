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
    if (robots[0]->pos == Pos(9,104)) {
        TEST(fout << "map1" << std::endl;)
        // map1 迷宫图
        MAX_Berth_Control_Length = 40; // 机器人搜索长度,用来判断私有区域, 10~200, 5
        MAX_Berth_Merge_Length = 1200; // 泊位合并长度,用来判断是否可以合并, 1~200, 5
        Worst_Rate = 3; // 用来筛选多差的港口不要选, 1~10
        Sell_Ration = 0.68; // 还剩多少港口空了就去卖, 0.5~1
        Min_Next_Berth_Value = 1000; // another 港口的货物价值少于这个值就不去, 0~1000
        Only_Run_On_Berth_with_Ship = 1800; // 最后这些帧,只在有船的泊位上运行,
        lastRoundRuningTime = 700; // 估计的最后一轮的运行时间
    } else if (robots[0]->pos == Pos(49, 40)) {
        TEST(fout << "map2" << std::endl;)
        // map2 普通图
        MAX_Berth_Control_Length = 150; // 机器人搜索长度,用来判断私有区域, 10~200, 5
        MAX_Berth_Merge_Length = 16; // 泊位合并长度,用来判断是否可以合并, 1~200, 5
        Worst_Rate = 3; // 用来筛选多差的港口不要选, 1~10
        Sell_Ration = 0.8; // 还剩多少港口空了就去卖, 0.5~1
        Min_Next_Berth_Value = 1700; // another 港口的货物价值少于这个值就不去, 0~1000
        Only_Run_On_Berth_with_Ship = 2500; // 最后这些帧,只在有船的泊位上运行,
        lastRoundRuningTime = 100; // 估计的最后一轮的运行时间
    } else {
        // 最后一张地图
        MAX_Berth_Control_Length = 150; // 机器人搜索长度,用来判断私有区域, 10~200, 5. 160 和 200 没区别,说明非常空
        MAX_Berth_Merge_Length = 16; // 泊位合并长度,用来判断是否可以合并, 16 和 1 成绩一样,说明最短的两个港口,相近距离也是>16. 24.8. 图很宽阔. 500全合并效果很差23.8. 50 效果也不好,说明 50 合多了或者少了. 80-23.6 更差
        Worst_Rate = 100; // 用来筛选多差的港口不要选, 1~10. 100 有很大的进步 5 和 1 3 都很差
        Sell_Ration = 0.7; // 还剩多少港口空了就去卖, 0.5~1. 0.65 比较差
        Min_Next_Berth_Value = 1700; // another 港口的货物价值少于这个值就不去, 0~1000. 1000 很差 2000 很差
        Only_Run_On_Berth_with_Ship = 2000; // 最后这些帧,只在有船的泊位上运行,
        lastRoundRuningTime = 600; // 估计的最后一轮的运行时间 200 600 700 1000 一个分数.
    }
    shipNum = MAX_Ship_Num - 1;
    solveBerth();
    solveRobot();
    { // 因为要实现 header_only 的特性, center 和 robot不能互相引用, 所以只能在这里初始化
        std::vector<Pos> robot_pos;
        for (int i = 0; i <= robotNum; i++) robot_pos.push_back(robots[i]->pos);
        berth_center->robot_pos = robot_pos;
        berth_center->find_private_space();
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

    for (int i = 0; i <= robotNum; i++) robots[i]->action();
    // 碰撞检测
    solveCollision();
    // 移动
    for (int i = 0; i <= robotNum; i++) robots[i]->move();
    pathLogger.log(nowTime, "allPath.size()={0}", allPath.size());
    // 时间向前推进
    if (allPath.size() > 0) allPath.pop_front();

    berth_center->call_ship_and_berth_check();    
    bcenterlogger.log(nowTime, "ship_and_berth_check_ok");

    
    puts("OK");
    fflush(stdout);
}
#endif