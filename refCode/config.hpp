#ifndef __CONFIG_H__
#define __CONFIG_H__
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
#include "vector.hpp"
const int MAX_Item_Type_Num = 7; // 最大物品类型数 1-7
const int MAX_Worktable_Type_Num = 9; // 最大工作台类型数 1-9
const int MAX_Worktable_Num = 50; // 最大工作台数
const int MAX_Robot_Num = 4; // 最大机器人数
const int MAP_Line_Length = 100; // 地图有多少行
const int MAP_Col_Length = 100; // 地图有多少列
const int MAX_TIME = 5 * 60 * 50; // 最大帧数
int canBuy[MAX_Item_Type_Num + 1] = {0}; // 当前物品需求
double TOL_Collision = 8; // 碰撞检测的距离

std::set<std::pair<int, int> > sellSet = {
    std::pair<int, int>(1, 4), std::pair<int, int>(1, 5), std::pair<int, int>(1, 9),
    std::pair<int, int>(2, 4), std::pair<int, int>(2, 6), std::pair<int, int>(2, 9),
    std::pair<int, int>(3, 5), std::pair<int, int>(3, 6), std::pair<int, int>(3, 9),
    std::pair<int, int>(4, 7), std::pair<int, int>(4, 9),
    std::pair<int, int>(5, 7), std::pair<int, int>(5, 9),
    std::pair<int, int>(6, 7), std::pair<int, int>(6, 9),
    std::pair<int, int>(7, 8), std::pair<int, int>(7, 9)
};// 可以卖的物品, 对于 <x,y>, x为物品类型, y为工作台类型, 即可以在y类型的工作台上卖出x类型的物品
std::map<int, int> createMap{
    std::pair<int, int>(1, 1), std::pair<int, int>(2, 2), std::pair<int, int>(3, 3),
    std::pair<int, int>(4, 4), std::pair<int, int>(5, 5), std::pair<int, int>(6, 6),
    std::pair<int, int>(7, 7) 
};// 可以生产的物品, 对于 <x,y>, x为工作台类型, y为物品类型, 即可以在y类型的工作台上购买/生产y类型的物品

std::map<int, int> buyMoneyMap{
    std::pair<int, int>(1, 3000), std::pair<int, int>(2, 4400), std::pair<int, int>(3, 5800),
    std::pair<int, int>(4, 15400), std::pair<int, int>(5, 17200), std::pair<int, int>(6, 19200),
    std::pair<int, int>(7, 76000) 
};// 购买价格, 对于 <x,y>, x为物品类型, y为金钱, 即可以在y金钱的情况下购买x类型的物品
std::map<int, int> sellMoneyMap{
    std::pair<int, int>(1, 6000), std::pair<int, int>(2, 7600), std::pair<int, int>(3, 9200),
    std::pair<int, int>(4, 22500), std::pair<int, int>(5, 25000), std::pair<int, int>(6, 27500),
    std::pair<int, int>(7, 105000) 
};// 卖出价格, 对于 <x,y>, x为物品类型, y为金钱, 即可以在y金钱的情况下卖出x类型的物品

std::map<int, int> buyPrioriryMap{
    std::pair<int, int>(1, 1), std::pair<int, int>(2, 1), std::pair<int, int>(3, 1),
    std::pair<int, int>(4, 2), std::pair<int, int>(5, 2), std::pair<int, int>(6, 2),
    std::pair<int, int>(7, 3) 
};// 购买优先级, 对于 <x,y>, x为工作台, y为优先级, 即可以在y优先级的情况下购买x工作台的物品

double WTtoWT[MAX_Worktable_Num][MAX_Worktable_Num] = {-1}; // 不带物品工作台到工作台的距离
double WTtoWTwithItem[MAX_Worktable_Num][MAX_Worktable_Num] = {-1}; // 带物品工作台到工作台的距离
double RobotToWT[MAX_Robot_Num][MAX_Worktable_Num] = {-1}; // 机器人到工作台的距离
int haveCreateNum[8] = {0}; // 工作台已经生产的物品数量

int robotNum = -1; // 当前机器人标号,实际数量-1, 0-robotNum
int robotNumFoe = -1; // 对方机器人标号,实际数量-1, 0-robotNumFoe
int worktableNum = -1; // 当前工作台标号,实际数量-1, 0-worktableNum
int worktableNumFoe = -1; // 对方工作台标号,实际数量-1, 0-worktableNumFoe
int money = 0; // 当前金钱
int nowTime = 0; // 当前帧数
int lossCollMoney = 0; // 当前损失的金钱
int lossTimeMoney = 0; // 当前每帧的金钱
int gankType = 0; // gank 对面的类型
bool inputFlag = true; // 是否input是否结束
bool flag2 = true;
std::set<Vector2D> FoeBlockPos; // 对面的阻挡点
std::vector<int> gankFoeWT; // gank 对面的工作台
#ifdef EBUG
#ifndef HACK
    std::ofstream fout("output.txt"); // 测试用输出
#else
    std::ofstream fout("outputHACK.txt"); // 测试用输出
#endif
    void quit() {
        fout << "lossCollMoney: " << lossCollMoney << std::endl;
        fout << "lossTimeMoney: " << lossTimeMoney << std::endl;
        fout.close();
    }
#endif
#ifdef EBUG
    #define TESTOUTPUT(x) x
#else
    #define TESTOUTPUT(x) 
#endif
#ifdef CREATE
    #define CREATEMAP(x) x
#else
    #define CREATEMAP(x)
#endif
#ifdef CREATE
    std::ofstream mapOut("../genMap/path.txt"); // 测试用输出
#endif

#endif