#ifndef __REBOT_H__
#define __REBOT_H__
#pragma GCC optimize(3)

#include "config.hpp"
#include "worktable.hpp"
#include "path.hpp"
#include "grid.hpp"
#include "radar.hpp"
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
#include <climits>
#include <queue>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
struct Robot{
    int id; // 机器人的 id
    double x; // 机器人的 x 坐标
    double y; // 机器人的 y 坐标
    int worktableId; // 机器人正在处理的工作台的 id, -1 表示没有
    int bringId; // 携带的物品的类型, 0 表示没有
    double timeCoef; // 机器人的时间系数
    double crashCoef; // 机器人的碰撞系数
    double angularSeppd; // 机器人的角速度
    double linearSpeedX; // 机器人的线速度X
    double linearSpeedY; // 机器人的线速度Y
    double direction; // 机器人的方向
    int worktableTogo; // 机器人要去的工作台的 id
    Path *path; // 机器人的路径
    std::vector<Vector2D> pathPoints; // 机器人的路径点
    int zeroTime = 0; // 机器人的零速度时间
    bool isWait = false; // 机器人是否在等待
    std::vector<int> couldReach; // 机器人能够到达的工作台
    double lasers[360]; // 机器人的激光雷达
    int runTime; // 机器人的运行时间
    bool isGanking; // 机器人是否正在干扰
    bool isGankRobot = false; // 攻击机器人
    int patrolNum = 0; // 巡逻到几号工作台了
    Vector2D gankPoint; // 干扰点
    bool haveBeenGanked = false; // 是否被干扰
    std::vector<std::vector<double>> visibleRobotPoint;
    std::vector<std::vector<double>> visibleRobotVelocity;
    std::vector<int> visibleRobotCarry;
    bool willDestroy;
    Robot() {
        this->id = -1;
        this->x = -1;
        this->y = -1;
        worktableId = -1;
        bringId = 0;
        timeCoef = 0;
        crashCoef = 0;
        angularSeppd = 0;
        linearSpeedX = 0;
        linearSpeedY = 0;
        direction = 0;
        path = nullptr;
        zeroTime = 0;
        runTime = 0;
        isGanking = false;
        willDestroy = false;
        gankPoint = Vector2D(0, 0);
    }
    Robot(int id, double x, double y) {
        this->id = id;
        this->x = x;
        this->y = y;
        worktableId = -1;
        bringId = 0;
        timeCoef = 0;
        crashCoef = 0;
        angularSeppd = 0;
        linearSpeedX = 0;
        linearSpeedY = 0;
        direction = 0;
        path = nullptr;
        zeroTime = 0;
        runTime = 0;
        isGanking = false;
        willDestroy = false;
        gankPoint = Vector2D(0, 0);
    }
    void checkCanBuy() {
        if (bringId != 0) {
            canBuy[bringId]--;
        }
    }
    void outputTest() {
        TESTOUTPUT(fout << "Robot id: " << id << std::endl;)
        // TESTOUTPUT(fout << "x: " << x << std::endl;)
        // TESTOUTPUT(fout << "y: " << y << std::endl;)
        // TESTOUTPUT(fout << "worktableId: " << worktableId << std::endl;)
        // TESTOUTPUT(fout << "bringId: " << bringId << std::endl;)
        // TESTOUTPUT(fout << "timeCoef: " << timeCoef << std::endl;)
        // TESTOUTPUT(fout << "crashCoef: " << crashCoef << std::endl;)
        // TESTOUTPUT(fout << "angularSeppd: " << angularSeppd << std::endl;)
        // TESTOUTPUT(fout << "linearSpeedX: " << linearSpeedX << std::endl;)
        // TESTOUTPUT(fout << "linearSpeedY: " << linearSpeedY << std::endl;)
        // TESTOUTPUT(fout << "direction: " << direction << std::endl;)
        // TESTOUTPUT(fout << "worktableTogo: " << worktableTogo << std::endl;)
        TESTOUTPUT(fout << "speed" << Vector2D(linearSpeedX, linearSpeedY).length() << std::endl;)
    }
    // 卖材料的函数
    void Sell() {
        if (bringId == 0) return;// 机器人没有携带原材料
        if (path == nullptr) return;
        // 机器人正在携带原材料
        if (worktableId != path->sellWorktableId) return; // 机器人不在预想工作台附近
        // 机器人正在工作台附近
        if (worktables[worktableId].inputId[bringId] == 0 
            && sellSet.find(std::make_pair(bringId, worktables[worktableId].type)) != sellSet.end()) {
            // 该工作台的此原料不满,且支持卖出
            TESTOUTPUT(fout << "sell " << id << std::endl;)
            printf("sell %d\n", id);
            // 统计损失
            lossCollMoney += sellMoneyMap[worktables[worktableId].type] * (1 - crashCoef);
            if (crashCoef < 1) {
                TESTOUTPUT(fout << "有碰撞损失: " << crashCoef << std::endl;)
            }
            lossTimeMoney += sellMoneyMap[worktables[worktableId].type] * (1 - timeCoef);
            // 工作台这一位设置成有东西了
            worktables[worktableId].inputId[bringId] = 1;
            {
                /**
                 * 如果没有生产的情况下, 该工作台的所有原料都满了, 则清空该工作台的原料
                */
                bool full = true;
                for (int item = 1; item <= MAX_Item_Type_Num; item++) {
                    if (sellSet.find(std::make_pair(item, worktables[worktableId].type)) != sellSet.end()) {
                        if (worktables[worktableId].inputId[item] == 0) {
                            full = false;
                            break;
                        }
                    }
                }
                if (full && worktables[worktableId].remainTime == -1) {
                    for (int item = 1; item <= MAX_Item_Type_Num; item++) {
                        worktables[worktableId].inputId[item] = 0;
                    }
                    worktables[worktableId].waitPriority = 3;
                }
            }
            worktables[worktableId].someWillSell[bringId]--;
            bringId = 0;
            path = nullptr;
            worktableTogo = -1;
            runTime = nowTime;
        }
    }
    // 买商品的函数
    void Buy() {
        if (bringId != 0) return;// 机器人已经携带原材料
        if (path == nullptr) return;
        // 机器人没有携带原材料
        if (worktableId != path->buyWorktableId) return; // 机器人不在预想工作台附近
        // 机器人正在工作台附近
        if (worktables[worktableId].output == true && money >= buyMoneyMap[createMap[worktables[worktableId].type]]) {
            // // 如果买了卖不出去
            // {
            //     // 如果因为碰撞到了这里发现买了卖不出去了
            //     // 就直接去下个点吧,只不过不买
            //     double goSellTime = WTtoWTwithItem[path->buyWorktableId][path->sellWorktableId];
            //     goSellTime = goSellTime * 0.65 / (MAX_SPEED * 0.9 / 50);
            //     if (goSellTime + nowTime > MAX_TIME) {
            //         worktables[worktableId].someWillBuy--;
            //         worktableTogo = path->sellWorktableId;
            //         pathPoints = movePath();
            //         return;
            //     }
            // }
            TESTOUTPUT(fout << "buy " << id << std::endl;)
            printf("buy %d\n", id);
            bringId = createMap[worktables[worktableId].type];
            money -= buyMoneyMap[createMap[worktables[worktableId].type]];
            worktables[worktableId].someWillBuy--;
            worktables[worktableId].output = false;
            if (worktables[worktableId].remainTime == 0) {
                worktables[worktableId].output = true;
            }
            worktableTogo = path->sellWorktableId;
            pathPoints = movePath();
            runTime = nowTime;
        }
    }


