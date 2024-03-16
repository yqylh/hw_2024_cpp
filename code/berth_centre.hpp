#ifndef __BERTH_CENTRE_H__
#define __BERTH_CENTRE_H__
#include "config.hpp"
#include "grid.hpp"
#include "ship.hpp"
#include "berth.hpp"
// #include "robot.hpp"
#include <vector>

/*
使用说明：
Berth_center 即控制塔台，用于调送货轮，指引机器人进入泊位
函数说明：
void init_doing() 初始化，指引船只进入泊位，在初始化时使用
void get_berth_want_goods_level() 计算每个泊位的需求等级，内部使用
int chose_bertch(int robot_id) 用于指引机器人来到最佳的泊位 
    使用方法 bert_id1，pos_id1,bert_id2,pos_id2 = chose_bertch(robot_id)
void pull_good(int bert_id) 装载货物时通知塔台
void declare_ship(int bert_id) 通知塔台有船要进入泊位
void declare_robot(int bert_id) 通知塔台有机器人要进入泊位
*/


class Berth_center {
public:
    bool is_init = false;

    void call_ship_and_berth_check(){
        /* 检查船坞\船的当前容量,满则发船 */
        normal_berth_check(); //船坞检查 不用外置循环

        for(int i = 0; i < MAX_Ship_Num; i++){
            if(allships[i] ->berthId != -1){
                if (allships[i] -> is_last_round) continue;
                if (allberths[allships[i]->berthId]->time + nowTime + 20 > MAX_TIME){
                    allships[i]->go(allships[i]->berthId);
                    allships[i] -> is_last_round = true;
                    bcenterlogger.log(nowTime, "ship{0} go last round", i);
                }
            }
            normal_ship_check(i);
        }
        bcenterlogger.log(nowTime, "ship_check ok");
    }

    std::vector<int> call_robot_choose_berth(){
        /* 用于指引机器人来到最佳的泊位,暂时只返回船坞的需求值,需要机器人加上自身距离再做判断 */
        return sortted_berth_want_goods_level;
    }

    void declare_robot_choose_berth(int bert_id){
        /* 机器人告知塔台将前往那个船坞 */
        declare_robot(bert_id);
    }

    void declare_robot_pull_good(int bert_id){
        /* 机器人告知塔台卸货 */
        bcenterlogger.log(nowTime, "declare_robot_pull_good");
        pull_good(bert_id);
    }

    void do_first_frame(){
        /* 初始化，指引船只进入泊位 */
        init_doing();
        first_frame_doing();
        is_init = true;
    }


private:
    /*Berth_center即船坞、轮船控制中心（塔台），进行统一调配，并指引机器人进入泊位*/
    Berth **allberths = berths;
    Ship **allships = ships;
    float berth_want_goods_level[MAX_Berth_Num]; //表示正常货物需求等级 0-?越低越急
    float berth_want_ship_level[MAX_Berth_Num]; //表示正常船只需求等级 0-?越低越急


    int bert_velocitys[MAX_Berth_Num];
    int bert_load_start_times[MAX_Berth_Num];
    int bert_load_finish_times[MAX_Berth_Num]; //运输到虚拟点的剩余时间
    int bert_times[MAX_Berth_Num]; //运输到虚拟点的时间
    int bert_fix_times[MAX_Berth_Num]; //运输到虚拟点的 “修正时间”（即运输到虚拟点的时间+船只装货时间）

    std::vector<int> sortted_bert_by_time = std::vector<int>(MAX_Berth_Num);
    std::vector<int> sortted_bert_by_velocity = std::vector<int>(MAX_Berth_Num);
    std::vector<int> sortted_bert_fix_times = std::vector<int>(MAX_Berth_Num);
    std::vector<int> sortted_berth_want_goods_level = std::vector<int>(MAX_Berth_Num);

    void init_doing(){
        /*
        todo: 这里应该有些不必要的变量，记得优化掉
        */
        //初始化各种参数
        for (int i = 0; i < MAX_Berth_Num; i++){
            berth_want_goods_level[i] = 15000.0;
            berth_want_ship_level[i] = 15000.0;
            bert_load_finish_times[i] = 0;
            bert_load_start_times[i] = 0;

            sortted_bert_by_time[i] = i;
            sortted_bert_by_velocity[i] = i;
            sortted_bert_fix_times[i] = i;
            sortted_berth_want_goods_level[i] = i;
        }
        for (int i = 0; i < MAX_Berth_Num; i++){
            bert_velocitys[i] = allberths[i]->velocity;
            bert_times[i] = allberths[i]->time;
            bert_fix_times[i] = bert_times[i] + bert_velocitys[i] * MAX_Capacity; //运输到虚拟点的时间+船只装货时间
        }
        std::sort(sortted_bert_by_time.begin(), sortted_bert_by_time.end(), [&](const int& a, const int& b) {
            return bert_times[a] < bert_times[b]; // 按时间升序排列
        });
        std::sort(sortted_bert_by_velocity.begin(), sortted_bert_by_velocity.end(), [&](const int& a, const int& b) {
            return bert_velocitys[a] < bert_velocitys[b]; // 按装填速度升序排列
        });
        std::sort(sortted_bert_fix_times.begin(), sortted_bert_fix_times.end(), [&](const int& a, const int& b) {
            return bert_fix_times[a] < bert_fix_times[b]; // 按修正时间升序排列
        });
        bcenterlogger.log(nowTime, "init_done");
    }
    
