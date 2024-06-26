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
#include "estimator.hpp"

//  * 0‘.’ ： 空地                                   | 机器人可以走  会发生碰撞
//  * 1‘*’ ： 海洋                                   | 船可以走  会发生碰撞
//  * 2‘#’ ： 障碍                                   | 谁都不可以走
//  * 3‘B’ ： 泊位  同时算主干道主航道                  | 船和机器人都可以走 不会发生碰撞
//  * 4‘>’ ： 陆地主干道                              | 机器人可以走 不会发生碰撞
//  * 5‘~’ ： 海洋主航道                              | 船可以走 不会发生碰撞
//  * 6‘R’ ： 机器人购买地块，同时该地块也是主干道.        | 机器人可以走, 不会发生碰撞
//  * 7‘S’ ： 船舶购买地块，同时该地块也是主航道           | 船可以走, 不会发生碰撞
//  * 8‘K’ ： 靠泊区 算主航道                          | 船可以走 不会发生碰撞
//  * 9‘C’ ： 海陆立体交通地块                         | 船和机器人都可以走 会发生碰撞
//  * 11‘c’ ： 海陆立体交通地块，同时为主干道和主航道      |船和机器人都可以走 不会发生碰撞
//  * 12‘T’ ： 交货点 特殊的靠泊区 所以也算主航道         | 船可以走 不会发生碰撞


void inputMap(){
    int totalSpawnPlace = 0;
    for (int i = 0; i < MAX_Line_Length; i++) {
        std::string line;
        getline(std::cin, line);
        for (int j = 0; j < MAX_Col_Length; j++) {
            switch (line[j]) {
                case '.':
                    totalSpawnPlace++;
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
                    berth_center->robot_buyer.emplace_back(Pos(i, j));
                    break;
                case 'S':
                    grids[i][j] = new Grid(i, j, 7);
                    berth_center->ship_buyer.emplace_back(Pos(i, j));
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
                    berth_center->delivery.emplace_back(Pos(i, j));
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
    solveBerth();
    PreproShipAllAble();
    berth_center->find_private_space();
    initBerthEstimator(totalSpawnPlace, MAX_Berth_Num);

    // srand(time(0));
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
    counter.registerVector2D("robot_path");
    counter.registerVector2D("robot_pos");
    counter.registerVector2D("ship_pos");
    counter.registerVector2D("gds");
    counter.registerVariable("sum_goods", 0);
    counter.registerVariable("sum_goods_value", 0);
}

void do_special_frame() {
    
    if (nowTime == 15000-lastRoundRuningTime) {
        counter.lock("robot_move_length");
        counter.lock("robot_get_nums");
        counter.add("robot_get_value_before_lastgame", counter.getValue("robot_get_value"));
        counter.lock("robot_get_value_before_lastgame");
    }
    if (nowTime == 14999) counter.writeToFile("../log/counter.txt");
    if (nowTime == 14999) {
        for (auto berth : berths) {
            berthLogger.log(nowTime, "Berth={},sum_goods={},sum_value={}", berth->id, berth->total_goods, berth->total_value); 
        }
    }
}


bool inputFrame() {
    if (scanf("%d%d",&nowTime, &money ) == EOF) {
        inputFlag = false;
        return false;
    }
    int K;
    scanf("%d", &K);
    counter.push_back("gds",-1000,nowTime);
    for (int i = 1; i <= K; i++) {
        int x, y, value;
        scanf("%d%d%d", &x, &y, &value);
        if (value != 0) {
            unsolvedItems.emplace_back(x, y, value);
            unsolvedItems.back().beginTime = nowTime;
            counter.push_back("gds",x,y);
            counter.push_back("gds",-1,value);
        }
        counter.add("sum_goods", 1);
        counter.add("sum_goods_value", value);
    }
    int R;
    scanf("%d", &R);
    counter.push_back("robot_pos",-1000,nowTime);
    for (int i = 1; i <= R; i++) {
        int id, bring, x, y;
        scanf("%d%d%d%d", &id, &bring, &x, &y);
        robots[id]->bring = bring;
        robots[id]->pos = Pos(x, y);
        counter.push_back("robot_pos",x,y);
    }
    int B;
    scanf("%d", &B);
    counter.push_back("ship_pos",-1000,nowTime);
    for (int i = 1; i <= B; i++) {
        int id, bring, x, y, direction, status;
        scanf("%d%d%d%d%d%d", &id, &bring, &x, &y, &direction, &status);
        ships[id]->capacity = bring;
        ships[id]->pos = Pos(x, y);
        ships[id]->direction = direction;
        ships[id]->status = status;
        counter.push_back("ship_pos",x,y);
        counter.push_back("ship_pos",direction,status);
    }
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    return true;
}

void buyRobot() {
    // 按顺序选择买去哪个港口的机器人
    for (int i = 0; i < _buyRobotQueue.size(); i++) {
        if (_buyRobotQueue[i].buyed)
            continue;
        int berthId = _buyRobotQueue[i].berthId;
        int minDis = 0x3f3f3f3f;
        int choosedBuyerId = -1;

        // 遍历所有的buyer看能否买到
        for (int j = 0; j < berth_center->robot_buyer.size(); j++) {
            auto buyerX = berth_center->robot_buyer[j].pos.x;
            auto buyerY = berth_center->robot_buyer[j].pos.y;
            int berthToBuyerDis = berths[berthId]->disWithTimeBerth[buyerX][buyerY];

            // 能买到，找最近的一个买
            if (berthToBuyerDis < minDis) {
                minDis = berthToBuyerDis;
                choosedBuyerId = j;
            }
        }

        // 如果能买到，且钱够，且机器人数量不超过预期，就买
        if (minDis != 0x3f3f3f3f && ((nowTime != 1 and money > 2000) or (nowTime == 1 and money > 10000)) && robots.size() < exptRobotCnt) {
            newRobot(berth_center->robot_buyer[choosedBuyerId].pos.x, berth_center->robot_buyer[choosedBuyerId].pos.y, berthId);
            _buyRobotQueue[i].buyed = true;
        } 
    }
}

void buyShip() {
    for (auto & shipBuyer : berth_center->ship_buyer) {
        if ((nowTime == 1 or (nowTime > 5000 and nowTime < 100000) ) and money > 8000 && ships.size() < _maxShipCnt) newShip(shipBuyer.pos.x, shipBuyer.pos.y);
        break;
    }
}

void solveFrame() {
    flowLogger.log(nowTime, "当前帧数={0}", nowTime);
    measureAndExecute("Action for all robots", [&]() {for (auto & robot : robots) robot->action();});
    // 碰撞检测
    // solveCollision();
    // 移动
    measureAndExecute("Move all robots", [&]() {for (auto & robot : robots) robot->move();});
    // 时间向前推进
    if (allPath.size() > 0) allPath.pop_front();
    // 船只调度
    measureAndExecute("Move all ships", [&]() {
        berth_center->call_ship_and_berth_check();
        // 船只移动
        for (auto & ship : ships) ship->move();
    });
    do_special_frame();
    
    if (robots.size() < exptRobotCnt) {
        buyRobot();
        berth_center->update_robot_choose_berth();
    }

    // TODO: 在机器人拿货速度大于船运货速度时，买新的船
    if (ships.size() < _maxShipCnt) buyShip();

    puts("OK");
    fflush(stdout);
}
#endif