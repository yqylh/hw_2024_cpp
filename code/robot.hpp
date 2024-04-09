#ifndef __REBOT_H__
#define __REBOT_H__
#pragma GCC optimize(3)

#include "config.hpp"
#include "grid.hpp"
#include "item.hpp"
#include "ship.hpp"
#include "berth.hpp"
#include "berth_centre.hpp"
#include "path.hpp"

struct Robot{
    int id; // 机器人的 id
    Pos pos; // 机器人的位置

    int status; // 机器人的状态 0表示寄了 1 表示正常运行 用来控制避障
    int bring; // 机器人带的货物 0 表示没有货物 1 表示有货物
    int bringTimeLimit; // 机器人到达带货物的时间
    int choosed_berth_id; // 机器人选择的泊位
    int item_value; // 机器人选择的物品的价值

    int lastWeak; // 上一次的弱智时间
    Pos lastWeakPos; // 上一次的弱智位置
    
    Navigator *navigator; // 导航器
    Pos tarItemPos;
    Pos tarBerthPos;
    Navigator *navigator_berth; // 去港口的导航器

    Robot() {
        this->id = -1;
    }
    Robot(int id, int x, int y) {
        this->id = id;
        this->pos = Pos(x, y);
        this->status = 1;
        this->bring = 0;
        this->choosed_berth_id = -1;
        this->lastWeak = -1;
        this->lastWeakPos = Pos(-1, -1);
        this->tarItemPos = Pos(-1, -1);
        this->tarBerthPos = Pos(-1, -1);
        this->navigator = nullptr;
        this->navigator_berth = nullptr;
        this->bringTimeLimit = 0;
        this->item_value = 0;
    }
    void clear() {
        this->status = 1;
        this->bring = 0;
        this->choosed_berth_id = -1;
        this->tarItemPos = Pos(-1, -1);
        this->tarBerthPos = Pos(-1, -1);
        this->navigator = nullptr;
        this->navigator_berth = nullptr;
        this->bringTimeLimit = 0;
        this->item_value = 0;
    }
    void action(); // 计算机器人的路径和生成行动
    void move(); // 生成机器人的移动指令
    void checkCollision(std::unordered_map<Pos, Pos> &otherPos);
    std::deque<Pos> actionFindBerth(Pos beginPos, int beginFrame); // 机器人在当前pos找港口或给定item的pos找港口
    bool actionFindItem(); // 机器人在当前的pos下寻找一个item
};

// 无论如何，调用这个函数一定是因为机器人没有货物
bool Robot::actionFindItem() {
    double value = 0;
    int minDis = 0x3f3f3f3f;
    auto berth_select = -1;
    auto targetItem = unsolvedItems.end();
    auto robot_berths = berth_center->get_robot_berth(id);
    for (auto & berth : robot_berths) {
        for (auto i = unsolvedItems.begin(); i != unsolvedItems.end();) {
            if (i->checkDead()) {
                unsolvedItems.erase(i++);
                continue;
            }
            auto toItemDis = disWithTime[i->pos.x][i->pos.y];
            // 到港口的是预测时间
            auto toBertDis = berths[berth]->disWithTimeBerth[i->pos.x][i->pos.y];
            // 判断是否可达
            if (toItemDis == 0x3f3f3f3f) {
                i++;
                continue;
            }
            // 判断是否超时
            if (nowTime + toItemDis + 1 > i->beginTime + Item_Continue_Time) {
                i++;
                continue;
            }
            //考虑时间价值 *(1 + (1000 + i->beginTime - nowTime) / 2000.0)
            double time_eff = std::exp((nowTime - i->beginTime) / 500.0);
            auto tempValue = double(i->value) * time_eff / (toItemDis  + toBertDis) ;
            if (tempValue > value){
                minDis = toItemDis;
                targetItem = i;
                value = tempValue;
                berth_select = berth;
            }
            i++;
        }
    }
    // 机器人没有可以到达的港口,或者没有货物
    if (minDis == 0x3f3f3f3f) return false;
    // 更新状态
    bringTimeLimit = targetItem->beginTime + Item_Continue_Time;
    item_value = targetItem->value;
    choosed_berth_id = berth_select;
    tarItemPos = targetItem->pos;
    int randUsePos = rand() % 2;
    tarBerthPos = berths[berth_select]->usePos[randUsePos];
    navigator = sovleGrid(tarItemPos);
    navigator_berth = berths[berth_select]->usePosNavigator[randUsePos];
    // 删除物品
    unsolvedItems.erase(targetItem);
    return true;
}


