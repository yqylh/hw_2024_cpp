#ifndef __BERTH_CENTRE_H__
#define __BERTH_CENTRE_H__
#include "config.hpp"
#include "grid.hpp"
#include "ship.hpp"
#include "berth.hpp"
// #include "robot.hpp"
#include <vector>

/**
 * @brief 机器人和船只的购买位置
*/
struct RobotBuyer {
    Pos pos;
    RobotBuyer(Pos _pos) : pos(_pos) {}
};
struct ShipBuyer {
    Pos pos;
    ShipBuyer(Pos _pos) : pos(_pos) {}
};

/**
 * @brief 交货点
*/
struct Delivery {
    Pos pos;
    Delivery(Pos _pos) : pos(_pos) {}
};

/**
 * Berth_center即船坞、轮船控制中心（塔台），进行统一调配，并指引机器人进入泊位
*/
class Berth_center {
public:
    std::vector<RobotBuyer> robot_buyer;
    std::vector<ShipBuyer> ship_buyer;
    std::vector<Delivery> delivery;
    std::vector<std::vector<int> > r_buyer_choose_berth; //机器人销售点选择的泊位
    std::vector<int> group_sorted_id; // 初始的分组排序
    std::vector<std::vector<int> > group;
    Berth_center() {
        group_sorted_id.clear();
    }    

