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
    
    bool havePath;
    std::deque<Pos> wholePath; // 机器人的路径，front为当前位置
    Pos tarItemPos;

    Robot() {
        this->id = -1;
    }
    Robot(int id, int x, int y) {
        this->id = id;
        this->pos = Pos(x, y);
        this->status = 1;
        this->bring = 0;
        this->choosed_berth_id = -1;
        this->havePath = false;
        this->wholePath.clear();
        this->lastWeak = -1;
        this->lastWeakPos = Pos(-1, -1);
        this->tarItemPos = Pos(-1, -1);
    }
    void action(); // 计算机器人的路径和生成行动
    void move(); // 生成机器人的移动指令
    void checkCollision(std::unordered_map<Pos, Pos> &otherPos);
    std::deque<Pos> actionFindBerth(Pos beginPos, int beginFrame); // 机器人在当前pos找港口或给定item的pos找港口
    std::deque<Pos> actionFindItem(); // 机器人在当前的pos下寻找一个item
};


// beginPos是起始位置如果起始位置没有给定，则说明起始位置是机器人，否则就是item，需要从item开始求港口距离
// beginFrame是起始时间，如果起始时间是0，则说明从机器人位置开始，否则从item开始，所有的dis都需要加上item
// 返回值是机器人的路径，无论是从机器人开始还是从item开始
// 如果已经找到了一段item的路径，但没有找到合适的港口，机器人会停在原地
std::deque<Pos> Robot::actionFindBerth(Pos beginPos=Pos(-1, -1), int beginFrame=0) {
    
    if (beginPos != Pos(-1, -1)) {
        solveGridWithTime(beginPos, id, beginFrame);
        // 从item开始求到港口的距离，在这种情况下，dis和path都是从item开始的
    }
    // 但是港口距离的计算和第一段到item的距离无关
    int minDis = 0x3f3f3f3f;
    if (choosed_berth_id != -1) {
        Berth* now_berth = berths[choosed_berth_id];
        Pos choosed_berth_pos = Pos(-1,-1);
        
        for(Pos sub_berths : now_berth->usePos){
            if(disWithTime[sub_berths.x][sub_berths.y] == 0x3f3f3f3f) continue;
            if (disWithTime[sub_berths.x][sub_berths.y]< minDis){
                minDis = disWithTime[sub_berths.x][sub_berths.y];
                choosed_berth_pos = sub_berths;
            }   
        }
        if (minDis != 0x3f3f3f3f) {
            beginPos = beginPos == Pos(-1, -1) ? pos : beginPos;

            auto tarPath = findPathWithTime(beginPos, choosed_berth_pos);
            if (tarPath.size() > 0) {
                // berth_center->declare_robot_choose_berth(choosed_berth_id);
                return tarPath;
            } else {
                pathLogger.log(nowTime, "rid={},toBerth={},noPath", id, choosed_berth_id);
            }
        }
    }
    if (minDis == 0x3f3f3f3f) {
        auto robot_berths = berth_center->get_robot_berth(id);
        for (auto & berth : robot_berths) {
            if (disWithTime[berths[berth]->pos.x][berths[berth]->pos.y] != 0x3f3f3f3f) {
                choosed_berth_id = berth;
                break;
            }
        }
        if (choosed_berth_id == -1) {
            for (int i = 0; i < MAX_Berth_Num; i++) {
                if (disWithTime[berths[i]->pos.x][berths[i]->pos.y] != 0x3f3f3f3f) {
                    choosed_berth_id = i;
                    break;
                }
            }
        }
        if (choosed_berth_id != -1) {
            Berth* now_berth = berths[choosed_berth_id];
            Pos choosed_berth_pos = Pos(-1,-1);
            int minDis = 0x3f3f3f3f;
            for(Pos sub_berths : now_berth->usePos){
                if(disWithTime[sub_berths.x][sub_berths.y] == 0x3f3f3f3f) continue;
                if (disWithTime[sub_berths.x][sub_berths.y]< minDis){
                    minDis = disWithTime[sub_berths.x][sub_berths.y];
                    choosed_berth_pos = sub_berths;
                }   
            }
            if (minDis != 0x3f3f3f3f) {
                //注意到这里真可能不可达
                auto tarPath = findPathWithTime(pos, choosed_berth_pos);
                if (tarPath.size() > 0) {
                    return tarPath;
                }
            }
        }
    }

    return std::deque<Pos>();
}