void Robot::action() {
    flowLogger.log(nowTime, "action {0}", id);
    if (bring == 0 && nowTime > bringTimeLimit) clear();
    // 目标item 且 没拿物品
    if (pos == tarItemPos && bring == 0) {
        // 在这里一定满足 nowTime <= bringTimeLimit. 因为已经判断了 nowTime > bringTimeLimit && bring == 0 的情况
        printf("get %d\n", id);
        flowLogger.log(nowTime, "get {0}", id);
        counter.push_back("robot_pos",-1000,-2);
        counter.push_back("robot_pos",id,2);
        counter.add("robot_get_nums", 1);
        counter.add("robot_get_value", item_value);
        bring = 1;
        // 去港口的导航器
        navigator = navigator_berth;
    }
    // 如果带着物品到达了港口
    if (pos == tarBerthPos && bring == 1 && grids[pos.x][pos.y]->type == 3) {
        printf("pull %d\n", id);
        flowLogger.log(nowTime, "pull {0}", id);
        counter.push_back("robot_pos",-1000,-2);
        counter.push_back("robot_pos",id,1);
        berth_center->declare_robot_pull_good(choosed_berth_id, item_value);
        clear();
    }

    // 对于所有的拿/送的状态,已经判断完毕了,下面要判断的是,如果没有路径
    // 只有两种情况:
    // 1. 机器人刚出生
    // 2. 机器人前面触发了 clear
    // 2.1 拿不到物品了
    // 2.2 放下物品了
    if (navigator == nullptr) {
        solveGridWithTime(pos, id);
        actionFindItem();
        robotLogger.log(nowTime, "robot{0} find item value{1} pos{2},{3} berth{4} pos{5},{6}", id, item_value, tarItemPos.x, tarItemPos.y, choosed_berth_id, tarBerthPos.x, tarBerthPos.y);
    }
}

void Robot::move() {
    // 如果机器人被撞到了
    if (!status) {
        status = 1;
        return;
    }
    if (navigator == nullptr) return;
    printf("move %d %d\n", id , navigator->getDir(pos.x, pos.y));
    flowLogger.log(nowTime, "move {0} {1}", id, navigator->getDir(pos.x, pos.y));
}


// first 表示机器人的目标位置, second 表示机器人原始位置
void Robot::checkCollision(std::unordered_map<Pos, Pos> &otherPos){
    Pos nextTimePos;
    int nextDir = -1;
    // 首先预处理自己之后几帧的位置
    if (!status || navigator == nullptr) {
        nextTimePos = pos;
    } else {
        nextDir = navigator->getDir(pos.x, pos.y);
        nextTimePos = pos + dir[nextDir];
    }
    // 如果存在互换的情况
    if (checkRobotNoColl(nextTimePos) && checkRobotNoColl(pos)) {
        otherPos[nextTimePos] = pos;
        return;
    }
    // robotLogger.log(nowTime, "robot{0} pos{1},{2} nextTimePos{3},{4} nextDir{5}", id, pos.x, pos.y, nextTimePos.x, nextTimePos.y, nextDir);
    // 如果下一帧的位置有机器人 或者有两个机器人交换位置
    if (otherPos.find(nextTimePos) != otherPos.end() || (otherPos.find(pos) != otherPos.end() && otherPos.find(pos)->second == nextTimePos) ){
        // robotLogger.log(nowTime, "robot{0} crash", id);
        // 不能继续走同样的方向,尽量不被追着揍
        std::vector<int> ableDir;
        if (nextDir == 0) ableDir = {2, 3, 1}; 
        if (nextDir == 1) ableDir = {2, 3, 0}; 
        if (nextDir == 2) ableDir = {0, 1, 3}; 
        if (nextDir == 3) ableDir = {0, 1, 2}; 
        if (nextDir == -1) ableDir = {0, 1, 2, 3};
        while (rand() % 2) std::swap(ableDir[rand() % ableDir.size()], ableDir[rand() % ableDir.size()]);
        // if (otherPos.find(nextTimePos) != otherPos.end() && otherPos.find(nextTimePos)->first == otherPos.find(nextTimePos)->second) {
        //     // 这是一个弱智的情况,有机器人停下来了,那我需要下一帧继续绕开走
        //     lastWeak = nowTime;
        //     lastWeakPos = pos;
        // }
        // 如果不能移动解决,那停留在原地
        if (otherPos.find(pos) == otherPos.end() || checkRobotNoColl(pos)) {
            status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
            otherPos[pos] = pos;
            return;
        }
        for (auto & d : ableDir) { 
            auto nextPos = pos + dir[d];
            if (checkRobotAble(nextPos) == false) continue;
            if (otherPos.find(nextPos) != otherPos.end() 
                || (otherPos.find(pos) != otherPos.end() && otherPos.find(pos)->second == nextPos) ) {
                continue;
            }
            printf("move %d %d\n", id, d);
            flowLogger.log(nowTime, "move {0} {1}", id, d);
            status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
            otherPos[nextPos] = pos;
            return;
        }
    } else { // 如果下一帧的位置没有机器人
        // 特殊情况:如果上一帧遇到了弱智,而且这一帧的下一步是回到弱智面前,就不要走了,换一个方向.
        // if (!status || navigator == nullptr) {}
        // else if (lastWeak == nowTime - 1 && nextTimePos == lastWeakPos) {
        //     int minDir = -1;
        //     // 遍历其他三个方向,选择一个最近的方向
        //     for (int d = 0; d < 4; d++) if (d != nextDir) {
        //         auto nextPos = pos + dir[d];
        //         // 判断越界和障碍物
        //         if (checkRobotAble(nextPos) == false) continue;
        //         // 判断是否有机器人
        //         if (otherPos.find(nextPos) != otherPos.end()) continue;
        //         // 判断是否重叠
        //         if (otherPos.find(pos) != otherPos.end() && otherPos.find(pos)->second == nextPos) continue;
        //         if (minDir == -1 || rand() % 2 == 0) minDir = d;
        //     }
        //     if (minDir != -1) {
        //         printf("move %d %d\n", id, minDir);
        //         status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
        //         otherPos[pos + dir[minDir]] = pos;
        //         flowLogger.log(nowTime, "move {0} {1}", id, minDir);
        //         return;
        //     }
        // }
        otherPos[nextTimePos] = pos;
    }
    return;
}
std::vector<Robot *> robots;


