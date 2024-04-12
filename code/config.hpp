#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <fstream>
#include <set>
#include <map>
#include <cmath>
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
#include "count.hpp"
#include <functional>
#include <type_traits>

/**
 * 常量定义
 * 用于定义比赛中一些基本不会变动的参数
*/
const int MAX_Line_Length = 200; // 地图有多少行
const int MAX_Col_Length = 200; // 地图有多少列
const int MAX_TIME = 5 * 60 * 50; // 最大帧数
const int Item_Continue_Time = 1000; // 物品持续时间
#define BitsetSize 40000

/**
 * 全局变量
 * 用于存储控制整体流程的一些变量
*/
int MAX_Robot_Num = 0; // 机器人数量 [)
int MAX_Ship_Num = 0; // 船的数量 [)
int MAX_Berth_Num = 0; // 泊位数量 [)
int MAX_Capacity; // 船的容量
int money = 25000; // 当前金钱
int nowTime = 0; // 当前帧数
bool inputFlag = true; // 是否input是否结束
std::vector<int> robotPriority; // 机器人的优先级
int priorityTimeControl = -1; // 优先级控制时间
std::unordered_map<Pos, bool> unMoveShip; // 不能移动的船
std::unordered_map<int, std::unordered_map<Pos, bool>> shipPos; // 船的位置

/**
 * 预处理&多线程控制相关变量
*/
auto programStart = high_resolution_clock::now(); // 计时器


/**
 * 超参数  160 80 0.85 200 是一组经典参数,适合大部分地图能跑一个不错的成绩
 * 孤军奋战: 40 1 0.7 800
 * 全员一锅粥: 40 500 0.7 800
 * MAX_Berth_Control_Length的建议参数 40 左右
 * MAX_Berth_Merge_Length的建议参数 1 / 40 / 80
 * Sell_Ration的建议参数 0.7 左右
 * Min_Next_Berth_Value的建议参数 0, 200, 800, 1000
*/
int MAX_Berth_Control_Length = 160; // 机器人搜索长度,用来判断私有区域, 10~200, 5
int MAX_Berth_Merge_Length = 80; // 泊位合并长度,用来判断是否可以合并, 1~200, 5
int Worst_Rate = 3; // 用来筛选多差的港口不要选, 1~10
double Sell_Ration = 0.7; // 还剩多少港口空了就去卖, 0.5~1
int Min_Next_Berth_Value = 1700; // another 港口的货物价值少于这个值就不去, 0~1000
int Only_Run_On_Berth_with_Ship = 350; // 最后这些帧,只在有船的泊位上运行,
int lastRoundRuningTime = 600; // 估计的最后一轮的运行时间

// 暂时不要调的参数,不一定有用 | 策略已经放弃了
int Min_Next_Berth_Goods = 10; // another 港口的货物少于这个值就不去, 0~100
int Last_Round_delay_time = 4500; // 预留给最后一轮的时间,含去 回 去

int _maxRobotCnt = 17;
int _maxShipCnt = 2;

const double _itemAtEnd = 4800;
const double _pulledItemAtEnd = 1900;

int exptRobotCnt = 0;

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
FileLogger estimatorLogger("../log/estimator_log.txt");

#ifdef DEBUG
    #define TEST(x) x
    std::ofstream fout("output.txt"); // 测试用输出
    std::ofstream tout("time.txt");
#else
    #define TEST(x) 
#endif

#ifdef DEBUG
    Counter counter;
#else
    Void_Counter counter;
#endif

#ifdef CREATE
    #define CREATEMAP(x) x
    std::ofstream mapOut("../genMap/path.txt"); // 测试用输出
#else
    #define CREATEMAP(x)
#endif

template<typename T>
void bugs_output(T x) {
#ifdef DEBUG
    fout << x << " ";
#endif
}
template<typename... Args>
void bugs(Args... args) {
#ifdef DEBUG
    (..., bugs_output(args));
    fout << std::endl;
#endif
}
class Timer {
public:
#ifdef DEBUG
    template<typename Func, typename... Args>
    static auto measure(const std::string& description, Func func, Args&&... args) {
        auto start = std::chrono::high_resolution_clock::now();
        
        if constexpr (std::is_same_v<std::invoke_result_t<Func, Args...>, void>) {
            // If the function returns void
            func(std::forward<Args>(args)...);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            tout << nowTime <<" "<< description << "\t - Elapsed time: \t" << elapsed.count() * 1000 << "\tmirco seconds.\n";
        } else {
            // If the function returns a value
            auto result = func(std::forward<Args>(args)...);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            tout << nowTime <<" "<< description << "\t - Elapsed time: \t" << elapsed.count() * 1000 << "\tmicro seconds.\n";
            return result;
        }
    }
#else
    template<typename Func, typename... Args>
    static auto measure(const std::string& description, Func func, Args&&... args) -> decltype(func(std::forward<Args>(args)...)) {
        return func(std::forward<Args>(args)...);
    }
#endif
};
#ifdef DEBUG
void measureAndExecute(const std::string& description, const std::function<void()>& action) {
    auto start = std::chrono::high_resolution_clock::now();
    action();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    tout << description << " - Elapsed time: " << elapsed.count() << " ms\n";
}
#else
void measureAndExecute(const std::string& description, const std::function<void()>& action) {
    action();
}
#endif

#endif