// 无论如何，调用这个函数一定是因为机器人没有货物
std::deque<Pos> Robot::actionFindItem() {
        
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
            auto tempValue = double(i->value) / (toItemDis + toBertDis) ;
            if (tempValue > value){
                minDis = toItemDis;
                targetItem = i;
                value = tempValue;
                berth_select = berth;
            }
            i++;
        }
    }
    if (minDis == 0x3f3f3f3f) {
        // 如果没有选择港口,那么先退化到shc方案
        for(auto i = unsolvedItems.begin(); i != unsolvedItems.end();) {
            if (i->checkDead()) {
                unsolvedItems.erase(i++);
                continue;
            }
            auto toItemDis = disWithTime[i->pos.x][i->pos.y];
            // auto toBertDis = berths[choosed_berth_id]->disWithTime[i->pos.x][i->pos.y];
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
            auto tempValue = double(i->value) / (toItemDis * 2.0) ;
            if (tempValue > value){
                minDis = toItemDis;
                targetItem = i;
                value = tempValue;
            }
            i++;
        }
    }

    if (minDis != 0x3f3f3f3f && targetItem != unsolvedItems.end()) {
        int targetItemIndex = std::distance(unsolvedItems.begin(), targetItem);
        // flowLogger.log(nowTime, "rid={},toItem={}", id, targetItemIndex);

        auto tarPath = findPathWithTime(pos, targetItem->pos);

        if (tarPath.size() > 0) {
            bringTimeLimit = targetItem->beginTime + Item_Continue_Time;
            item_value = targetItem->value;
            unsolvedItems.erase(targetItem);
            choosed_berth_id = berth_select;
            /*
            havePath = true;
            addPathToAllPath(wholePath, id);
            */
            counter.max_put("robot_move_length_max", tarPath.size());
            counter.min_put("robot_move_length_min", tarPath.size());
            counter.push_back("robot_move_length_vector", tarPath.size());
            return tarPath;
        } else {
            pathLogger.log(nowTime, "rid={},toItem={},noPath", id, targetItemIndex);
        }
    }

    return std::deque<Pos>();
}


