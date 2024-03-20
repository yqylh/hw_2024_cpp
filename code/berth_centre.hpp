#ifndef __BERTH_CENTRE_H__
#define __BERTH_CENTRE_H__
#include "config.hpp"
#include "grid.hpp"
#include "ship.hpp"
#include "berth.hpp"
// #include "robot.hpp"
#include <vector>

class Berth_center {
public:
    bool is_init = false;

    void call_ship_and_berth_check(){
        /* 检查船坞\船的当前容量,满则发船 */
        normal_berth_check(); // 船坞检查 不用外置循环

        for(int i = 0; i < MAX_Ship_Num; i++){
            if(allships[i] ->berthId != -1){
                // 判断是否是最后一轮
                if (allships[i] -> is_last_round) continue;
                if (allberths[allships[i]->berthId]->time + nowTime + 20 > MAX_TIME){
                    allships[i]->go(allships[i]->berthId);
                    allships[i] -> is_last_round = true;
                    // bcenterlogger.log(nowTime, "ship{0} go last round", i);
                }
            }
            normal_ship_check(i);
        }
        // bcenterlogger.log(nowTime, "ship_check ok");

        if (nowTime >= 14999) {
            int leftTotal = 0;
            for (int i = 0; i < MAX_Berth_Num; i++) {
                berthLogger.log(nowTime, "berth{},goodsNum:{}", i, allberths[i]->goodsNum);
                leftTotal += allberths[i]->goodsNum;
            }
            berthLogger.log(nowTime, "leftTotal:{}", leftTotal);
            berthLogger.log(nowTime, "tmpTotalGoods:{}", tmpTotalGoods + leftTotal);
        }
    }

    float* call_robot_choose_berth(){
        cal_berth_want_goods_level();
        /* 用于指引机器人来到最佳的泊位,暂时只返回船坞的需求值,需要机器人加上自身距离再做判断 */
        return berth_want_goods_level;
    }

    void declare_robot_choose_berth(int bert_id){
        /* 机器人告知塔台将前往那个船坞 */
        declare_robot(bert_id);
    }

    void declare_robot_pull_good(int bert_id){
        /* 机器人告知塔台卸货 */
        // bcenterlogger.log(nowTime, "declare_robot_pull_good");
        pull_good(bert_id);
    }

