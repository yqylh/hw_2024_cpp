#ifndef __INPUT_H__
#define __INPUT_H__
#include "config.hpp"
#include "robot.hpp"
#include "ship.hpp"
#include "grid.hpp"
#include "berth.hpp"

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
            }
        }
    }
    for (int i = 0; i < MAX_Berth_Num; i++) {
        int x, y, time, velocity;
        scanf("%d%d%d%d", &x, &y, &time, &velocity);
        berths[i] = new Berth(i, x, y, time, velocity);
    }
    std::cin >> capacity;
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    puts("OK");
    fflush(stdout);
}


bool inputFrame() {
//     if (scanf("%d%d",&nowTime, &money ) == EOF) {
//         inputFlag = false;
//         return false;
//     }
//     int tableNum;
//     scanf("%d", &tableNum);
//     // 处理工作台的信息
//     for (int i = 0; i < tableNum; i++) {
//         int type; // 工作台类型
//         double x, y; // 工作台坐标
//         int inputStatus; // 原材料状态
//         int outputStatus; // 输出状态
//         scanf("%d%lf%lf%d%d%d", &type, &x, &y, &worktables[i].remainTime, &inputStatus, &outputStatus);

//         worktables[i].output = outputStatus;
//         // 处理原材料对应的信息
//         for (int bitnum = 1; bitnum <= MAX_Item_Type_Num; bitnum++) {
//             if (inputStatus & (1 << bitnum)) {
//                 worktables[i].inputId[bitnum] = 1;
//             } else {
//                 worktables[i].inputId[bitnum] = 0;
//             }
//         }
//     }
//     // 处理机器人的信息
//     for (int i = 0; i <= robotNum; i++) {
//         scanf("%d%d%lf%lf%lf%lf%lf%lf%lf%lf", &robots[i].worktableId, &robots[i].bringId, &robots[i].timeCoef, &robots[i].crashCoef, &robots[i].angularSeppd, &robots[i].linearSpeedX, &robots[i].linearSpeedY, &robots[i].direction, &robots[i].x, &robots[i].y);
//     }
//     std::string line;
//     while(getline(std::cin, line) && line != "OK");
//     return true;
}

void solveFrame() {
//     TESTOUTPUT(fout << nowTime << std::endl;)

//     for (int i = 0; i <= robotNum; i++) robots[i].action();

//     // TESTOUTPUT(fout << "碰撞检测" << std::endl;)

//     TESTOUTPUT(fout << "开始移动" << std::endl;)
//     for (int i = 0; i <= robotNum; i++) robots[i].Move();

    puts("OK");
//     TESTOUTPUT(fout << "OK" << std::endl;)
    fflush(stdout);
}
#endif