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
#include <unordered_map>
#include <climits>
#include <queue>
#include <list>

/**
 * 常量定义
 * 用于定义比赛中一些基本不会变动的参数
*/
const int MAX_Robot_Num = 10; // 最大机器人数
const int MAX_Ship_Num = 5; // 最大船数
const int MAX_Berth_Num = 10; // 最大泊位数
const int MAX_Line_Length = 200; // 地图有多少行
const int MAX_Col_Length = 200; // 地图有多少列
const int MAX_TIME = 5 * 60 * 50; // 最大帧数
const int Item_Continue_Time = 1000; // 物品持续时间
const int Ship_Move_Time = 500; // 船移动时间

/**
 * 全局变量
 * 用于存储控制整体流程的一些变量
*/

int robotNum = -1; // 当前机器人标号,实际数量-1, 0-robotNum
int shipNum = -1; // 当前船标号,实际数量-1, 0-shipNum
int money = 0; // 当前金钱
int nowTime = 0; // 当前帧数
bool inputFlag = true; // 是否input是否结束
int capacity; // 船的容量

#ifdef EBUG
    #define TESTOUTPUT(x) x
    std::ofstream fout("output.txt"); // 测试用输出
#else
    #define TESTOUTPUT(x) 
#endif

#ifdef CREATE
    #define CREATEMAP(x) x
    std::ofstream mapOut("../genMap/path.txt"); // 测试用输出
#else
    #define CREATEMAP(x)
#endif

#endif