    void do_first_frame(std::vector<Pos> pos){
        this->robot_pos = pos;
        /* 初始化，指引船只进入泊位 */
        init_doing();
        first_frame_doing();
        is_init = true;
    }
    void solve_robot_berth() {
        for (int i = 0; i < MAX_Robot_Num; i++) robot_choose_berth[i] = -1;
        for (int i = 0; i < MAX_Berth_Num; i++) {
            for (int j = 0; j < MAX_Robot_Num; j++) {
                if (berths[i]->disWithTimeBerth[robot_pos[j].x][robot_pos[j].y] != 0x3f3f3f3f) {
                    centerLogger.log(nowTime, "berth{0} could reach by robot{1}", i, j);
                }
            }
        }

        // 对于每个泊位
        for (int i = 0; i < MAX_Ship_Num; i++) {
            // 获取泊位 id
            auto berth_id = sortted_bert_by_one_round_time[i];
            std::vector<std::pair<int, int>> robot_dis;
            // 排序机器人到泊位的距离
            for (int j = 0; j < MAX_Robot_Num; j++) if (robot_choose_berth[j] == -1) {
                robot_dis.push_back(std::make_pair(j, berths[berth_id]->disWithTimeBerth[robot_pos[j].x][robot_pos[j].y]));
            }
            std::sort(robot_dis.begin(), robot_dis.end(), [&](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                return a.second < b.second;
            });
            // 选择距离最近的两个机器人
            for (int j = 0; j < 2; j++) {
                if (robot_dis[j].second != 0x3f3f3f3f) {
                    robot_choose_berth[robot_dis[j].first] = berth_id;
                }
            }
        }
        // 可能有些机器人没有选择到泊位,因为地图是分散的
        for (int i = 0; i < MAX_Robot_Num; i++) {
            if (robot_choose_berth[i] == -1) {
                for (int j = 0; j < MAX_Ship_Num; j++) {
                    auto berth_id = sortted_bert_by_one_round_time[j];
                    if (berths[berth_id]->disWithTimeBerth[robot_pos[i].x][robot_pos[i].y] != 0x3f3f3f3f) {
                        robot_choose_berth[i] = berth_id;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < MAX_Robot_Num; i++) {
            centerLogger.log(nowTime, "robot_choose_berth: {0} {1}", i, robot_choose_berth[i]);
        }
    }
    int get_robot_berth(int id) {
        return robot_choose_berth[id];
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
    int robot_choose_berth[MAX_Robot_Num]; //机器人选择的泊位
    std::vector<Pos> robot_pos;
    std::vector<int> sortted_bert_by_one_round_time = std::vector<int>(MAX_Berth_Num);
    std::vector<int> sortted_bert_by_velocity = std::vector<int>(MAX_Berth_Num);
    std::vector<int> sortted_bert_fix_times = std::vector<int>(MAX_Berth_Num);
    std::vector<int> sortted_berth_want_goods_level = std::vector<int>(MAX_Berth_Num);
    /**
     * 用来查找每个机器人私有的区域
    */
    void find_private_space() {
        // 每个泊位的可以接触到多少机器人
        int berth_can_reach_robot[MAX_Berth_Num] = {0};
        for (int i = 0; i < MAX_Berth_Num; i++) {
            for (int j = 0; j < MAX_Robot_Num; j++) {
                if (berths[i]->disWithTimeBerth[robot_pos[j].x][robot_pos[j].y] != 0x3f3f3f3f) {
                    centerLogger.log(nowTime, "berth{0} could reach by robot{1}", i, j);
                    berth_can_reach_robot[i]++;
                }
            }
        }
        std::vector<std::vector<int> > group;
        int is_grouped[MAX_Berth_Num];
        for (int i = 0; i < MAX_Berth_Num; i++) is_grouped[i] = -1;
        for (int i = 0; i < MAX_Berth_Num; i++) {
            if (is_grouped[i] != -1) continue;
            is_grouped[i] = i;
            group.push_back(std::vector<int>());
            group.back().push_back(i);
            for (int j = i + 1; j < MAX_Berth_Num; j++) {
                if (is_grouped[j] != -1) continue;
                if (berths[i]->disWithTimeBerth[berths[j]->pos.x][berths[j]->pos.y] != 0x3f3f3f3f) {
                    group.back().push_back(j);
                    is_grouped[j] = i;
                }
            }
        }
        for (int i = 0; i < group.size(); i++) {
            centerLogger.log(nowTime, "group{}:", i);
            for (int j = 0; j < group[i].size(); j++) {
                centerLogger.log(nowTime, "    {}", group[i][j]);
            }
        }
        // 每个机器人的私有区域 的路径长度
        std::vector<int> berth_onwer_space[MAX_Berth_Num];
        // 空地大小
        int ground_num = 0;
        for (int x = 0; x < MAX_Line_Length; x++) {
            for (int y = 0; y < MAX_Col_Length; y++) {
                // 排除障碍物和海洋
                if (grids[x][y]->type == 1 || grids[x][y]->type == 2) continue;
                ground_num++;
                std::vector<int> owner;
                // 我们只考虑 150 帧内的情况
                int min_num = 150;
                for (int i = 0; i < MAX_Berth_Num; i++) {
                    if (berths[i]->disWithTimeBerth[x][y] < min_num) {
                        min_num = berths[i]->disWithTimeBerth[x][y];
                        owner.clear();
                        owner.push_back(i);
                    } else if (berths[i]->disWithTimeBerth[x][y] == min_num) {
                        owner.push_back(i);
                    }
                }
                for (auto &i : owner) {
                    berth_onwer_space[i].push_back(min_num);
                }
            }
        }
        // 每个泊位的平均私有区域长度
        double avg_onwer_space_length[MAX_Berth_Num];
        // 独享区域生成 max_capacity 需要的时间
        double create_time[MAX_Berth_Num];
        for (int i = 0; i < MAX_Berth_Num; i++) {
            double sum = 0;
            for (auto &len : berth_onwer_space[i]) {
                sum += len;
            }
            avg_onwer_space_length[i] = sum / berth_onwer_space[i].size();
            // 每帧平均生成 5 个物品 每帧该地区生成(avg_onwer_space_length[i] / ground_num * 5)个物品,一共需要下面的时间才能生成完. ? berth_onwer_space[i].size()
            create_time[i] = MAX_Capacity / (avg_onwer_space_length[i] / double(ground_num) * 5.0);
            centerLogger.log(nowTime, "berth{}: avg_onwer_space_length: {} create_time: {}", i, avg_onwer_space_length[i], create_time[i]);
            // 这个 bug 导致 create_time 极大,远超了其他参数, 而create_time只取决于avg_onwer_space_length,也就是平均路径长度,所以这个 bug 的反应了平均路径长度
        }
        // 计算一个港口一轮需要的时间
        double one_round_time[MAX_Berth_Num];
        for (int i = 0; i < MAX_Berth_Num; i++) {
            // 一个机器人平均一个物品需要的时间
            double robot_running_time = avg_onwer_space_length[i] * 2;
            // 两个机器人运输完一船货物需要的时间
            double ship_waiting_time = robot_running_time * MAX_Capacity / ((berth_can_reach_robot[i] > 3) ? 2 : berth_can_reach_robot[i]);
            // 如果拉满一船需要的时间比生成时间长，那么需要重新计算拉满一船的时间 也就是创建完最后一个再加上机器人运行的时间
            if (ship_waiting_time < create_time[i]) {
                ship_waiting_time = create_time[i] + robot_running_time;
            }
            // 最后两个货物还需要 1~2 帧放到船上
            ship_waiting_time += 2.0 / bert_velocitys[i];
            // 如果运输一船货物需要的时间没有搬运的时间长,那么更新为 一个机器人拉过去的时间 + 全部的搬运时间
            if (ship_waiting_time < double(MAX_Capacity) / bert_velocitys[i]) {
                ship_waiting_time = double(MAX_Capacity) / bert_velocitys[i] + robot_running_time;
            }
            // 来回虚拟点的时间 + 船只等待的时间
            one_round_time[i] = bert_times[i] * 2 + ship_waiting_time;
        }
        // 排序港口的优先级
        std::sort(sortted_bert_by_one_round_time.begin(), sortted_bert_by_one_round_time.end(), [&](const int& a, const int& b) {
            return one_round_time[a] < one_round_time[b]; // 按时间升序排列
        });
        for (int i = 0; i < MAX_Berth_Num; i++) {
            centerLogger.log(nowTime, "one_round_time: {0} {1}", sortted_bert_by_one_round_time[i], one_round_time[sortted_bert_by_one_round_time[i]]);
        }
        int temp[MAX_Berth_Num];
        int group_selected[MAX_Berth_Num] = {0};
        bool selected[MAX_Berth_Num] = {0};
        int selected_num = 0;
        int group_num = 1;
        
        while (selected_num < MAX_Ship_Num) {
            for (int i = 0; i < MAX_Berth_Num; i++) {
                auto ordered_id = sortted_bert_by_one_round_time[i];
                if (selected[ordered_id]) continue;
                if (group_selected[is_grouped[ordered_id]] >= group_num) continue;
                //!!todo:还需要判断是不是太差了
                temp[selected_num++] = ordered_id;
                selected[ordered_id] = true;
                group_selected[is_grouped[ordered_id]]++;
                if (selected_num >= MAX_Ship_Num) break;
            }
            group_num++;
            if (group_num > 5) break;
        }
        for (int i = 0; i < MAX_Berth_Num; i++) {
            auto ordered_id = sortted_bert_by_one_round_time[i];
            if (!selected[ordered_id]) {
                temp[selected_num++] = ordered_id;
            }
        }
        for (int i = 0; i < MAX_Berth_Num; i++) {
            sortted_bert_by_one_round_time[i] = temp[i];
        }
        centerLogger.log(nowTime, "sortted_bert_by_one_round_time: {0} {1} {2} {3} {4}", sortted_bert_by_one_round_time[0], sortted_bert_by_one_round_time[1], sortted_bert_by_one_round_time[2], sortted_bert_by_one_round_time[3], sortted_bert_by_one_round_time[4]);
    }
    
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

            sortted_bert_by_one_round_time[i] = i;
            sortted_bert_by_velocity[i] = i;
            sortted_bert_fix_times[i] = i;
            sortted_berth_want_goods_level[i] = i;
        }
        for (int i = 0; i < MAX_Berth_Num; i++){
            bert_velocitys[i] = allberths[i]->velocity;
            bert_times[i] = allberths[i]->time;
            bert_fix_times[i] = bert_times[i] + MAX_Capacity / bert_velocitys[i]; //运输到虚拟点的时间+船只装货时间
        }
        std::sort(sortted_bert_by_velocity.begin(), sortted_bert_by_velocity.end(), [&](const int& a, const int& b) {
            return bert_velocitys[a] > bert_velocitys[b]; // 按装填速度升序排列
        });
        std::sort(sortted_bert_fix_times.begin(), sortted_bert_fix_times.end(), [&](const int& a, const int& b) {
            return bert_fix_times[a] < bert_fix_times[b]; // 按修正时间升序排列
        });
        find_private_space();
        // bcenterlogger.log(nowTime, "init_done");
    }
    
    void first_frame_doing(){
        //指引初始状态的船只进入泊位，能不能放到init_doing里？
        for (int i = 0; i < MAX_Ship_Num; i++){
            allships[i]->go_berth(sortted_bert_by_one_round_time[i]); // 指引船只进入泊位, 优先级按时间升序排列
            declare_ship(sortted_bert_by_one_round_time[i], i);
            allberths[sortted_bert_by_one_round_time[i]]->on_way_ship++;
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
            // bcenterlogger.log(nowTime, "goodsNum :{}", allberths[i]->goodsNum + allberths[i]->on_way_robot);
            float remaining_shipment = (allberths[i]->goodsNum + allberths[i]->on_way_robot) / float(MAX_Capacity); //剩余运输次数,货物包括已经在泊位上的和机器人手上的
            // bcenterlogger.log(nowTime, "remaining_shipment: {} int: {} , bert_fix_times:{}", remaining_shipment,floor(remaining_shipment),bert_fix_times[i]);
            berth_want_goods_level[i] = bert_fix_times[i] * ceil(remaining_shipment); //剩余运输次数越多，需求等级越低
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

    int ship_choose_berth(int shipId){
        //用于指引船只进入最佳的泊位
        // cal_berth_want_ship_level();
        // int best_bert_id = -1;
        // int best_fixed_level = 15000;
        // for (int i = 0; i < MAX_Berth_Num; i++){
        //     int refixed_level = berth_want_ship_level[i] + bert_fix_times[i];
        //     if (refixed_level < best_fixed_level){
        //         best_fixed_level = refixed_level;
        //         best_bert_id = i;
        //     }
        // }
        // return best_bert_id;
        return sortted_bert_by_one_round_time[shipId];
    }

    int bert_ship_goods_check(int bert_id){
        //检查泊位和船只状态，返回值为泊位剩余容量
        // 如果港口非空
        if (!allberths[bert_id]->shipId.empty()) {
            // 如果船在港口，可以装卸货，或者船在虚拟点，那就取队列最前端的船
            if (allships[allberths[bert_id]->shipId[0]]->status == 1){
                // 
                if (bert_load_start_times[bert_id] == MAX_TIME){
                    bert_load_start_times[bert_id] = nowTime;
                    bert_load_finish_times[bert_id] = nowTime + allberths[bert_id]->goodsNum / bert_velocitys[bert_id] + 1;
                }
                if (allberths[bert_id]->goodsNum > 0){
                    allberths[bert_id]->ship_wait_start_time = nowTime;
                    if (bert_load_finish_times[bert_id] < nowTime){
                        allships[allberths[bert_id]->shipId[0]]->capacity += allberths[bert_id]->goodsNum;
                        allberths[bert_id]->goodsNum = 0;
                        bert_load_start_times[bert_id] = 0;
                    }
                    else{
                        int loaded_goods =  (nowTime - bert_load_start_times[bert_id]) * bert_velocitys[bert_id];
                        if (loaded_goods > allberths[bert_id]->goodsNum){
                            loaded_goods = allberths[bert_id]->goodsNum;
                        }
                        allships[allberths[bert_id]->shipId[0]]->capacity += loaded_goods;
                        allberths[bert_id]->goodsNum -= loaded_goods;
                        // bcenterlogger.log(nowTime, "loaded_goods: {}, remaining goods: {}", loaded_goods, allberths[bert_id]->goodsNum);
                        bert_load_start_times[bert_id] = bert_load_start_times[bert_id] + loaded_goods / bert_velocitys[bert_id];
                    }
                } else if (allberths[bert_id]->on_way_robot < 0) {
                    // bcenterlogger.log(nowTime, "warning : goodsNum: {0}", allberths[bert_id]->goodsNum);
                }
            }
        }
        return MAX_Capacity - allberths[bert_id]->goodsNum;
    }

    void declare_ship(int bert_id,int ship_id){
        allberths[bert_id]->on_way_ship++;
        allberths[bert_id]->shipId.push_back(ship_id);
        allberths[bert_id]->ship_wait_start_time = nowTime + allberths[bert_id]->time;
    }

    void declare_robot(int bert_id){
        allberths[bert_id]->on_way_robot++;
    }

    void ship_declare_go(int bert_id){
        allberths[bert_id]->on_way_ship--;
        allberths[bert_id]->shipId.clear();
    }

    void ship_rechange(int bert_id){
        // ship_declare_go(bert_id);
        cal_berth_want_ship_level();
        int this_ship_id = allberths[bert_id]->shipId[0];
        allberths[bert_id]->on_way_ship--;
        allberths[bert_id]->shipId.clear();
        int best_bert_id = -1;
        int best_fixed_level = 15000;
        for (int i = 0; i < MAX_Berth_Num; i++){
            int refixed_level = berth_want_ship_level[i];
            // bcenterlogger.log(nowTime, "berth {} :refixed_level: {}", i, refixed_level);
            if (refixed_level < best_fixed_level){
                best_fixed_level = refixed_level;
                best_bert_id = i;
            }
        }//重新找一个被堆了大量货物的泊位
        // bcenterlogger.log(nowTime, "ship{0} rechange to berth {1}", this_ship_id, best_bert_id);

        for (int i = 0; i < MAX_Robot_Num; i++){
            if (robot_choose_berth[i] == -1){ //找到没有被指引的机器人,指引他们去该泊位
                robot_choose_berth[i] = best_bert_id;
                sortted_bert_by_one_round_time[this_ship_id] = best_bert_id;
            }
        }
        allberths[best_bert_id]->on_way_ship++;
        allberths[best_bert_id]->shipId.push_back(this_ship_id);
        allberths[best_bert_id]->ship_wait_start_time = nowTime + 500;
        allships[this_ship_id]->move_berth(best_bert_id);
        if(!allberths[bert_id]->shipId.empty()){
            // bcenterlogger.log(nowTime, "ship{0} rechange to berth{1} fail", this_ship_id, best_bert_id);
            for(auto i : allberths[bert_id]->shipId){
                // bcenterlogger.log(nowTime, "ship{0} still on berth{1}", i, bert_id);
            }
        }
    }

    void pull_good(int bert_id){
        // 装载货物: 首先顺手检查一下港口和轮船状态，然后装货
        // bcenterlogger.log(nowTime, "pull_good:berths {}->goodsNum: {}", bert_id,allberths[bert_id]->goodsNum);
        allberths[bert_id]->goodsNum++;
        allberths[bert_id]->on_way_robot--;
        if(allberths[bert_id]->on_way_ship > 0) if(allships[allberths[bert_id]->shipId[0]]->status == 0){
            //没船,等待船来装货
            bert_load_start_times[bert_id] = MAX_TIME;
            bert_load_finish_times[bert_id] = MAX_TIME + 1;
        }
        else if(bert_load_finish_times[bert_id] < nowTime){
            bert_load_finish_times[bert_id] = 1 + nowTime;
            bert_load_start_times[bert_id] = nowTime;
        }else{
            bert_ship_goods_check(bert_id);
            bert_load_finish_times[bert_id] += 1;
        }
        // bcenterlogger.log(nowTime, "pulled_good:berths {}->goodsNum: {}", bert_id,allberths[bert_id]->goodsNum);
    }

    void normal_berth_check(){
        // bcenterlogger.log(nowTime, "call_ship_and_berth_check");
        for(int i = 0; i < MAX_Berth_Num; i++){
            // bcenterlogger.log(nowTime, "berth: {0}", i);
            bert_ship_goods_check(i);
            // bcenterlogger.log(nowTime, "berth: {0} statcheck ok", i);
            if (!allberths[i]->shipId.empty()){
                //有船,检查船的状态
                if (allships[allberths[i]->shipId[0]]->leftCapacity() == 0 && allships[allberths[i]->shipId[0]]->status == 1){
                    //发船
                    // bcenterlogger.log(nowTime, "have ship!");
                    // bcenterlogger.log(nowTime, "ship on berth: {0}", allberths[i]->shipId[0]);
                    // bcenterlogger.log(nowTime, "leftCapacity: {0}", allships[allberths[i]->shipId[0]]->leftCapacity());

                    ship_declare_go(i);
                    allships[allberths[i]->shipId[0]]->go(i);
                    
                    // bcenterlogger.log(nowTime, "ship{0} go", allberths[i]->shipId[0]);
                }
            }
            // bcenterlogger.log(nowTime, "berth: {0} ok", i);
        }
        // bcenterlogger.log(nowTime, "berth_check ok");
    }

    void normal_ship_check(int shipId) {
        // bcenterlogger.log(nowTime, "ship: {0}", shipId);
        if (allships[shipId]->status == 0) {
            //运输中，不做处理
            return;
        } else if (allships[shipId]->status == 1) {
            if (allships[shipId]->berthId == -1) {
                // 送货完毕,重新找泊位
                allships[shipId]->capacity = 0;

                int best_bert_id = ship_choose_berth(shipId);
                declare_ship(best_bert_id, shipId);
                allships[shipId]->go_berth(best_bert_id);
                
                shipLogger.log(nowTime, "centre command ship{0} to berth{1}", shipId, best_bert_id);
            } else {
                // 装货中

                return;
            }
        } else if (allships[shipId]->status == 2) {
            //排队的,等会处理
            return;
        }
        // bcenterlogger.log(nowTime, "ship: {0} ok", shipId);
    }
};

Berth_center *berth_center = new Berth_center();

#endif