    void first_frame_doing(){
        //指引初始状态的船只进入泊位，能不能放到init_doing里？
        for (int i = 0; i < MAX_Ship_Num; i++){
            allships[i]->go_berth(sortted_bert_by_time[i]); // 指引船只进入泊位, 优先级按时间升序排列
            declare_ship(sortted_bert_by_time[i], i);
            allberths[sortted_bert_by_time[i]]->on_way_ship ++;
        }
    }

    void cal_berth_want_goods_level(){
        //按泊位多，船少计算，计算每个泊位对机器人的需求等级
        for (int i = 0; i < MAX_Berth_Num; i++){
            if (allberths[i]->waitting_ship != 0 && //有船(可能不止一只)
            allberths[i]->goodsNum + allships[allberths[i]->shipId[0]]->capacity < MAX_Capacity * allberths[i]->waitting_ship //这船(可能不止一只)还装不满
            ){
                berth_want_goods_level [i] = 0; //急需货物，快来！
                continue;
            }
            float remaining_shipment = (allberths[i]->goodsNum + allberths[i]->on_way_robot) / float(MAX_Capacity); //剩余运输次数,货物包括已经在泊位上的和机器人手上的
            berth_want_goods_level[i] = bert_fix_times[i] * floor(remaining_shipment); //剩余运输次数越多，需求等级越低
        }
        std::sort(sortted_berth_want_goods_level.begin(), sortted_berth_want_goods_level.end(), [&](const int& a, const int& b) {
            return berth_want_goods_level[a] < berth_want_goods_level[b]; // 按需求等级升序排列
        });
    }

    void cal_berth_want_ship_level(){
        for (int i = 0; i < MAX_Berth_Num; i++){
            float remaining_shipment = allberths[i]->on_way_ship - (allberths[i]->goodsNum + allberths[i]->on_way_robot) / float(MAX_Capacity); //剩余运输次数,货物包括已经在泊位上的和机器人手上的
            berth_want_ship_level[i] = bert_fix_times[i] * remaining_shipment; //剩余运输次数越多，需求等级越低
        }
    }

    int ship_choose_berth(){
        //用于指引船只进入最佳的泊位
        cal_berth_want_ship_level();
        int best_bert_id = -1;
        int best_fixed_level = 15000;
        for (int i = 0; i < MAX_Berth_Num; i++){
            int refixed_level = berth_want_ship_level[i] + bert_fix_times[i];
            if (refixed_level < best_fixed_level){
                best_fixed_level = refixed_level;
                best_bert_id = i;
            }
        }
        return best_bert_id;
    }

    // int robot_choose_berth(int robot_id){
    //     /* !!! 废弃代码 !!!*/
    //     /* !!! 废弃代码 !!!*/
    //     /* !!! 废弃代码 !!!*/

    //     /*用于指引机器人来到最佳的泊位
    //     使用: bert_id1，pos_id1,bert_id2,pos_id2 = chose_bertch(robot_id)
    //     bert_id1，pos_id1,bert_id2,pos_id2分别代表最佳位置和次佳位置,可供机器人寻路时自行选择。*/
    //     cal_berth_want_goods_level();
    //     Pos robot_pos = robots[robot_id]->pos;

    //     int best_bert_id = -1, second_bert_id = -1;
    //     int pos_id1 = -1, pos_id2 = -1;
    //     int best_fixed_level = 15000, second_fixed_level = 15000;
    //     for (int i = 0; i < MAX_Berth_Num; i++){
    //         if (allberths[i]->on_way_robot < 5){
    //             continue; //泊位上有五个以上机器人，不要去打扰，快检查是不是撞一起了
    //         }
    //         int min_length = 10000, min_id = -1;
    //         int id = 0;
    //         // bugs
    //         for (Direction* & dir : berths[i]->usePosDir) {
    //             if (dir[robot_pos.x][robot_pos.y].length < min_length){
    //                 min_length = dir[robot_pos.x][robot_pos.y].length;
    //                 min_id = id;
    //             }
    //             id++;
    //         }