void solveRobotPriority() {
    // 记录 friends
    std::vector<std::vector<int>> friends(MAX_Robot_Num, std::vector<int>(0));
    std::unordered_set<Pos> robotPos;
    // 计算每个节点的 friend 的 id
    for (int i = 0; i < MAX_Robot_Num; i++) {
        robotPos.insert(robots[i]->pos);
        for (int j = i + 1; j < MAX_Robot_Num; j++) {
            if (robots[i]->pos.length(robots[j]->pos) <= 2) {
                friends[i].push_back(j);
                friends[j].push_back(i);
            }
        }
    }
    // 记录周围的空闲度
    std::vector<int> free(MAX_Robot_Num, 0);
    for (int i = 0; i < MAX_Robot_Num; i++) {
        for (int j = 0; j < 4; j++) {
            auto nextPos = robots[i]->pos + dir[j];
            if (checkRobotAble(nextPos) == false) continue;
            if (robotPos.find(nextPos) != robotPos.end()) continue;
            free[i]++;
        }
    }
    // 使用Lambda表达式作为比较函数
    auto compare = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        if (a.second == b.second) {
            int ida = a.first, idb = b.first;
            if (robots[ida]->pos.x == robots[idb]->pos.x) {
                if (robots[ida]->pos.y == robots[idb]->pos.y) {
                    return ida < idb;
                } else {
                    return robots[ida]->pos.y < robots[idb]->pos.y;
                }
            } else {
                return robots[ida]->pos.x < robots[idb]->pos.x;
            }
        } else {
            return a.second < b.second;
        }
    };
    // 初始化set，指定Lambda表达式作为比较器
    std::set<std::pair<int, int>, decltype(compare)> mySet(compare);
    // 按照 free 排序    id   free. 如果 free 相同按照 x y 排序
    for (int i = 0; i < MAX_Robot_Num; i++) {
        mySet.insert(std::make_pair(i, free[i]));
    }
    /**
     * 1. 首先找到没计算的点中空闲度最小的点
     * 2. 然后找到它的邻居 优先距离 1,其次距离 2,然后将其删除
     * 3. 重复 1 2 步骤
    */
    std::unordered_map<int, std::vector<int>> group; group.clear();
    std::vector<bool> visited(MAX_Robot_Num, false);
    while (!mySet.empty()) {
        auto it = mySet.begin();
        int u = it->first;
        mySet.erase(it);
        group[u] = std::vector<int>();
        std::queue<int> q;
        q.push(u);
        visited[u] = true;
        while (!q.empty()) {
            auto top = q.front();
            q.pop();
            group[u].push_back(top);
            for (auto & item : mySet) if (item.first == top) {
                mySet.erase(item);
                break;
            }
            Pos topPos = robots[top]->pos;
            for (int l = 0; l <= 2; l++) {
                for (auto &frd : friends[top]) {
                    if (visited[frd] == false && topPos.length(robots[frd]->pos) == l) {
                        q.push(frd);
                        visited[frd] = true;
                    }
                }
            }
        }
    }
    // 按组进行碰撞检测
    for (auto & g : group) {
        for (auto & i : g.second) {
            robotPriority.push_back(i);
        }
    }
    priorityTimeControl = nowTime + 30;
}

void solveCollision() {
    if (priorityTimeControl < nowTime || robotPriority.size() != robots.size()){
        robotPriority.clear();
        solveRobotPriority();
    }
    std::unordered_map<Pos, Pos> nextPos; nextPos.clear();
    for (auto & i : robotPriority) {
        robots[i]->checkCollision(nextPos);
    }
}


void newRobot(int x, int y) {
    printf("lbot %d %d\n", x, y);
    robots.push_back(new Robot(MAX_Robot_Num++, x, y));
    money -= 2000;
    berth_center->robot_pos.emplace_back(x, y);
}

#endif