void Robot::action() {
    flowLogger.log(nowTime, "action {0}", id);
    // 如果机器人的路径首位不等于当前位置，则需要重新计算路径。
    if (wholePath.size() == 0 || pos != wholePath.front()) {
        havePath = false;
        wholePath.clear();
        deletePathFromAllPath(id);
    }
    // 如果找到了目标item
    if (pos == tarItemPos) {
        // 如果确实没拿东西，就拿一下
        if (bring == 0 && nowTime <= bringTimeLimit) {
            printf("get %d\n", id);
            flowLogger.log(nowTime, "get {0}", id);
            counter.add("robot_get_nums", 1);
            counter.add("robot_get_value", item_value);
            // TODO: 需要检测一下是否真的拿到东西了？？？在前面判断一下，如果没有拿到，需要重新计算
            bring = 1;
        } 
        // 然后判断是否需要去港口，为1说明不需要去港口，鉴于已经走到了，清空路径，否则继续走
        if (wholePath.size() == 1) {
            havePath = false;
            wholePath.clear();
            deletePathFromAllPath(id);
        }
    }
    // ==1 表示走到港口了或者找到Item了，但是找到Item已经在上一条语句中判断过了，如果是找到Item了，那就不会==1
    if (wholePath.size() == 1) {
        // 机器人到达目的地
        if (bring == 1 && grids[pos.x][pos.y]->type == 3) {
            printf("pull %d\n", id);
            flowLogger.log(nowTime, "pull {0}", id);
            bring = 0;
            if (choosed_berth_id != -1) {
                berth_center->declare_robot_pull_good(choosed_berth_id, item_value);
            }
        }

        havePath = false;
        wholePath.clear();
        deletePathFromAllPath(id);
    }

    // 没走到，正常走过程中
    if (wholePath.size() > 1) return;

    // 能运行到这里，说明机器人已经没有路径了，可能是因为碰撞引起的重新寻路，可能是走到了需要找新的路径
    // 如果是重新寻路，则需要判断应该去港口还是去找新的物体
    // 但是无论如何，都要先从机器人为起点进行一次寻路

    // 找当前机器人在不碰撞前提下，从自己出发的所有路，路长存在disWithTime，前序点存在preWithTime
    // auto beginSolveTime = high_resolution_clock::now();
    solveGridWithTime(pos, id);
    // auto endSolveTime = high_resolution_clock::now();
    // pathLogger.log(nowTime, "rId={0}solveGridWithTime time={1}", id, duration_cast<microseconds>(endSolveTime - beginSolveTime).count());
    // 拿着东西找港口，直接用前面计算好的dis就行
    if (bring == 1) {
        auto berthPath = actionFindBerth();
        // 可能为空
        if (berthPath.size() > 0) {
            wholePath = berthPath;
            havePath = true;
            addPathToAllPath(wholePath, id);
            // pathLogger.log(nowTime, "id={},berthTar=({},{}),pathSize={}", id, berthPath.back().x, berthPath.back().y, wholePath.size());

        }
    }

    // 没拿东西找东西，先计算item的dis，然后传入item的坐标，再计算港口的dis
    if (bring == 0) {
        auto itemPath = actionFindItem();
        // 可能为空，如果为空，也不用去找港口的路
        if (itemPath.size() > 0) {
            // 起始点：当前点，目标点：item的位置
            // 起始时间：0，到item的时间为：itemPath.size() - 1
            wholePath = itemPath;
            havePath = true;
            addPathToAllPath(wholePath, id);
            auto itemPos = itemPath.back();
            auto itemTime = itemPath.size() - 1;
            tarItemPos = itemPos;
            auto berthPath = actionFindBerth(itemPos, itemTime);
            // 起始点：item的位置，目标点：港口的位置
            // 起始时间：itemTime，到港口的时间为：itemTime + berthPath.size() - 1
            // 可能为空
            pathLogger.log(nowTime, "id={},itemTar=({},{}),pathSize={}", id, itemPos.x, itemPos.y, wholePath.size());

            if (berthPath.size() > 0) {
                //注意，由于上一个path的终点是item的位置，这里的起点也是item位置，所以不需要再计算一次，应该把item pop出去
                berthPath.pop_front();
                wholePath.insert(wholePath.end(), berthPath.begin(), berthPath.end());
                havePath = true;
                // 直接用wholePath，就不需要考虑从哪个时间加入了
                addPathToAllPath(wholePath, id);
                pathLogger.log(nowTime, "id={},itemTar=({},{}),berthTar=({},{}),pathSize={}", id, itemPos.x, itemPos.y, berthPath.back().x, berthPath.back().y, wholePath.size());
            }
        }
    } 
    // TODO: 没位置去的机器人别堵路啊，能不能死一死（？
    if (wholePath.size() == 0) {
        robotLogger.log(nowTime, "noPath id={},bring={},havePath={},pathSize={}", id, bring, havePath, wholePath.size());
        havePath = false;
        updateFixPos(pos, id);
    }
}

void Robot::move() {
    // 如果机器人被撞到了
    if (!status) {
        status = 1;
        return;
    }
    // 只要路径小于2，就说明无论如何不用走
    if (wholePath.size() < 2) return;
    // robotLogger.log(nowTime, "id={},bring={},havePath={},pathSize={},from=({},{}),to=({},{}),status={}", id, bring, havePath, wholePath.size(), wholePath.front().x, wholePath.front().y, wholePath.back().x, wholePath.back().y, status);
    auto nowPos = wholePath.front();
    auto nextPos = wholePath.at(1);
    int nextDir = getDirWithPath(nowPos, nextPos);
    // 把当前位置弹出
    wholePath.pop_front();
    printf("move %d %d\n", id , nextDir);
    counter.add("robot_move_length", 1);

    flowLogger.log(nowTime, "move {0} {1}", id, nextDir);
}