    // 用于指引船只进入最佳的泊位
    int ship_choose_berth() {
        int max_goods = -1;
        int max_goods_id = -1;
        for (int i = 0; i < MAX_Berth_Num; i++) {
            if (berths[i]->shipId.empty() == false) continue;
            if (berths[i]->sum_value > 0 && berths[i]->sum_value > max_goods) {
                max_goods = berths[i]->sum_value;
                max_goods_id = i;
            }
        }
        if (max_goods_id == -1) {
            TEST(fout << "ship_choose_berth error" << std::endl;)
            // throw;
        }
        return max_goods_id;
    }
    // 用于指引机器人进入最佳的泊位
    std::vector<int> get_robot_berth(int id) {
        // if (nowTime + Only_Run_On_Berth_with_Ship > MAX_TIME) {
        //     std::vector<int> ret;
        //     for (int i = 0; i < MAX_Berth_Num; i++) {
        //         if (berths[i]->shipId.size() != 0) ret.push_back(i);
        //     }
        //     if (ret.size() != 0) return ret;
        //     else return r_buyer_choose_berth[id];
        // } else {
        //     return r_buyer_choose_berth[id];
        // }
        std::vector<int> ret;
        for (int i = 0; i < MAX_Berth_Num; i++) ret.push_back(i);
        return ret;
    }
    // 机器人告知塔台卸货
    void declare_robot_pull_good(int bert_id, int item_value){
        berths[bert_id]->goodsNum++;
        tmpTotalGoods++;
        berths[bert_id]->item_value_queue.push(item_value);
        berths[bert_id]->sum_value += item_value;
        berths[bert_id]->total_goods++;
        berths[bert_id]->total_value += item_value;
    }
    // 船只告知塔台进入泊位
    void declare_ship(int bert_id,int ship_id){
        berths[bert_id]->shipId.push_back(ship_id);
    }
    // 如果船在港口而且没装满，把货物运到船上
    void bert_ship_goods_check(int bert_id){
        if (!berths[bert_id]->shipId.empty() && ships[berths[bert_id]->shipId[0]]->status == 1) {
            if (berths[bert_id]->goodsNum > 0){
                int loaded_goods = berths[bert_id]->goodsNum;
                if (loaded_goods > berths[bert_id]->velocity) loaded_goods = berths[bert_id]->velocity;
                ships[berths[bert_id]->shipId[0]]->capacity += loaded_goods;
                berths[bert_id]->goodsNum -= loaded_goods;
                while (loaded_goods--) {
                    berths[bert_id]->sum_value -= berths[bert_id]->item_value_queue.front();
                    berths[bert_id]->item_value_queue.pop();
                }
            }
        }
    }
    // 检查港口的状态
    void normal_berth_check(int bert_id){
        auto berth_ptr = berths[bert_id];
        // 因为先移动,所以先检查船的状态
        if (!berth_ptr->shipId.empty()) {
            auto ship_ptr = ships[berth_ptr->shipId[0]];
            if (ship_ptr->status != 1) return;
            // 让船去虚拟点的几种情况
            // line1: 如果船只装满了
            // line2: 或者是最后一轮了
            // line3: 如果港口没货物了, 并且船装满了百分之 ratio
            if (   ship_ptr->leftCapacity() == 0
                || berth_ptr->time + nowTime + 2 > MAX_TIME
                || (berth_ptr->goodsNum == 0 && ship_ptr->capacity > MAX_Capacity * Sell_Ration && nowTime + berth_ptr->time * 2 + lastRoundRuningTime < MAX_TIME)
            ) {
                berth_ptr->shipId.clear();
                ship_ptr->goSell(delivery[0].pos);
                return;
            }
            // 让船去别的地方的情况
            // 港口没货了,并且船没装满Sell_Ration
            // 但是去了之后不能超时
            if (berth_ptr->goodsNum == 0 && berth_ptr->time + nowTime + 10 + 500 < MAX_TIME) {
                int best_bert_id = ship_choose_berth();
                if (best_bert_id == -1) return;
                if (berths[best_bert_id]->sum_value < Min_Next_Berth_Value) return;
                berth_ptr->shipId.clear();
                declare_ship(best_bert_id, ship_ptr->id);
                ship_ptr->moveToBerth(best_bert_id, berths[best_bert_id]->pos);
                shipLogger.log(nowTime, "centre command ship{0} move_berth to berth{1}", ship_ptr->id, best_bert_id);
                return;
            }
        }
    }
    // 用来查找每个机器人私有的区域
    void find_private_space() {
        // 对所有的泊位进行分组
        std::vector<int> is_grouped(MAX_Berth_Num, -1);
        group = std::vector<std::vector<int> >(MAX_Berth_Num, std::vector<int>());
        r_buyer_choose_berth = std::vector<std::vector<int> >(robot_buyer.size(), std::vector<int>());
        for (int i = 0; i < MAX_Berth_Num; i++) {
            if (is_grouped[i] != -1) continue;
            is_grouped[i] = i;
            group[i].push_back(i);
            group_sorted_id.push_back(i);
            for (int j = i + 1; j < MAX_Berth_Num; j++) {
                if (is_grouped[j] != -1) continue;
                // 如果两个泊位之间的距离小于 MAX_Berth_Control_Length / 2,那么就是一个组
                if (berths[i]->disWithTimeBerth[berths[j]->pos.x][berths[j]->pos.y] < MAX_Berth_Merge_Length) {
                    group[i].push_back(j);
                    is_grouped[j] = i;
                }
            }
        }
        // 输出一下分组信息
        for (auto & i : group_sorted_id) {
            // 选择分组内最优的泊位
            std::sort(group[i].begin(), group[i].end(), [](const int& a, const int& b) {
                // MAX_Capacity / berths[i]->velocity + berths[i]->time
                return MAX_Capacity / berths[a]->velocity + berths[a]->time < MAX_Capacity / berths[b]->velocity + berths[b]->time;
            });
            centerLogger.log(nowTime, "group{}:", i);
            for (int j = 0; j < group[i].size(); j++) {
                centerLogger.log(nowTime, "    {}", group[i][j]);
            }
        }
        // 每个组所拥有的私有区域的面积
        std::vector<std::vector<int> > berth_onwer_space(MAX_Berth_Num, std::vector<int>());
        // 空地大小
        int ground_num = 0;
        for (int x = 0; x < MAX_Line_Length; x++) {
            for (int y = 0; y < MAX_Col_Length; y++) {
                // 排除障碍物和海洋
                if (robot_grids[x][y]->type == -1) continue;
                ground_num++;
                std::set<int> owner;
                // 我们只考虑 MAX_Berth_Control_Length 帧内的情况
                int min_num = MAX_Berth_Control_Length;
                for (int i = 0; i < MAX_Berth_Num; i++) {
                    if (berths[i]->disWithTimeBerth[x][y] < min_num) {
                        min_num = berths[i]->disWithTimeBerth[x][y];
                        owner.clear();
                        owner.insert(is_grouped[i]);
                    } else if (berths[i]->disWithTimeBerth[x][y] == min_num) {
                        owner.insert(is_grouped[i]);
                    }
                }
                for (auto &i : owner) berth_onwer_space[i].push_back(min_num);
            }
        }
        // 每个组的平均私有区域长度
        double avg_onwer_space_length[MAX_Berth_Num];
        for (auto & i : group_sorted_id) {
            double sum = 0;
            for (auto &len : berth_onwer_space[i]) sum += len;
            avg_onwer_space_length[i] = sum / berth_onwer_space[i].size();
        }
        // 按照私有区域的大小排序
        // 可选参数有 
        // berth_onwer_space[a].size() 越大越好
        // avg_onwer_space_length[a] 越小越好
        // berth_onwer_space[a].size() / avg_onwer_space_length[a] 越大越好
        std::sort(group_sorted_id.begin(), group_sorted_id.end(), [&](const int& a, const int& b) {
            return berth_onwer_space[a].size() / avg_onwer_space_length[a] > berth_onwer_space[b].size() / avg_onwer_space_length[b];
        });
        for (auto & i : group_sorted_id) {
            centerLogger.log(nowTime, "berth group{}, onwer_space{}, avg_onwer_space_length{}, 参数{}", i, berth_onwer_space[i].size(), avg_onwer_space_length[i], berth_onwer_space[i].size() / avg_onwer_space_length[i]);
        }
        // 需要考虑的: 组可以接触到哪些机器人购买点(一个购买点暂定一个组) 组是否太烂了
        std::vector<std::vector<int> > group_can_reach_robot(MAX_Berth_Num, std::vector<int>());
        std::vector<bool> r_buyer_selected(robot_buyer.size(), false);
        bool need_select_worst = false;
        while (true) {
            bool flag = false;
            // 按照优先级,每个组选择一个最近的机器人购买点
            for (auto & i : group_sorted_id) {
                // 如果这个组的评分*3小于最好的组的评分,那么就不考虑这个组
                if (berth_onwer_space[i].size() / avg_onwer_space_length[i] * Worst_Rate < berth_onwer_space[group_sorted_id.front()].size() / avg_onwer_space_length[group_sorted_id.front()] && need_select_worst == false) continue;
                int min_dis = INT_MAX;
                int min_r_buyer = -1;
                for (int r_buyer = 0; r_buyer < robot_buyer.size(); r_buyer++) {
                    if (r_buyer_selected[r_buyer]) continue;
                    for (auto & berth_id : group[i]) {
                        if (berths[berth_id]->disWithTimeBerth[robot_buyer[r_buyer].pos.x][robot_buyer[r_buyer].pos.y] < min_dis) {
                            min_dis = berths[berth_id]->disWithTimeBerth[robot_buyer[r_buyer].pos.x][robot_buyer[r_buyer].pos.y];
                            min_r_buyer = r_buyer;
                        }
                    }
                }
                if (min_dis != INT_MAX) {
                    group_can_reach_robot[i].push_back(min_r_buyer);
                    r_buyer_selected[min_r_buyer] = true;
                    flag = true;
                }
            }
            centerLogger.log(nowTime, "flag:{}", flag);
            if (!flag) {
                // 如果在need_select_worst状态下仍然没更新,那么就退出
                if (need_select_worst) break;
                // 如果一个机器人购买点没有被选择,那说明 1. 没有组可以 reach 2. 组太烂了被 skip 了 组可能有多个组
                bool isEnd = true;
                for (int i = 0; i < robot_buyer.size(); i++) if (!r_buyer_selected[i]) isEnd = false;
                // 如果所有的机器人购买点都被选择了,那么就退出. 不然设定 need_select_worst = true 继续跑
                if (isEnd) break;
                else need_select_worst = true;
            }
        }
        for (auto & i : group_sorted_id) {
            centerLogger.log(nowTime, "group{} can reach robot:", i);
            for (auto & j : group_can_reach_robot[i]) {
                centerLogger.log(nowTime, "    {}", j);
                for (auto & berth_id : group[i]) r_buyer_choose_berth[j].push_back(berth_id);
            }
        }
    }
    void finish_log();
    void normal_check_ship(int shipId){
        shipLogger.log(nowTime, "ship{} status:{} pos:{},{} targetPos:{},{} berthId:{}",
            ships[shipId]->id, ships[shipId]->status, ships[shipId]->pos.x, ships[shipId]->pos.y, ships[shipId]->targetPos.x, ships[shipId]->targetPos.y, ships[shipId]->berthId);
        // 到达了销售点卖掉了. 或者船刚出生
        if ((ships[shipId]->berthId == -2) || (ships[shipId]->pos == ships[shipId]->targetPos && ships[shipId]->berthId == -1)) {
            int best_bert_id = ship_choose_berth();
            if (best_bert_id == -1) {
                if (ships[shipId]->berthId == -2) {
                    best_bert_id = 0;
                } else {
                    return;
                }
            }
            declare_ship(best_bert_id, shipId);
            ships[shipId]->moveToBerth(best_bert_id, berths[best_bert_id]->pos);
            return;
        }
        // 到达了目标泊位, 当前位置是靠泊区, 并且船是运行状态
        if (ships[shipId]->berthId != -1 && ships[shipId]->status == 0 && boat_grids[ships[shipId]->pos.x][ships[shipId]->pos.y]->Grid::type == 8) {
            ships[shipId]->berth();
            return;
        }
    }
    // 每一轮都要执行的检查状态
    void call_ship_and_berth_check(){
        if (nowTime == 1) {
            newShip(ship_buyer[0].pos.x, ship_buyer[0].pos.y);
            return;
        }
        for(int i = 0; i < MAX_Berth_Num; i++){
            /* 检查港口火舞,有货物就卸货,不然考虑卸货问题*/
            normal_berth_check(i);
            // 卸货
            bert_ship_goods_check(i);
        }
        // 判断船是否需要前往港口,如果需要就前往
        for (int i = 0; i < MAX_Ship_Num; i++) {
            bcenterlogger.log(nowTime, "ship{} check", i);
            normal_check_ship(i);
            bcenterlogger.log(nowTime, "ship{} check end", i);
        }
        if (nowTime == 15000) finish_log();
    }
};

Berth_center *berth_center = new Berth_center();

// 最后一回合的 统计信息
void Berth_center::finish_log() {
    int leftTotal = 0;
    for (int i = 0; i < MAX_Berth_Num; i++) {
        berthLogger.log(nowTime, "berth{},goodsNum:{}", i, berths[i]->sum_value);
        leftTotal += berths[i]->sum_value;
    }
    berthLogger.log(nowTime, "leftTotal:{}", leftTotal);
    // berthLogger.log(nowTime, "tmpTotalGoods:{}", tmpTotalGoods + leftTotal);
}
#endif