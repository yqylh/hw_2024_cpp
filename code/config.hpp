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
#include <unordered_set>
#include <climits>
#include <queue>
#include <list>
#include <chrono>
#include <thread>
#include <bitset>
#include <memory>
#include <atomic>
#include <ctime>
#include "pos.hpp"
using namespace std::chrono;
#include "logger.hpp"

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
int MAX_Capacity; // 船的容量
#define BitsetSize 40000

/**
 * 全局变量
 * 用于存储控制整体流程的一些变量
*/

int robotNum = -1; // 当前机器人标号,实际数量-1, 0-robotNum
int shipNum = -1; // 当前船只标号,实际数量-1, 0-shipNum
int money = 0; // 当前金钱
int nowTime = 0; // 当前帧数
bool inputFlag = true; // 是否input是否结束

/**
 * 预处理&多线程控制相关变量
*/
auto programStart = high_resolution_clock::now(); // 计时器


/**
 * 超参数
*/
int MAX_Berth_Control_Length = 160; // 机器人搜索长度,用来判断私有区域, 10~200, 5
int MAX_Berth_Merge_Length = 80; // 泊位合并长度,用来判断是否可以合并, 1~200, 5
int Worst_Rate = 3; // 用来筛选多差的港口不要选, 1~10
double Sell_Ration = 0.85; // 还剩多少港口空了就去卖, 0.5~1
int Min_Next_Berth_Goods = 10; // another 港口的货物少于这个值就不去, 0~100
int Last_Round_delay_time = 4500; // 预留给最后一轮的时间,含去 回 去

Pos dir[4] = {Pos(0, 1), Pos(0, -1), Pos(-1, 0), Pos(1, 0)};
// 0 表示右移一格 1 表示左移一格 2 表示上移一格 3 表示下移一格
std::unordered_map<Pos, int> Pos2move = {
    {Pos(0, 1), 0},
    {Pos(0, -1), 1},
    {Pos(-1, 0), 2},
    {Pos(1, 0), 3}
};

/**
 * 日志记录
 * 用于全局的日志器
 * 建议写日志时少用空格，使用逗号分隔
*/


FileLogger shipLogger("../log/ship_log.txt");
FileLogger robotLogger("../log/robot_log.txt");
FileLogger berthLogger("../log/berth_log.txt");
FileLogger itemLogger("../log/item_log.txt");
FileLogger centerLogger("../log/center_log.txt");
// 用来输出每一帧和各个主要函数的开始和结束，还有运行时间，主要是用来定位 bug 位置
FileLogger flowLogger("../log/flow_log.txt");
FileLogger bcenterlogger("../log/bcenter_log.txt");
FileLogger pathLogger("../log/path_log.txt");
FileLogger allPathLogger("../log/allPath_log.txt");

#ifdef DEBUG
    #define TEST(x) x
    std::ofstream fout("output.txt"); // 测试用输出
#else
    #define TEST(x) 
#endif

#ifdef CREATE
    #define CREATEMAP(x) x
    std::ofstream mapOut("../genMap/path.txt"); // 测试用输出
#else
    #define CREATEMAP(x)
#endif

#endif