// first 表示机器人的目标位置, second 表示机器人原始位置
void Robot::checkCollision(std::unordered_map<Pos, Pos> &otherPos){
    Pos nextTimePos;
    int nextDir = -1;
    // 首先预处理自己之后几帧的位置
    if (!status || wholePath.size() < 2 || pos == wholePath.back()) {
        nextTimePos = pos;
    } else {
        nextTimePos = wholePath.at(1);
        nextDir = getDirWithPath(pos, nextTimePos);
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
        if (otherPos.find(nextTimePos) != otherPos.end() && otherPos.find(nextTimePos)->first == otherPos.find(nextTimePos)->second) {
            // 这是一个弱智的情况,有机器人停下来了,那我需要下一帧继续绕开走
            lastWeak = nowTime;
            lastWeakPos = pos;
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
        // 如果可以停留在原地,有点弱智这里??这里应该是直接可以留下来
        if (otherPos.find(pos) == otherPos.end()) {
            status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
            otherPos[pos] = pos;
            return;
        }
    } else { // 如果下一帧的位置没有机器人
        // 特殊情况:如果上一帧遇到了弱智,而且这一帧的下一步是回到弱智面前,就不要走了,换一个方向.
        if (!status || wholePath.size() < 2 || pos == wholePath.back()) {}
        else if (lastWeak == nowTime - 1 && nextTimePos == lastWeakPos) {
            int minLength = 1e9; int minDir = -1;
            // 遍历其他三个方向,选择一个最近的方向
            for (int d = 0; d < 4; d++) if (d != nextDir) {
                auto nextPos = pos + dir[d];
                auto length = wholePath.back().length(nextPos);
                // 判断越界和障碍物
                if (checkRobotAble(nextPos) == false) continue;
                // 判断是否有机器人
                if (otherPos.find(nextPos) != otherPos.end()) continue;
                // 判断是否重叠
                if (otherPos.find(pos) != otherPos.end() && otherPos.find(pos)->second == nextPos) continue;
                if (length < minLength) {
                    minLength = length;
                    minDir = d;
                } else if (length == minLength) {
                    if (rand() % 2 == 0) minDir = d;
                }
            }
            if (minDir != -1) {
                printf("move %d %d\n", id, minDir);
                status = 0; // 假装被撞了 不会触发 move 下一帧的输入会改回正常
                otherPos[pos + dir[minDir]] = pos;
                flowLogger.log(nowTime, "move {0} {1}", id, minDir);
                return;
            }
        }
        otherPos[nextTimePos] = pos;
    }
    return;
}
std::vector<Robot *> robots;

void solveCollision() {
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
                return robots[ida]->pos.y < robots[idb]->pos.y;
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
            for (auto &frd : friends[top]) {
                if (visited[frd] == false && topPos.length(robots[frd]->pos) == 1) {
                    q.push(frd);
                    visited[frd] = true;
                }
            }
            for (auto &frd : friends[top]) {
                if (visited[frd] == false && topPos.length(robots[frd]->pos) == 2) {
                    q.push(frd);
                    visited[frd] = true;
                }
            }
        }
    }
    // 按组进行碰撞检测
    std::unordered_map<Pos, Pos> nextPos; nextPos.clear();
    for (auto & g : group) {
        for (auto & i : g.second) {
            robots[i]->checkCollision(nextPos);
        }
    }
    return;
}


void newRobot(int x, int y) {
    printf("lbot %d %d\n", x, y);
    robots.push_back(new Robot(MAX_Robot_Num++, x, y));
    fixPos.emplace_back(Pos(-1, -1));
    money -= 2000;
}

#endif