    //         int refixed_level = berth_want_goods_level[i] + min_length;
    //         if (refixed_level < best_fixed_level){
    //             second_fixed_level = best_fixed_level;
    //             second_bert_id = best_bert_id;
    //             best_fixed_level = refixed_level;
    //             best_bert_id = i;
    //             pos_id1 = min_id;
    //         }
    //         else if (refixed_level < second_fixed_level){
    //             second_fixed_level = refixed_level;
    //             second_bert_id = i;
    //             pos_id2 = min_id;
    //         }
    //     }
    //     return best_bert_id, pos_id1, second_bert_id, pos_id2;
    // }

    int bert_ship_goods_check(int bert_id){
        //检查泊位和船只状态，返回值为泊位剩余容量
        if (!allberths[bert_id]->shipId.empty()){
            if (allberths[bert_id]->goodsNum > 0){
                if (bert_load_finish_times[bert_id] < nowTime){
                    allships[allberths[bert_id]->shipId[0]]->capacity += allberths[bert_id]->goodsNum;
                    allberths[bert_id]->goodsNum = 0;
                }else{
                    int loaded_goods =  (nowTime - bert_load_start_times[bert_id]) / bert_velocitys[bert_id];
                    allships[allberths[bert_id]->shipId[0]]->capacity += loaded_goods;
                    allberths[bert_id]->goodsNum -= loaded_goods;
                }
            }
        }
        return MAX_Capacity - allberths[bert_id]->goodsNum;
    }

    void declare_ship(int bert_id,int ship_id){
        allberths[bert_id]->on_way_ship ++;
        allberths[bert_id]->shipId.push_back(ship_id);
    }

    void declare_robot(int bert_id){
        allberths[bert_id]->on_way_robot ++;
    }

    void ship_declare_go(int bert_id){
        allberths[bert_id]->on_way_ship --;
    }

    void pull_good(int bert_id){
        // 装载货物: 首先顺手检查一下港口和轮船状态，然后装货
        allberths[bert_id]->goodsNum ++;
        allberths[bert_id]->on_way_robot --;
        bert_load_finish_times[bert_id] = bert_velocitys[bert_id] + nowTime;
        bert_load_start_times[bert_id] = nowTime;
    }

    void normal_berth_check(){
        bcenterlogger.log(nowTime, "call_ship_and_berth_check");
        for(int i = 0; i < MAX_Berth_Num; i++){
            bcenterlogger.log(nowTime, "berth: {0}", i);
            bert_ship_goods_check(i);
            bcenterlogger.log(nowTime, "berth: {0} statcheck ok", i);
            if (!allberths[i]->shipId.empty()){
                //有船,检查船的状态
                if (allships[allberths[i]->shipId[0]]->leftCapacity() <= 1 && allships[allberths[i]->shipId[0]]->status == 1){
                    //发船
                    bcenterlogger.log(nowTime, "have ship!");
                    bcenterlogger.log(nowTime, "ship on berth: {0}", allberths[i]->shipId[0]);
                    bcenterlogger.log(nowTime, "leftCapacity: {0}", allships[allberths[i]->shipId[0]]->leftCapacity());
                    ship_declare_go(i);
                    allships[allberths[i]->shipId[0]]->go(i);
                    bcenterlogger.log(nowTime, "ship{0} go", allberths[i]->shipId[0]);
                    allships[allberths[i]->shipId[0]]->capacity = 0;
                }
            }
            bcenterlogger.log(nowTime, "berth: {0} ok", i);
        }
        bcenterlogger.log(nowTime, "berth_check ok");
    }

    void normal_ship_check(int i){
        bcenterlogger.log(nowTime, "ship: {0}", i);
        // shipLogger.log(nowTime, "ship{0} status: {1}, berthId: {2}", i, allships[i]->status, allships[i]->berthId);
        if (allships[i]->status == 0){
            //运输中，不做处理
            return;
        }
        else if (allships[i]->berthId == -1){
            //送货完毕,重新找泊位
            int best_bert_id = ship_choose_berth();
            declare_ship(best_bert_id, i);
            allships[i]->go_berth(best_bert_id);
            shipLogger.log(nowTime, "centre command ship{0} to berth{1}", i, best_bert_id);
        }
        else if (allships[i]->berthId == 2){
            //排队的,等会处理
            return;
        }
        bcenterlogger.log(nowTime, "ship: {0} ok", i);
    }
};

Berth_center *berth_center = new Berth_center();

#endif