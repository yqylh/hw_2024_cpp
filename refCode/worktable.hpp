#ifndef __WORKTABLE_H__
#define __WORKTABLE_H__
#include "config.hpp"

struct Worktable{
    int id; // 工作台的 id
    double x; // 工作台的 x 坐标
    double y; // 工作台的 y 坐标
    int remainTime; // 工作台的剩余时间, -1 表示没有
    int type; // 工作台的类型
    bool output; // 工作台的输出状态
    int inputId[MAX_Item_Type_Num + 1]; // 工作台的输入物品的 id, 0 表示没有
    int someWillBuy; // 是否有人选择了这个工作台
    int someWillSell[MAX_Item_Type_Num + 1]; // 是否有人选择了这个工作台卖出
    int waitPriority; // 是否在等待
    double near7; // 卖时的参数
    bool isNearCorner; // 是否在角落
    bool blocked; // 是否被占领
    Worktable() {
        this->id = -1;
        this->x = -1;
        this->y = -1;
        this->remainTime = -1;
        this->type = -1;
        this->output = false;
        for (int i = 0; i <= MAX_Item_Type_Num; i++) {
            this->inputId[i] = 0;
            this->someWillSell[i] = 0;
        }
        this->someWillBuy = 0;
        near7 = 1;
    }
    Worktable(int id, double x, double y, int type) {
        this->id = id;
        this->x = x;
        this->y = y;
        this->remainTime = -1;
        this->type = type;
        this->output = false;
        for (int i = 0; i <= MAX_Item_Type_Num; i++) {
            this->inputId[i] = 0;
            this->someWillSell[i] = 0;
        }
        this->someWillBuy = 0;
        near7 = 1;
    }
    void checkCanBuy() {
        // 检查对哪些物品有需求
        for (int i = 1; i <= MAX_Item_Type_Num; i++) {
            if (sellSet.find(std::make_pair(i, this->type)) != sellSet.end()) {
                if (this->inputId[i] == 0) {
                    canBuy[i]++;
                }
            }
        }
    }
    void outputTest() {
        TESTOUTPUT(fout << "Worktable id: " << id << std::endl;)
        TESTOUTPUT(fout << "x: " << x << std::endl;)
        TESTOUTPUT(fout << "y: " << y << std::endl;)
        TESTOUTPUT(fout << "remainTime: " << remainTime << std::endl;)
        TESTOUTPUT(fout << "type: " << type << std::endl;)
        TESTOUTPUT(fout << "output: " << output << std::endl;)
        TESTOUTPUT(fout << "inputId: ";)
        for (int i = 1; i <= MAX_Item_Type_Num; i++) {
            TESTOUTPUT(fout << "item" << i << inputId[i] << " ";)
        }
        TESTOUTPUT(fout << std::endl;)
    }
    void checkWait() {
        int all = 0;
        int have = 0;
        // 检查是否在等待多个
        for (int i = 1; i <= MAX_Item_Type_Num; i++) {
            if (sellSet.find(std::make_pair(i, this->type)) != sellSet.end()) {
                all++;
                if (this->inputId[i] == 1) {
                    have++;
                }
            }
        }
        // 分四级
        // 4 5 6 存在一个输入
        if (all == 2 && have == 1) {
            this->waitPriority = 4;
        }
        // 7 存在两个输入
        if (all == 3 && have == 2) {
            this->waitPriority = 5;
        } else
        // 7 存在一个输入
        if (all == 3 && have >= 1) {
            this->waitPriority = 4;
        }
        // 4 5 6 7 不存在输入
        if (have == 0) {
            this->waitPriority = 3;
        }
        // 8 || 9 结果最差, 将其设置为 0
        if (all == 7 || all == 1) this->waitPriority = 2;
        // // 不分级
        // this->waitPriority = 2;
        // // 分两级
        // this->waitPriority = 3;
        // // 8 || 9 结果最差, 将其设置为 0
        // if (all == 7 || all == 1) this->waitPriority = 2;
    }
};
Worktable worktables[MAX_Worktable_Num + 5];
Worktable worktablesFoe[MAX_Worktable_Num];

#endif