    void Move() {
        if (worktableTogo == -1 && isGankRobot == false) {
            pathPoints.push_back(Vector2D(worktables[1].x, worktables[1].y));
        }
        while (pathPoints.size() > 0 && (Vector2D(x, y) - pathPoints[0]).length() < 0.4) {
            if (isGanking && pathPoints.size() == 1) break;
            pathPoints.erase(pathPoints.begin());
        }
        if (pathPoints.size() == 0) {
            TESTOUTPUT(fout << "forward " << id << " " << 0 << std::endl;)
            printf("forward %d %d\n", id, 0);
            TESTOUTPUT(fout << "rotate " << id << " " << 0 << std::endl;)
            printf("rotate %d %d\n", id, 0);
            return;
        }
        TESTOUTPUT(fout << "from" << "(" << x << ", " << y << ")" << "to" << "(" << pathPoints[0].x << ", " << pathPoints[0].y << ")" << std::endl;)
        std::vector<double> vec1 = {1, 0};
        std::vector<double> vec2 = {pathPoints[0].x - x, pathPoints[0].y - y};
        double cosAns = (vec1[0] * vec2[0] + vec1[1] * vec2[1]) / (sqrt(vec1[0] * vec1[0] + vec1[1] * vec1[1]) * sqrt(vec2[0] * vec2[0] + vec2[1] * vec2[1]));
        double angle = acos(cosAns);
        // 通过叉乘判断方向
        // > 0 逆时针 在上面
        // < 0 顺时针 在下面
        if (vec1[0] * vec2[1] - vec1[1] * vec2[0] < 0) {
            angle = -angle;
        }
        /*
            决定一下转向的大小
        */
        double rotate = 0;
        if (angle > direction) {
            if (std::abs(angle - direction) < M_PI) { // 如果角度差小于180°
                rotate = std::abs(angle - direction); // 就直接逆时针转过去
            } else { // 如果角度差大于180°
                rotate = -(M_PI*2-std::abs(angle - direction)); // 就顺时针转过去
            }
        } else { 
            if (std::abs(angle - direction) < M_PI) { // 如果角度差小于180°
                rotate = -(std::abs(angle - direction)); // 就直接顺时针转过去
            } else { // 如果角度差大于180°
                rotate = M_PI*2-std::abs(angle - direction); // 就逆时针转过去
            }
        }

        double absRotate = std::abs(rotate);
        double length = sqrt(vec2[0] * vec2[0] + vec2[1] * vec2[1]);
        /*
            决定一下速度
        */
        double speed = 0;
        // TESTOUTPUT(fout << "absRotate: " << absRotate << " length =" << length << " asin=" << asin(0.4 / length) << std::endl;)
        // 由于判定范围是 0.4m,所以如果度数满足这个条件, 就直接冲过去
        if (0.4 / length > 1) {
            speed = MAX_SPEED;
        } else
        if (absRotate < asin(0.4 / length)) {
            speed = MAX_SPEED;
        } else 
        if (absRotate < M_PI / 4) {
            // 如果度数大于x小于45° 
            // 如果转向时间的路程不会更长,就可以走
            if (absRotate > 0.0001)
                speed = length * 50 / (absRotate / 3.6 + 1);
            if (speed > MAX_SPEED) {
                speed = MAX_SPEED;
            }
        } else if (absRotate < M_PI / 2) {
            // 如果度数大于45°小于 90°
            speed = 0;
        } else {
            // 如果度数大于90°, 就先倒退转过去
            speed = -2;
        }
        if(isGanking == false) {
            if (RoB == RED) {
                if (length < (bringId != 0 ? 1.6 : 1.1) && speed > 0) {
                    speed = length / (bringId != 0 ? 1.6 : 1.1) * MAX_SPEED;
                    if (bringId != 0 && crashCoef > 0.91) { // 最多碰撞到 0.88
                        if (speed < 1) 
                            speed = 1;
                    } else {
                        if (speed < 3) 
                            speed = 3;
                    }
                }
            } else {
                if (length < (bringId == 0 ? 1.4 : 1.1) && speed > 0) {
                    speed = length / 1.1 * MAX_SPEED;
                    if (bringId != 0 && crashCoef > 0.81) { // 最多碰撞到 0.8
                        if (speed < 1) speed = 1;
                    } else {
                        if (speed < 3) speed = 3;
                    }
                }
            }
        }
        if (haveBeenGanked) speed = MAX_SPEED;
        TESTOUTPUT(fout << "forward " << id << " " << speed << std::endl;)
        printf("forward %d %lf\n", id, speed);
        /*
            由于转向决定的是速度,所以 1s 最多转3.6°
            如果转向大于3.6°, 就直接最大速度转向
            如果转向小于3.6°, 就按照比例转向
        */
        if (absRotate > 0.0001){
            if (absRotate < M_PI / 25) {
                double Ratio = absRotate / (M_PI / 25); 
                rotate = rotate / absRotate * M_PI * Ratio;
            } else {
                rotate = rotate / absRotate * M_PI;
            }
        }
        if (haveBeenGanked) rotate = M_PI;
        TESTOUTPUT(fout << "rotate " << id << " " << rotate << std::endl;)
        printf("rotate %d %lf\n", id, rotate);
    }
    // 如果没地方卖,就销毁
    int Destroy() {
        /**
         * 特殊情况，有个工作台快做完了，而且材料全了（没啥用，暂时弃用）
        */
        for (auto i : worktables) {
            if (i.remainTime > 0 && sellSet.find(std::make_pair(bringId, i.type)) != sellSet.end()) {
                int all = 0;
                int have = 0;
                for (int item = 1; item <= MAX_Item_Type_Num; item++) {
                    if (sellSet.find(std::make_pair(item, i.type)) != sellSet.end()) {
                        all++;
                        if (i.inputId[item] > 0) {
                            have++;
                        }
                    }
                }
                if (have == all) {
                    return i.id;
                }
            }
        }
        TESTOUTPUT(fout << "destroy " << id << std::endl;)
        printf("destroy %d\n", id);
        return -1;
    }
    void FindAPath() {
        std::vector<Path *> paths;
        std::vector<Path *> paths4567;
        for (auto & buy : worktables) {
            /**
             * 有产物, 没人预约
             * 有产物, 有人预约买,但是第二个在做或者做完阻塞了
             * 没产物, 没人预约买,在做了
            */
            if (buy.id == -1) break;
            bool couldReachFlag = false;
            for (auto & couldReachItem :couldReach) {
                if (couldReachItem == buy.id) {
                    couldReachFlag = true;
                    break;
                }
            }
            if (couldReachFlag == false) continue;
            if (buy.blocked == true) continue;
            if ((buy.output == true && buy.someWillBuy == 0)
                || (buy.output == true && buy.someWillBuy == 1 && buy.remainTime != -1)
                || (buy.output == false && buy.someWillBuy == 0 && buy.remainTime != -1)
                || (buy.type < 4)
            ) {} else continue;
            // 等待生产的时间
            double waitBuyTime = 0; 
            if ( (buy.someWillBuy == 1) || (buy.output == false && buy.someWillBuy == 0) ) waitBuyTime = buy.remainTime;
            // 路程时间消耗
            double goBuyTime;
            if (worktableId == -1) {
                goBuyTime = RobotToWT[id][buy.id];
            } else {
                goBuyTime = WTtoWT[worktableId][buy.id];
            }
            if (goBuyTime > 1e6) continue;
            goBuyTime = goBuyTime * 0.65 / (MAX_SPEED * 0.8 / 50);
            // 购买的产品
            int productId = createMap[buy.type];
            // 如果等待时间比路程时间长,就不用买了
            if (goBuyTime + ((productId == 7 && nowTime > MAX_TIME * 0.9) == true ? 30 : 0) < waitBuyTime) continue;
            for (auto & sell : worktables) {
                if (sell.id == -1) break;
                bool couldReachFlag = false;
                for (auto & couldReachItem :couldReach) {
                    if (couldReachItem == sell.id) {
                        couldReachFlag = true;
                        break;
                    }
                }
                if (couldReachFlag == false) continue;
                if (sell.blocked == true) continue;
                // 确保这个工作台支持买,而且输入口是空的
                if (sellSet.find(std::make_pair(productId, sell.type)) == sellSet.end() || sell.inputId[productId] == 1) continue;
                // 确保不是墙角
                if (sell.isNearCorner) continue;
                /**
                 * 确保没人预约卖
                 * 或者类型是8 || 9
                */
                if (sell.someWillSell[productId] == 0 
                    // 4 5 6有人预约卖,但是缺的这一个卖完就可以再卖
                    || (sell.someWillSell[productId] == 1 && sell.waitPriority == 4 && (sell.type == 4 || sell.type == 5 || sell.type == 6) 
                        && (sell.output == false || sell.remainTime == -1))
                    // 7有人预约卖,但是缺的这一个卖完就可以再卖
                    || (sell.someWillSell[productId] == 1 && sell.waitPriority == 5 && (sell.type == 7)
                        && (sell.output == false || sell.remainTime == -1)) 
                    // 8 9 直接卖
                    || sell.type == 8 || sell.type == 9) {} else continue;
                
                // 时间消耗
                double goSellTime = WTtoWTwithItem[buy.id][sell.id];
                if (goSellTime > 1e6) continue;
                goSellTime = goSellTime * 0.65 / (MAX_SPEED * 0.8 / 50);
                double sumTime = std::max(goBuyTime, waitBuyTime) + goSellTime;
                if (sumTime + 200 + nowTime > MAX_TIME) continue;
                // 时间损失
                double timeLoss;
                if (sumTime >= 9000) {
                    timeLoss = 0.8;
                } else {
                    timeLoss = 0.8 + (1-0.8) * (1 - sqrt(1 - (1 - sumTime / 9000) * (1 - sumTime / 9000)));
                }
                // 卖出产品赚取的钱
                double earnMoney = sellMoneyMap[productId] * timeLoss - buyMoneyMap[productId];
                // 尽量不卖给 9
                if (sell.type == 9) earnMoney = earnMoney * 0.6;
                earnMoney *= buy.near7;
                earnMoney *= sell.near7;
                if (sell.waitPriority == 5 && (EQUAL(sell.near7, 1, 1e-3)) == false) {
                    earnMoney *= 2;
                }
                if (sell.waitPriority == 4 && (EQUAL(sell.near7, 1, 1e-3)) == false && sell.type != 7 && sell.output == false && sell.remainTime == -1) {
                    earnMoney *= 2;
                }
                if (buy.isNearCorner) {
                    earnMoney *= 0.1;
                }
                // // 有资源缺口 即卖工作台的类型对应的产品(type 相同)有缺口 就促进生产
                // if (canBuy[sell.type] > 0 && (sell.type == 4 || sell.type == 5 || sell.type == 6) && (EQUAL(sell.near7, 1, 1e-3)) == false) {
                //     // TESTOUTPUT(fout << "canBuy " << sell.type << std::endl;)
                //     earnMoney *= 1.2;
                // }
                // 动态调度三种 456 的生产
                if (sell.near7 > 1 && (sell.type >= 4 && sell.type <= 6)
                    && haveCreateNum[4] >= haveCreateNum[sell.type] 
                    && haveCreateNum[5] >= haveCreateNum[sell.type] 
                    && haveCreateNum[6] >= haveCreateNum[sell.type]) {
                    earnMoney *= 2;
                }
                Path * path = new Path(buy.id, sell.id, id, earnMoney, sumTime);
                if ((productId == 4 || productId == 5 || productId == 6 || productId == 7) && ((buy.remainTime == 0 && buy.someWillBuy == 0) || (buy.remainTime < goBuyTime && buy.output == true && buy.someWillBuy == 0))) {
                    paths4567.push_back(path);
                }else {
                    paths.push_back(path);
                }
            }
        }
        // 理论上只有快结束才出现
        if (paths.size() == 0 && paths4567.size() == 0) {
            TESTOUTPUT(fout << "error" << std::endl;)
            if (couldReach.size() == 0) {
                worktableTogo = -1;
            } else 
            if (pathPoints.size() == 0) {
                worktableTogo = couldReach[rand() % couldReach.size()];
                pathPoints = movePath();
            }
            return;
        }
        std::sort(paths.begin(), paths.end(), [](Path * a, Path * b) {
            return a->parameters > b->parameters;
        });
        std::sort(paths4567.begin(), paths4567.end(), [](Path * a, Path * b) {
            return a->parameters > b->parameters;
        });
        if (paths4567.size() > 0 && (paths.size() == 0 || paths4567[0]->parameters > paths[0]->parameters * 0.8)) {
            path = paths4567[0];
        } else {
            path = paths[0];
        }
        TESTOUTPUT(fout << "robot" << id << " find path " << path->buyWorktableId << " " << path->sellWorktableId << std::endl;)
        worktableTogo = path->buyWorktableId;
        if (worktables[worktableTogo].near7 > 1 && (worktables[worktableTogo].type >= 4 && worktables[worktableTogo].type <= 6)) {
            haveCreateNum[worktables[worktableTogo].type]++;
        }
        pathPoints = movePath();
        worktables[path->buyWorktableId].someWillBuy++;
        worktables[path->sellWorktableId].someWillSell[createMap[worktables[path->buyWorktableId].type]]++;
    }
    double point_to_segment_distance(Vector2D begin, Vector2D end, Vector2D obstacle) {
        Vector2D begin_to_end = end-begin;
        Vector2D begin_to_obstacle = obstacle - begin;
        Vector2D end_to_obstacle = obstacle - end;
        if (begin_to_end * begin_to_obstacle <= 0) return (begin-obstacle).length();
        if (begin_to_end * end_to_obstacle >= 0) return (end-obstacle).length();
        return fabs(begin_to_end ^ begin_to_obstacle) / (begin-end).length();
    }
    std::vector<std::pair<Vector2D, Vector2D>> fixpath(std::vector<std::pair<Vector2D, Vector2D>> path, std::set<Vector2D> *blocked = nullptr) {
        // TESTOUTPUT(fout << path.size() << std::endl;)
        std::vector<std::pair<Vector2D, Vector2D>> ret;
        auto begin = path.begin();
        // 不断延迟线段的终点
        // 如果碰撞了,就不加这个点, 以最后一个点开始继续这个过程
        // 如果没有碰撞,就加上这个点的碰撞点
        while (begin != path.end()) {
            // TESTOUTPUT(fout << "线段从 " << begin->x << "," << begin->y << "开始" << std::endl;)
            auto end = begin;
            end++;
            while (end != path.end()) {
                end++;
                if (end == path.end()) break;
                bool flag = false;
                // TESTOUTPUT(fout << "测试到 " << end->x << "," << end->y << "是否碰撞" << std::endl;)
                for (double x = begin->first.x; flag == false ; x += (end->first.x - begin->first.x) / std::abs(end->first.x - begin->first.x) * 0.5) {
                    // TESTOUTPUT(fout << "x = " << x << std::endl;)
                    for (double y = begin->first.y; flag == false ; y += (end->first.y - begin->first.y) / std::abs(end->first.y - begin->first.y) * 0.5) {
                        for (auto & obstacle : grids[Vector2D(x, y)]->obstacles) {
                            // 计算obstacle到 begin->end这条线段的距离
                            double distance = point_to_segment_distance(begin->second, end->second, obstacle);
                            if (distance < (bringId == 0 ? 0.45 : 0.53)) {// 碰撞了
                                // TESTOUTPUT(fout << "碰撞点" << obstacle.x << "," << obstacle.y << std::endl;)
                                flag = true;
                                break;
                            }
                        }
                        if (blocked != nullptr)for (auto & obstacle : *blocked) {
                            // TESTOUTPUT(fout << "blocked" << obstacle.x << "," << obstacle.y << std::endl;)
                            double distance = point_to_segment_distance(begin->second, end->second, obstacle);
                            if (distance < 0.53 + 0.4) {// 碰撞了 0.53是机器人半径, 0.4是方格斜边的一半
                                flag = true;
                                break;
                            }
                        }
                        if (y == end->first.y) break;
                    }
                    if (x == end->first.x) break;
                }
                if (flag) {
                    // TESTOUTPUT(fout << "碰撞了" << std::endl;)
                    break;
                }
            }
            ret.push_back(*begin);
            end--; 
            if (end == begin) end++;
            begin = end;
        }
        ret.push_back(path.back());
        return ret;
    }
    std::vector<Vector2D> DodgingCorners(std::vector<Vector2D> path, std::set<Vector2D> *blocked = nullptr) {
        std::vector<std::pair<Vector2D, Vector2D>> solved;
        auto lastPoint = path.front();
        for (auto & point : path) {
            // 如果是起点,就不用处理, 精度更准
            if (point == path.front()) {
                Vector2D p1 = point;
                Vector2D p2 = Vector2D(x,y);
                solved.push_back(std::make_pair(p1, p2));
                continue;
            }
            Vector2D p1 = point;
            Vector2D p2 = point;
            if (bringId == 0) {
                int num = 0;
                for (auto & item : grids[p1]->obstacles) {
                    if ((item - p1).length() < 0.5) {
                        auto delta = p1 - item;
                        double ratio = 0.65 / 0.25;
                        // double ratio = 0.5 / delta.length();
                        // if (num == 0) {
                            p2 = item + delta * ratio;
                        //     num++;
                        // } else {
                            // p2 = p2 + delta * ratio;
                            break;
                        // }
                    }
                }
            } else {
                for (auto & item : grids[p1]->obstacles) {
                    if ((item - p1).length() < 0.6) {
                        auto delta = item - p1;
                        auto deltaLastToNow = p1 - lastPoint;
                        double ratio;
                        if ( !(lastPoint == p1) && deltaLastToNow.angle(delta) < M_PI / 2) {
                            ratio = 0.7;
                        } else {
                            ratio = 0.7;
                        }

                        if (grids[p1]->obstacles.size() <= 4) { // 最少 4 个
                            p2 = p2 - delta / delta.length() * 0.9;
                        } else {
                            p2 = p2 - delta / delta.length() * 0.6;
                        }
                        // 
                        // auto delta = p1 - item;
                        // double ratio = 0.55 / 0.25;
                        // p2 = item + delta * ratio;
                        // break;
                    }
                }
            }
            solved.push_back(std::make_pair(p1, p2));
            lastPoint = point;
        }
        solved = fixpath(solved, blocked);
        path.clear();
        for (auto & item : solved) {
            path.push_back(item.second);
        }
        CREATEMAP(mapOut << "time=" << nowTime << " robotId=" << id << " optimized carry=" << (bringId == 0 ? false : true) << std::endl;)
        for (auto & item : path) {
            CREATEMAP(mapOut << "(" << item.x << "," << item.y << ")" << "->";)
        }
        CREATEMAP(mapOut << std::endl;)
        return path;
    }
    /**
     * 计算路径
     * 计算从一个坐标移动到另一个坐标的路径
     * 通过 BFS 实现
     * 返回值应该是一个n个点的坐标的数组
    */
    std::vector<Vector2D> movePath(std::set<Vector2D> *blocked = nullptr){
        Vector2D to(worktables[worktableTogo].x, worktables[worktableTogo].y);
        std::vector<Vector2D> path;
        std::map<Vector2D, Vector2D> fromWhere;
        std::queue<Vector2D> q;
        double x = this->x;
        double y = this->y;
        // 计算当前位置的格子
        x = int(x / 0.5) * 0.5 + 0.25;
        y = int(y / 0.5) * 0.5 + 0.25;
        q.push(Vector2D(x, y));
        fromWhere.insert(std::make_pair(Vector2D(x, y), Vector2D(x, y)));
        // int findItme = 0;
        bool find = false;
        while (!q.empty() && find == false) {
            // findItme++;
            // 当前的位置
            Vector2D now = q.front();
            q.pop();
            // 八个方向的移动
            std::vector<std::pair<double, double>> adds = {{0, 0.5}, {0.5, 0}, {0, -0.5}, {-0.5, 0}, {0.5, 0.5}, {-0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}, {0, 0}};
            int nowCantGo = 0;
            std::vector<Vector2D> obstaclesNow;
            if (blocked != nullptr) for (auto & add2 : adds) {
                if (blocked->find(now + Vector2D(add2.first, add2.second)) != blocked->end()
                    || grids[now + Vector2D(add2.first, add2.second)]->type == 1) {
                    nowCantGo++;
                    obstaclesNow.push_back(now + Vector2D(add2.first, add2.second));
                }
            }
            for (auto &add : adds) {
                // 移动后的位置
                Vector2D next = now + Vector2D(add.first, add.second);
                if (next.x <= 0.25 || next.x >= 49.75 || next.y <= 0.25 || next.y >= 49.75) continue;
                // 没访问过
                if (fromWhere.find(next) != fromWhere.end()) continue;
                // 是墙
                if (grids[next]->type == 1) continue;
                if (blocked != nullptr) {
                    int cantGoFlag = 0;
                    std::vector<Vector2D> obstacles;
                    for (auto & add2 : adds) {
                        if (blocked->find(next + Vector2D(add2.first, add2.second)) != blocked->end()
                            || grids[next + Vector2D(add2.first, add2.second)]->type == 1) {
                            cantGoFlag++;
                            obstacles.push_back(next + Vector2D(add2.first, add2.second));
                        }
                    }
                    if (cantGoFlag > 2) continue;
                    if (cantGoFlag == 2 && (obstacles[0] - obstacles[1]).length() > 0.5) continue;
                    // 如果已经被卡了,只能往后走,不许往前走
                    if ((nowCantGo == 2 && (obstaclesNow[0] - obstaclesNow[1]).length() > 0.5) || nowCantGo > 2) {
                        if (Vector2D(add.first, add.second).angle(Vector2D(cos(direction), sin(direction))) < M_PI / 2) continue;
                    }
                }
                // 是其他机器人的位置
                // if (blocked != nullptr && blocked->find(next) != blocked->end()) continue;
                if (bringId == 0) {
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
        // TESTOUTPUT(fout << "find " << findItme << std::endl;)
        while ( 1 ) {
            path.push_back(to);
            if (to == fromWhere[to]) break;
            to = fromWhere[to];
        }
        std::reverse(path.begin(), path.end());
        CREATEMAP(mapOut << "time=" << nowTime << " robotId=" << id << " origin carry=" << (bringId == 0 ? false : true) << std::endl;)
        for (auto & item : path) {
            CREATEMAP(mapOut << "(" << item.x << "," << item.y << ")" << "->";)
        }
        CREATEMAP(mapOut << std::endl;)
        if (path.size() > 2) path = DodgingCorners(path, blocked);
        return path;
    }
    void checkDead();
    // 机器人具体的行为
    void action(){
        Sell();
        if (path == nullptr) {
            // if (nowTime > 6000 && id == 2 && RoB == BLUE) {
            //     isGankRobot = true;
            //     return;
            // }
            if (nowTime + 300 > MAX_TIME && RoB == BLUE) {
                isGankRobot = true;
                return;
            }
            FindAPath();
        }
        Buy();
        checkDead();
        TESTOUTPUT(fout << "robot" << id << " want-go " << worktableTogo << " type=" << worktables[worktableTogo].type << std::endl;)
    }
    void findNullPath(std::set<Vector2D> *cantGo, std::set<Vector2D> *blocked) {
        isWait = true;
        std::vector<Vector2D> path;
        std::map<Vector2D, Vector2D> fromWhere;
        std::queue<Vector2D> q;
        // 计算当前位置的格子
        double nowx = int(x / 0.5) * 0.5 + 0.25;
        double nowy = int(y / 0.5) * 0.5 + 0.25;
        q.push(Vector2D(nowx, nowy));
        fromWhere.insert(std::make_pair(Vector2D(nowx, nowy), Vector2D(nowx, nowy)));
        bool find = false;
        Vector2D target;
        while (!q.empty() && find == false) {
            // 当前的位置
            Vector2D now = q.front();
            q.pop();
            // 八个方向的移动
            std::vector<std::pair<double, double>> adds = {{0, 0.5}, {0.5, 0}, {0, -0.5}, {-0.5, 0}, {0.5, 0.5}, {-0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}, {0, 0}};
            int nowCantGo = 0;
            std::vector<Vector2D> obstaclesNow;
            for (auto & add2 : adds) {
                if (cantGo->find(now + Vector2D(add2.first, add2.second)) != cantGo->end()
                    || grids[now + Vector2D(add2.first, add2.second)]->type == 1) {
                    nowCantGo++;
                    obstaclesNow.push_back(now + Vector2D(add2.first, add2.second));
                }
            }
            for (auto &add : adds) {
                // 移动后的位置
                Vector2D next = now + Vector2D(add.first, add.second);
                if (next.x <= 0.25 || next.x >= 49.75 || next.y <= 0.25 || next.y >= 49.75) continue;
                // 没访问过
                if (fromWhere.find(next) != fromWhere.end()) continue;
                // 是墙
                if (grids[next]->type == 1) continue;
                // 是其他机器人的位置
                int cantGoFlag = 0;
                std::vector<Vector2D> obstacles;
                for (auto & add2 : adds) {
                    if (cantGo->find(next + Vector2D(add2.first, add2.second)) != cantGo->end()
                        || grids[next + Vector2D(add2.first, add2.second)]->type == 1) {
                        cantGoFlag++;
                        obstacles.push_back(next + Vector2D(add2.first, add2.second));
                    }
                }
                if (cantGoFlag > 2) continue;
                if (cantGoFlag == 2 && (obstacles[0] - obstacles[1]).length() > 0.5) continue;
                // 如果已经被卡了,只能往后走,不许往前走
                if ((nowCantGo == 2 && (obstaclesNow[0] - obstaclesNow[1]).length() > 0.5) || nowCantGo > 2) {
                    if (Vector2D(add.first, add.second).angle(Vector2D(cos(direction), sin(direction))) < M_PI / 2) continue;
                }
                // if (cantGo->find(next) != cantGo->end()) continue;
                if (bringId == 0) {
                    // 不携带物品
                    // 可以碰两个角
                    int num = 0;
                    std::vector<Vector2D> obstacles;
                    for (auto & item : grids[next]->obstacles) {
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
                    if (num == 2 && (obstacles[0]-obstacles[1]).length() > 0.5) continue;
                    // 在墙角斜着走
                    if (num == 2 && nowNum == 2 && add.first != 0 && add.second != 0) continue;
                } else {
                    // 携带物品
                    int num = 0;
                    Vector2D obstacles;
                    for (auto & item : grids[next]->obstacles) {
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
                bool isNull = true;
                for (auto &add : adds) {
                    if (grids[next + Vector2D(add.first, add.second)]->type == 0 
                        && blocked->find(next + Vector2D(add.first, add.second)) == blocked->end()) {
                        continue;
                    } else {
                        isNull = false;
                        break;
                    }
                }
                if (isNull) {
                    find = true;
                    target = next;
                }
            }
        }
        while ( 1 ) {
            path.push_back(target);
            if (target == fromWhere[target]) break;
            target = fromWhere[target];
        }
        std::reverse(path.begin(), path.end());
        CREATEMAP(mapOut << "time=" << nowTime << " robotId=" << id << " origin carry=" << (bringId == 0 ? false : true) << std::endl;)
        for (auto & item : path) {
            CREATEMAP(mapOut << "(" << item.x << "," << item.y << ")" << "->";)
        }
        CREATEMAP(mapOut << std::endl;)
        if (path.size() > 2) path = DodgingCorners(path);
        pathPoints = path;
    }
    void moveToPoint(Vector2D point){
        worktableTogo = worktableNum + 1 + id;
        worktables[worktableTogo].x = point.x;
        worktables[worktableTogo].y = point.y;
        // if (id == 2 && isGankRobot && (Vector2D(x, y) - point).length() < 1) {
        //     pathPoints.clear();
        //     pathPoints.push_back(point);
        //     return;
        // }
        // 还没到
        if ((Vector2D(x, y)-point).length() > 0.4) {
            // 路径正确,检查是否死锁
            if (pathPoints.size() > 0 && pathPoints.back() == point) {
                checkDead();
            } else {
                // 路径空的或者路径错误,重新规划
                pathPoints = movePath();
            }
        } else {
            // 到了
            pathPoints.push_back(point);
        }
    }
    void moveToFoeWT(int wtId) {
        moveToPoint(Vector2D(worktablesFoe[wtId].x, worktablesFoe[wtId].y));
        return;
    }
    void moveToWT(int wtId) {
        moveToPoint(Vector2D(worktables[wtId].x, worktables[wtId].y));
        return;
    }
    void buyOne(int wtId) {
        if (worktableId == wtId) {
            TESTOUTPUT(fout << "buy " << id << std::endl;)
            printf("buy %d\n", id);
            return;
        }
        moveToWT(wtId);
    }
};

Robot robots[MAX_Robot_Num];
Robot robotsFoe[MAX_Robot_Num];

std::map<Vector2D, double> *getPathLabel(std::vector<Vector2D> path, int id) {
    std::vector<std::pair<double, double>> adds = {{0, 0.5}, {0.5, 0}, {0, -0.5}, {-0.5, 0}, {0.5, 0.5}, {-0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}, {0, 0}};
    path.insert(path.begin(), Vector2D(robots[id].x, robots[id].y));
    std::map<Vector2D, double> *pathPoints = new std::map<Vector2D, double>();
    double sum = 0;
    for (auto from = path.begin(); from != path.end(); from++) {
        auto next = from + 1;
        if (next == path.end()) break;
        auto vec = *next - *from; // 方向向量
        auto aStep = vec / vec.length() * 0.25; // 一次移动0.25 的距离
        for (auto nextVec = *from; (nextVec - *next).length() > 0.25; nextVec = nextVec + aStep) {
            double nowx = int(nextVec.x / 0.5) * 0.5 + 0.25;
            double nowy = int(nextVec.y / 0.5) * 0.5 + 0.25;
            for (auto & add : adds) {
                pathPoints->insert(std::make_pair(Vector2D(nowx + add.first, nowy + add.second), sum));
            }
            sum += 0.25;
            if (sum > TOL_Collision) break;
        }
        if (sum > TOL_Collision) break;
    }
    double nowx = int(path.rbegin()->x / 0.5) * 0.5 + 0.25;
    double nowy = int(path.rbegin()->y / 0.5) * 0.5 + 0.25;
    for (auto & add : adds) {
        pathPoints->insert(std::make_pair(Vector2D(nowx + add.first, nowy + add.second), sum));
    }
    return pathPoints;
}

void DetecteCollision(int robot1, int robot2, std::set<Vector2D> *robot1PathPointsAll, std::set<Vector2D> *robot1PointsAll){
    if (robots[robot1].worktableTogo == -1 || robots[robot2].worktableTogo == -1) return;
    if (robots[robot1].pathPoints.size() != 0 && robots[robot1].pathPoints[0] == Vector2D(0,0)) return;
    if (robots[robot2].pathPoints.size() != 0 && robots[robot2].pathPoints[0] == Vector2D(0,0)) return;
    if ((Vector2D(robots[robot1].x, robots[robot1].y) - Vector2D(robots[robot2].x, robots[robot2].y)).length() > 6) return;
    // 如果两个机器人卡死了
    bool isRobotStop = false;
    if ((Vector2D(robots[robot1].x, robots[robot1].y) - Vector2D(robots[robot2].x, robots[robot2].y)).length() 
        < (robots[robot1].bringId == 0 ? 0.45 : 0.53) + (robots[robot2].bringId == 0 ? 0.45 : 0.53) + 0.1 
        && Vector2D(robots[robot1].linearSpeedX, robots[robot1].linearSpeedY).length() < 0.1
        && Vector2D(robots[robot2].linearSpeedX, robots[robot2].linearSpeedY).length() < 0.1) {
        TESTOUTPUT(fout << "robot" << robot1 << " and robot" << robot2 << " 卡死了" << std::endl;)
        isRobotStop = true;
        // std::swap(robot1, robot2);
    }
    // 给 robot1/robot2 的路径打标签
    std::map<Vector2D, double> *robot1PathPointsMap = getPathLabel(robots[robot1].pathPoints, robot1);
    std::map<Vector2D, double> *robot2PathPointsMap = getPathLabel(robots[robot2].pathPoints, robot2);
    int isCollision = 0;
    double minCollisionLength2on1 = 1e9;
    double minCollisionLength1on2 = 1e9;
    // 检测 robot1 的路径上是否有 robot2 的位置
    for (auto & item : *robot2PathPointsMap) {
        if (robot1PathPointsMap->find(item.first) != robot1PathPointsMap->end()) {
            // 发生碰撞
            isCollision++;
            minCollisionLength2on1 = std::min(minCollisionLength2on1, item.second);
        }
    }
    // 检测碰撞点最近的距离
    for (auto & item : *robot1PathPointsMap) {
        if (robot2PathPointsMap->find(item.first) != robot2PathPointsMap->end()) {
            minCollisionLength1on2 = std::min(minCollisionLength1on2, item.second);
        }
    }
    if (isCollision < 3) return;
    // 追逐情况
    if (RoB == RED) {
        if (minCollisionLength2on1 > 2 || minCollisionLength1on2 > 2) return;    
    } else {
        if (minCollisionLength2on1 > 1.6 || minCollisionLength1on2 > 1.6) return;    
    }
    // if (std::abs(minCollisionLength2on1 - minCollisionLength1on2) > 0.8) return;

    std::set<Vector2D> *robot1PathPoints = new std::set<Vector2D>();
    std::set<Vector2D> *robot2PathPoints = new std::set<Vector2D>();
    for (auto & item : *robot1PathPointsMap) robot1PathPoints->insert(item.first);
    for (auto & item : *robot2PathPointsMap) robot2PathPoints->insert(item.first);

    TESTOUTPUT(fout << "robot" << robot1 << " and robot" << robot2 << " 检测碰撞" << std::endl;)
    std::vector<Vector2D> erasedPoints, erasedPoints2;
    // // 在robot1的路径上删除 robot2 开始的位置
    {
        double nowx = int(robots[robot2].x / 0.5) * 0.5 + 0.25;
        double nowy = int(robots[robot2].y / 0.5) * 0.5 + 0.25;
        std::vector<std::pair<double, double>> adds = {{0, 0.5}, {0.5, 0}, {0, -0.5}, {-0.5, 0}, {0.5, 0.5}, {-0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}, {0, 0}};
        for (auto & add : adds) {
            if (robot1PathPoints->find(Vector2D(nowx + add.first, nowy + add.second)) != robot1PathPoints->end()) {
                robot1PathPoints->erase(Vector2D(nowx + add.first, nowy + add.second));
                erasedPoints.push_back(Vector2D(nowx + add.first, nowy + add.second));
            }
            for (auto & add2 : adds) {
                if (robot1PathPoints->find(Vector2D(nowx + add.first + add2.first, nowy + add.second + add2.second)) != robot1PathPoints->end()) {
                    robot1PathPoints->erase(Vector2D(nowx + add.first + add2.first, nowy + add.second + add2.second));
                    erasedPoints.push_back(Vector2D(nowx + add.first + add2.first, nowy + add.second + add2.second));
                }
            }
        }
    }
    // // 在robot2的路径上删除 robot1 开始的位置
    {
        double nowx = int(robots[robot1].x / 0.5) * 0.5 + 0.25;
        double nowy = int(robots[robot1].y / 0.5) * 0.5 + 0.25;
        std::vector<std::pair<double, double>> adds = {{0, 0.5}, {0.5, 0}, {0, -0.5}, {-0.5, 0}, {0.5, 0.5}, {-0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}, {0, 0}};
        for (auto & add : adds) {
            if (robot2PathPoints->find(Vector2D(nowx + add.first, nowy + add.second)) != robot2PathPoints->end()) {
                robot2PathPoints->erase(Vector2D(nowx + add.first, nowy + add.second));
                erasedPoints2.push_back(Vector2D(nowx + add.first, nowy + add.second));
            }
            for (auto & add2 : adds) {
                if (robot2PathPoints->find(Vector2D(nowx + add.first + add2.first, nowy + add.second + add2.second)) != robot2PathPoints->end()) {
                    robot2PathPoints->erase(Vector2D(nowx + add.first + add2.first, nowy + add.second + add2.second));
                    erasedPoints2.push_back(Vector2D(nowx + add.first + add2.first, nowy + add.second + add2.second));
                }
            }
        }
    }
    std::set<Vector2D> *robot1Points = new std::set<Vector2D>();
    std::set<Vector2D> *robot2Points = new std::set<Vector2D>();
    // // 记录 robot1 现在的位置
    // {
    //     double nowx = int(robots[robot1].x / 0.5) * 0.5 + 0.25;
    //     double nowy = int(robots[robot1].y / 0.5) * 0.5 + 0.25;
    //     std::vector<std::pair<double, double>> adds = {{0, 0.5}, {0.5, 0}, {0, -0.5}, {-0.5, 0}, {0.5, 0.5}, {-0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}, {0, 0}};
    //     for (auto & add : adds) {
    //         robot1Points->insert(Vector2D(nowx + add.first, nowy + add.second));
    //     }
    // }
    // // 记录 robot2 现在的位置
    // {
    //     double nowx = int(robots[robot2].x / 0.5) * 0.5 + 0.25;
    //     double nowy = int(robots[robot2].y / 0.5) * 0.5 + 0.25;
    //     std::vector<std::pair<double, double>> adds = {{0, 0.5}, {0.5, 0}, {0, -0.5}, {-0.5, 0}, {0.5, 0.5}, {-0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}, {0, 0}};
    //     for (auto & add : adds) {
    //         robot2Points->insert(Vector2D(nowx + add.first, nowy + add.second));
    //     }
    // }
    // 把 robot1Points 中的点加入到 robot1PointsAll 中
    // {
    //     if (robot1PointsAll->size() == 0)
    //     for (auto & item : *robot1Points) {
    //         robot1PointsAll->insert(item);
    //     }
    // }
    // 把 robot1Points 中的点加入到 robot1PathPoints 中
    // 为了防止 robot1 起始点被删除了,两方过于相近,走了不该走的路
    // {
    //     for (auto & item : *robot1Points) {
    //         robot1PathPoints->insert(item);
    //     }
    // }
    // // 把 robot2Points 中的点加入到 robot2PathPoints 中
    // // 为了防止 robot2 起始点被删除了,两方过于相近,走了不该走的路
    // {
    //     for (auto & item : *robot2Points) {
    //         robot2PathPoints->insert(item);
    //     }
    // }
    // 把 robot1PathPoints 中的点加入到 robot1PathPointsAll 中
    // {
    //     for (auto & item : *robot1PathPoints) {
    //         robot1PathPointsAll->insert(item);
    //     }
    // }
    // 重新规划路径
    robots[robot2].pathPoints = robots[robot2].movePath(robot1PathPoints); 
    if (robots[robot2].pathPoints[0] == Vector2D(0,0)) {
        TESTOUTPUT(fout << "robot" << robot2 << " 无法规划路径" << std::endl;)
        for (auto & item : erasedPoints) {
            robot1PathPointsAll->insert(item);
        }
        for (auto & item : erasedPoints2) {
            robot2PathPoints->insert(item);
        }
        robots[robot2].findNullPath(robot1PointsAll, robot1PathPointsAll);
        if (robots[robot2].pathPoints[0] == Vector2D(0,0)) {
            TESTOUTPUT(fout << "robot" << robot2 << " 无法找到空路径" << std::endl;)
            robots[robot1].findNullPath(robot2Points, robot2PathPoints);
            if (robots[robot1].pathPoints[0] == Vector2D(0,0)) {
                TESTOUTPUT(fout << "robot" << robot1 << " 也无法找到空路径" << std::endl;)
                robots[robot1].pathPoints = robots[robot1].movePath();
                robots[robot2].pathPoints.clear();
                robots[robot2].pathPoints.push_back(robots[robot1].pathPoints.back());
            } else {
                robots[robot2].pathPoints = robots[robot2].movePath();
            }
        }
    } else {
        TESTOUTPUT(fout << "robot" << robot2 << " 已重新规划路径" << std::endl;)
    }
}

void checkDestory(int id) {
    if (robots[id].willDestroy == true) {
        robots[id].willDestroy = false;
        printf("destroy %d\n", id);
        TESTOUTPUT(fout << "destroy " << id << std::endl;)
    }
}
void delayDestroy(int id) {
    robots[id].willDestroy = true;
}
void refindsell(int wtId, int id) {
    int newWorktable = -1;
    auto productId = robots[id].bringId;
    // 找一个新的工作台去卖
    for (auto & couldReachItem : robots[id].couldReach) {
        auto & sell = worktables[couldReachItem];
        if (sell.blocked == true) continue;
        // 确保这个工作台支持买,而且输入口是空的
        if (sellSet.find(std::make_pair(productId, sell.type)) == sellSet.end() || sell.inputId[productId] == 1) continue;
        // 确保不是墙角
        if (sell.isNearCorner) continue;
        /**
         * 确保没人预约卖
         * 或者类型是8 || 9
        */
        if (sell.someWillSell[productId] == 0 
            // 4 5 6有人预约卖,但是缺的这一个卖完就可以再卖
            || (sell.someWillSell[productId] == 1 && sell.waitPriority == 4 && (sell.type == 4 || sell.type == 5 || sell.type == 6) 
                && (sell.output == false || sell.remainTime == -1))
            // 7有人预约卖,但是缺的这一个卖完就可以再卖
            || (sell.someWillSell[productId] == 1 && sell.waitPriority == 5 && (sell.type == 7)
                && (sell.output == false || sell.remainTime == -1)) 
            // 8 9 直接卖
            || sell.type == 8 || sell.type == 9) {} else continue;      
        newWorktable = couldReachItem;
        break;        
    }
    if (newWorktable != -1) {
        robots[id].worktableTogo = newWorktable;
        robots[id].path->sellWorktableId = newWorktable;
        worktables[wtId].someWillSell[productId]--;
        worktables[newWorktable].someWillSell[productId]++;
        robots[id].pathPoints = robots[id].movePath();
        return;
    }
    // 找不到则销毁 重新规划路径
    delayDestroy(id);
    robots[id].path = nullptr;
    robots[id].bringId = 0;
    worktables[wtId].someWillSell[robots[id].bringId]--;
    robots[id].worktableTogo = -1;
}
void Robot::checkDead(){
    if (std::abs(Vector2D(linearSpeedX,linearSpeedY).length()) < 0.0001) {
        zeroTime++;
    } else {
        zeroTime = 0;
    }
    if (zeroTime > 10) {
        TESTOUTPUT(fout << "robotDead " << id << " " << nowTime << std::endl;)
        pathPoints = movePath();
        isWait = false;
    }
    if (zeroTime == 0) {
        if (nowTime % 100 == id * 10) {
            pathPoints = movePath();
            // if (path != nullptr && worktableTogo == path->sellWorktableId && pathPoints.size() <= 2) {
            //     TESTOUTPUT(fout << "robot" << id << " 无法规划路径" << std::endl;)
            //     // refindsell(worktableTogo, id);
            //     TESTOUTPUT(fout << "robot" << id << " 重新规划路径" << std::endl;)
            //     return;
            // }
        }
    }
    if (pathPoints.size() != 0 && pathPoints[0] == Vector2D(0,0) && Vector2D(linearSpeedX,linearSpeedY).length() < 1) {
        haveBeenGanked = true;
    } else {
        haveBeenGanked = false;
    }
}

#endif