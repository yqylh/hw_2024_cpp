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
            switch (line[j]) {
                case '.':
                    grids[i][j] = new Grid(i, j, 0);
                    break;
                case '*':
                    grids[i][j] = new Grid(i, j, 1);
                    break;
                case '#':
                    grids[i][j] = new Grid(i, j, 2);
                    break;
                case 'B':
                    grids[i][j] = new Grid(i, j, 3);
                    break;
                case '>':
                    grids[i][j] = new Grid(i, j, 4);
                    break;
                case '~':
                    grids[i][j] = new Grid(i, j, 5);
                    break;
                case 'R':
                    grids[i][j] = new Grid(i, j, 6);
                    break;
                case 'S':
                    grids[i][j] = new Grid(i, j, 7);
                    break;
                case 'K':
                    grids[i][j] = new Grid(i, j, 8);
                    break;
                case 'C':
                    grids[i][j] = new Grid(i, j, 9);
                    break;
                case 'c':
                    grids[i][j] = new Grid(i, j, 11);
                    break;
                case 'T':
                    grids[i][j] = new Grid(i, j, 12);
                    break;
                default:
                    throw;
            }
        }
    }
    scanf("%d", &MAX_Berth_Num);
    for (int i = 0; i < MAX_Berth_Num; i++) {
        int id, x, y, velocity;
        scanf("%d%d%d%d", &id, &x, &y, &velocity);
        berths.emplace_back(new Berth(id, x, y, velocity));
        berthLogger.log(nowTime, "Berth{0} :id={1} x={2} y={3} v={4}", i, id, x, y, velocity);
    }
    std::cin >> MAX_Capacity;
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    // solveBerth();
    // solveRobot();
    // { // 因为要实现 header_only 的特性, center 和 robot不能互相引用, 所以只能在这里初始化
    //     std::vector<Pos> robot_pos;
    //     for (auto & robot : robots) robot_pos.push_back(robot->pos);
    //     berth_center->robot_pos = robot_pos;
    //     berth_center->find_private_space();
    // }
    srand(time(0));
    puts("OK");
    fflush(stdout);
    counter.registerVariable("shipNum", MAX_Ship_Num);
    counter.registerVariable("robotNum", MAX_Robot_Num);
    counter.registerVariable("robot_move_length", 0);
    counter.registerVariable("robot_get_nums", 0);
    counter.registerVariable("robot_get_value", 0);
    counter.registerVariable("robot_get_value_before_lastgame", 0);
    counter.registerVariable("robot_move_length_max", 0);
    counter.registerVariable("robot_move_length_min", 40000);
    counter.registerVector("robot_move_length_vector");
}

void do_special_frame() {
    
    if (nowTime == 15000-lastRoundRuningTime) {
        counter.lock("robot_move_length");
        counter.lock("robot_get_nums");
        counter.add("robot_get_value_before_lastgame", counter.getValue("robot_get_value"));
        counter.lock("robot_get_value_before_lastgame");
    }
    if (nowTime == 14950) counter.writeToFile("../log/counter.txt");
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
        if (value != 0) {
            unsolvedItems.emplace_back(x, y, value);
            unsolvedItems.back().beginTime = nowTime;
        }
    }
    int R;
    scanf("%d", &R);
    for (int i = 1; i <= R; i++) {
        int id, bring, x, y;
        scanf("%d%d%d%d%d", &id, &bring, &x, &y);
    }
    int B;
    for (int i = 1; i <= B; i++) {
        int id, bring, x, y, direction, status;
        scanf("%d%d%d%d%d%d", &id, &bring, &x, &y, &direction, &status);
    }
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    return true;
}

void solveFrame() {
    // flowLogger.log(nowTime, "当前帧数={0}", nowTime);

    // for (auto & robot : robots) robot->action();
    // // 碰撞检测
    // solveCollision();
    // // 移动
    // for (auto & robot : robots) robot->move();
    // pathLogger.log(nowTime, "allPath.size()={0}", allPath.size());
    // // 时间向前推进
    // if (allPath.size() > 0) allPath.pop_front();

    // berth_center->call_ship_and_berth_check();    
    // bcenterlogger.log(nowTime, "ship_and_berth_check_ok");

    // do_special_frame();
    
    puts("OK");
    fflush(stdout);
}
#endif