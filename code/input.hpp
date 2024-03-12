#ifndef __INPUT_H__
#define __INPUT_H__
#include "config.hpp"
#include "robot.hpp"
#include "ship.hpp"
#include "grid.hpp"

void inputMap(){
    for (int i = 0; i < MAP_Line_Length; i++) {
        std::string line;
        getline(std::cin, line);
        for (int j = 0; j < MAP_Col_Length; j++) {
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
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    return true;
}

void solveFrame() {
    printf("%d\n", nowTime);
    TESTOUTPUT(fout << nowTime << std::endl;)

    for (int i = 0; i <= robotNum; i++) robots[i].action();

    // TESTOUTPUT(fout << "碰撞检测" << std::endl;)

    TESTOUTPUT(fout << "开始移动" << std::endl;)
    for (int i = 0; i <= robotNum; i++) robots[i].Move();

    puts("OK");
    TESTOUTPUT(fout << "OK" << std::endl;)
    fflush(stdout);
}
#endif