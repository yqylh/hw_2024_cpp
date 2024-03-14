#ifndef __BERTH_CENTRE_H__
#define __BERTH_CENTRE_H__
#include "config.hpp"
#include "grid.hpp"
#include "ship.hpp"
#include "berth.hpp"
#include <vector>

/*
使用说明：
Berth_center 即控制塔台，用于调送货轮，指引机器人进入泊位
函数说明：
void init_doing() 初始化，指引船只进入泊位，在初始化时使用
void get_berth_want_goods_level() 计算每个泊位的需求等级，内部使用
int chose_bertch(int robot_id) 用于指引机器人来到最佳的泊位 
    使用方法 bert_id1，pos_id1,bert_id2,pos_id2 = chose_bertch(robot_id)
*/


struct Berth_center {
    /*Berth_center即船坞、轮船控制中心（塔台），进行统一调配，并指引机器人进入泊位*/
    Berth **allberths = berths;
    Ship **allships = ships;
    float berth_want_goods_level[MAX_Berth_Num]; //表示正常货物需求等级 0-?越低越急

    int bert_velocitys[MAX_Berth_Num];
    int bert_times[MAX_Berth_Num]; //运输到虚拟点的时间
    int bert_fix_times[MAX_Berth_Num]; //运输到虚拟点的 “修正时间”（即运输到虚拟点的时间+船只装货时间）


    // int sortted_bert_by_time[MAX_Berth_Num];
    // int sortted_bert_by_velocity[MAX_Berth_Num];
    std::vector<int> sortted_bert_by_time = std::vector<int>(MAX_Berth_Num);
    std::vector<int> sortted_bert_by_velocity = std::vector<int>(MAX_Berth_Num);
    std::vector<int> sortted_bert_fix_times = std::vector<int>(MAX_Berth_Num);

    void init_doing(){
        /*
        todo: 这里应该有些不必要的变量，记得优化掉
        */
        //初始化 berth_want_goods_level
        for (int i = 0; i < MAX_Berth_Num; i++){
            berth_want_goods_level[i] = 15000.0;
        }
        for (int i = 0; i < MAX_Berth_Num; i++){
            bert_velocitys[i] = allberths[i]->velocity;
            bert_times[i] = allberths[i]->time;
            bert_fix_times[i] = bert_times[i] + bert_velocitys[i] * allships[0]->capacity; //运输到虚拟点的时间+船只装货时间
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
        for (int i = 0; i < MAX_Ship_Num; i++){
            allships[i]->go(sortted_bert_by_time[i]); // 指引船只进入泊位, 优先级按时间升序排列
            allberths[sortted_bert_by_time[i]]->on_way_ship ++;
        }
        
    }

    void get_berth_want_goods_level(){
        //按泊位多，船少计算
        for (int i = 0; i < MAX_Berth_Num; i++){
            if (allberths[i]->waitting_ship != 0 && //有船(可能不止一只)
            allberths[i]->goodsNum + allships[allberths[i]->shipId]->goodsNum < MAX_Capacity * allberths[i]->waitting_ship //这船(可能不止一只)还装不满
            ){
                berth_want_goods_level [i] = 0; //急需货物，快来！
                continue;
            }
            int remaining_shipment = (allberths[i]->goodsNum +allberths[i]->on_way_robot)/ allships[0]->capacity; //剩余运输次数,货物包括已经在泊位上的和机器人手上的
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
            int min_length = 10000, min_id = -1;
            int id = 0;
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

    void declare_ship(int bert_id){
        allberths[bert_id]->on_way_ship ++;
    }

    void declare_robot(int bert_id){
        allberths[bert_id]->on_way_robot ++;
    }

    void pull_good(int bert_id){
        allberths[bert_id]->goodsNum ++;
    }
};

#endif