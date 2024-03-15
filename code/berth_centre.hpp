#ifndef __BERTH_CENTRE_H__
#define __BERTH_CENTRE_H__
#include "config.hpp"
#include "grid.hpp"
#include "ship.hpp"
#include "berth.hpp"
#include "robot.hpp"
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


struct Berth_center {
    /*Berth_center即船坞、轮船控制中心（塔台），进行统一调配，并指引机器人进入泊位*/
    Berth **allberths = berths;
    Ship **allships = ships;
    float berth_want_goods_level[MAX_Berth_Num]; //表示正常货物需求等级 0-?越低越急

    int bert_velocitys[MAX_Berth_Num];
    int bert_load_start_times[MAX_Berth_Num];
    int bert_load_finish_times[MAX_Berth_Num]; //运输到虚拟点的剩余时间
    int bert_times[MAX_Berth_Num]; //运输到虚拟点的时间
    int bert_fix_times[MAX_Berth_Num]; //运输到虚拟点的 “修正时间”（即运输到虚拟点的时间+船只装货时间）

    std::vector<int> sortted_bert_by_time = std::vector<int>(MAX_Berth_Num);
    std::vector<int> sortted_bert_by_velocity = std::vector<int>(MAX_Berth_Num);
    std::vector<int> sortted_bert_fix_times = std::vector<int>(MAX_Berth_Num);

    void init_doing(){
        /*
        todo: 这里应该有些不必要的变量，记得优化掉
        */
        //初始化各种参数
        for (int i = 0; i < MAX_Berth_Num; i++){
            berth_want_goods_level[i] = 15000.0;
            bert_load_finish_times[i] = 0;
            bert_load_start_times[i] = 0;
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
    }
    
    void first_frame_doing(){
        //指引初始状态的船只进入泊位，能不能放到init_doing里？
        for (int i = 0; i < MAX_Ship_Num; i++){
            allships[i]->go(sortted_bert_by_time[i]); // 指引船只进入泊位, 优先级按时间升序排列
            allberths[sortted_bert_by_time[i]]->on_way_ship ++;
        }
    }

    void get_berth_want_goods_level(){
        //按泊位多，船少计算，计算每个泊位对机器人的需求等级
        for (int i = 0; i < MAX_Berth_Num; i++){
            if (allberths[i]->waitting_ship != 0 && //有船(可能不止一只)
            allberths[i]->goodsNum + allships[allberths[i]->shipId]->capacity < MAX_Capacity * allberths[i]->waitting_ship //这船(可能不止一只)还装不满
            ){
                berth_want_goods_level [i] = 0; //急需货物，快来！
                continue;
            }
            int remaining_shipment = (allberths[i]->goodsNum +allberths[i]->on_way_robot)/ MAX_Capacity; //剩余运输次数,货物包括已经在泊位上的和机器人手上的
            berth_want_goods_level[i] = bert_fix_times[i] * remaining_shipment;
        }
    }

    int chose_bertch(int robot_id){
        /*用于指引机器人来到最佳的泊位
        使用: bert_id1，pos_id1,bert_id2,pos_id2 = chose_bertch(robot_id)
        bert_id1，pos_id1,bert_id2,pos_id2分别代表最佳位置和次佳位置,可供机器人寻路时自行选择。*/
        get_berth_want_goods_level();
        Pos robot_pos = robots[robot_id]->pos;

        int best_bert_id = -1, second_bert_id = -1;
        int pos_id1 = -1, pos_id2 = -1;
        int best_fixed_level = 15000, second_fixed_level = 15000;
        for (int i = 0; i < MAX_Berth_Num; i++){
            if (allberths[i]->on_way_robot < 5){
                continue; //泊位上有五个以上机器人，不要去打扰，快检查是不是撞一起了
            }
            int min_length = 10000, min_id = -1;
            int id = 0;
            // bugs
            for (auto & dir : berths[i]->usePosDir) {
                if (dir[robot_pos.x][robot_pos.y].length < min_length){
                    min_length = dir[robot_pos.x][robot_pos.y].length;
                    min_id = id;
                }
                id++;
            }

            int refixed_level = berth_want_goods_level[i] + min_length;
            if (refixed_level < best_fixed_level){
                second_fixed_level = best_fixed_level;
                second_bert_id = best_bert_id;
                best_fixed_level = refixed_level;
                best_bert_id = i;
                pos_id1 = min_id;
            }
            else if (refixed_level < second_fixed_level){
                second_fixed_level = refixed_level;
                second_bert_id = i;
                pos_id2 = min_id;
            }
        }
        return best_bert_id, pos_id1, second_bert_id, pos_id2;
    }

    int bert_ship_goods_check(int bert_id){
        //检查泊位和船只状态，返回值为泊位剩余容量
        if (allberths[bert_id]->goodsNum > 0){
            if (bert_load_finish_times[bert_id] < nowTime){
                allships[allberths[bert_id]->shipId]->capacity += allberths[bert_id]->goodsNum;
                allberths[bert_id] = 0;
            }else{
                int pulled_goods =  (nowTime - bert_load_start_times[bert_id]) / bert_velocitys[bert_id];
                allships[allberths[bert_id]->shipId]->capacity += pulled_goods;
                allberths[bert_id]->goodsNum -= pulled_goods;
            }
        }
        return MAX_Capacity - allberths[bert_id]->goodsNum;
    }

    void declare_ship(int bert_id){
        allberths[bert_id]->on_way_ship ++;
    }

    void declare_robot(int bert_id){
        allberths[bert_id]->on_way_robot ++;
    }

    void pull_good(int bert_id){
        // 装载货物: 首先顺手检查一下港口和轮船状态，然后装货
        /* 暂时检测港口和轮船状态的函数放这了，如果有条件还是放到主时间轴上比较好 */
        int cap_left = bert_ship_goods_check(bert_id);
        allberths[bert_id]->goodsNum ++;
        allberths[bert_id]->on_way_robot --;
        bert_load_finish_times[bert_id] = bert_velocitys[bert_id] + nowTime;
        bert_load_start_times[bert_id] = nowTime;
        if (cap_left <= 2){
            allships[allberths[bert_id]->shipId]->go(bert_id);
        }
    }
};

#endif