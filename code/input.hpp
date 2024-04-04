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

void gengerate_grid(int i, int j, int type){
    robot_grids[i][j] = new RoadGrid(i,j,type,0);
    boat_grids[i][j] = new RoadGrid(i,j,type,1);
}

void inputMap(){
    for (int i = 0; i < MAX_Line_Length; i++) {
        std::string line;
        getline(std::cin, line);
        
        for (int j = 0; j < MAX_Col_Length; j++) {
            if (line[j] == '.') {
                gengerate_grid(i, j, 0);
            } else if (line[j] == '*') {
                gengerate_grid(i, j, 1);
            } else if (line[j] == '#') {
                gengerate_grid(i, j, 2);
            } else if (line[j] == 'A') { //废弃
                // robotNum++;
                // robots[robotNum] = new Robot(robotNum, i, j);
                gengerate_grid(i, j, 0);
            } else if (line[j] == 'B') {
                gengerate_grid(i, j, 3);
            } else if (line[j] == '>') {
                gengerate_grid(i, j, 5);
            } else if (line[j] == '~') {
                gengerate_grid(i, j, 6);
            } else if (line[j] == 'R') {
                berth_center->robot_buyer.emplace_back(Pos(i, j));
                gengerate_grid(i, j, 7);
            } else if (line[j] == 'S') {
                berth_center->ship_buyer.emplace_back(Pos(i, j));
                gengerate_grid(i, j, 8);
            } else if (line[j] == 'K') {
                gengerate_grid(i, j, 9);
            } else if (line[j] == 'C') {
                gengerate_grid(i, j, 10);
            } else if (line[j] == 'c') {
                gengerate_grid(i, j, 11);
            } else if (line[j] == 'T') {
                gengerate_grid(i, j, 12);
            } else {
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
    solveBerth();
    berth_center->find_private_space();
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
        scanf("%d%d%d%d", &id, &bring, &x, &y);
        robots[id]->bring = bring;
        robots[id]->pos = Pos(x, y);
    }
    int B;
    scanf("%d", &B);
    for (int i = 1; i <= B; i++) {
        int id, bring, x, y, direction, status;
        scanf("%d%d%d%d%d%d", &id, &bring, &x, &y, &direction, &status);
        ships[id]->capacity = bring;
        ships[id]->pos = Pos(x, y);
        ships[id]->direction = direction;
        ships[id]->status = status;
    }
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    return true;
}

void solveFrame() {
    flowLogger.log(nowTime, "当前帧数={0}", nowTime);
    
    for (auto & robot : robots) robot->action();
    // 碰撞检测
    // solveCollision();
    // 移动
    for (auto & robot : robots) robot->move();
    if (nowTime == 1) {
        bcenterlogger.log(nowTime, "购买机器人！");
        for (auto & robotBuyer : berth_center->robot_buyer) {
            newRobot(robotBuyer.pos.x, robotBuyer.pos.y);
            newRobot(robotBuyer.pos.x, robotBuyer.pos.y);
            newRobot(robotBuyer.pos.x, robotBuyer.pos.y);
            newRobot(robotBuyer.pos.x, robotBuyer.pos.y);
        }
        bcenterlogger.log(nowTime, "购买机器人完成！");
    }
    // 时间向前推进
    if (allPath.size() > 0) allPath.pop_front();
    bcenterlogger.log(nowTime, "时间向前推进完成！");
    // 船只调度
    berth_center->call_ship_and_berth_check();
    bcenterlogger.log(nowTime, "船只调度完成！");
    // 船只移动
    for (auto & ship : ships) ship->move();
    bcenterlogger.log(nowTime, "船只移动完成！");
    do_special_frame();
    bcenterlogger.log(nowTime, "一帧完成！");
    puts("OK");
    fflush(stdout);
}
#endif