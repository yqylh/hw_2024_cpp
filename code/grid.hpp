#ifndef __GRID_H__
#define __GRID_H__

#include "config.hpp"
struct Navigator {
    std::bitset<BitsetSize> visited; // 用于记录是否访问过
    std::bitset<BitsetSize * 2> dirNext; // 用于记录方向 00 表示右移一格 01 表示左移一格 10 表示上移一格 11 表示下移一格
    Navigator(){
        visited.reset();
        dirNext.reset();
    }
    int getVisitedIndex(int x, int y) {
        return x * MAX_Col_Length + y;
    }
    int getDirIndex(int x, int y) {
        return x * MAX_Col_Length * 2 + y * 2;
    }
    void setVisited(int x, int y) {
        visited[getVisitedIndex(x, y)] = 1;
    }
    bool isVisited(int x, int y) {
        return visited[getVisitedIndex(x, y)];
    }
    void setDir(int x, int y, int d) {
        int index = getDirIndex(x, y);
        if (d & 1) dirNext[index] = 1;
        if (d & 2) dirNext[index + 1] = 1;
    }
    int getDir(int x, int y) {
        int ans = 0;
        int index = getDirIndex(x, y);
        if (dirNext[index] == 1) ans += 1;
        if (dirNext[index + 1] == 1) ans += 2;
        return ans;
    }
};

/**
 * @brief 网格类
 * 解释一些 type 的类型
 * 0‘.’ ： 空地                                   | 机器人可以走  会发生碰撞
 * 1‘*’ ： 海洋                                   | 船可以走  会发生碰撞
 * 2‘#’ ： 障碍                                   | 谁都不可以走
 * 3‘B’ ： 泊位  同时算主干道主航道                  | 船和机器人都可以走 不会发生碰撞
 * 
 * 4‘>’ ： 陆地主干道                              | 机器人可以走 不会发生碰撞
 * 5‘~’ ： 海洋主航道                              | 船可以走 不会发生碰撞
 * 6‘R’ ： 机器人购买地块，同时该地块也是主干道.        | 机器人可以走, 不会发生碰撞
 * 7‘S’ ： 船舶购买地块，同时该地块也是主航道           | 船可以走, 不会发生碰撞
 * 
 * 8‘K’ ： 靠泊区 算主航道                          | 船可以走 不会发生碰撞
 * 9‘C’ ： 海陆立体交通地块                         | 船和机器人都可以走 会发生碰撞
 * 10 : 机器人
 * 11‘c’ ： 海陆立体交通地块，同时为主干道和主航道      |船和机器人都可以走 不会发生碰撞

 * 12‘T’ ： 交货点 特殊的靠泊区 所以也算主航道         | 船可以走 不会发生碰撞

*/


/*
#define BLANK 0b0000 0000 0000 0001 
#define SEA 0b0000 0000 0000 0010
#define OBSTACLE 0b0000 0000 0000 0100
#define BERTH 0b0000 0000 0000 1000 // ROBOT_NOCOL // SJHIP_NOCOL

#define ROUTE 0b0000 0000 0001 0000 // ROBOT_NOCOL
#define SEA_ROUTE 0b0000 0000 0010 0000 // SHIP_NOCOL
#define ROBOT_BUYER 0b0000 0000 0100 0000 // ROBOT_NOCOL
#define SHIP_BUYER 0b0000 0000 1000 0000 // SHIP_NOCOL

#define BERTH_CENTER 0b0000 0001 0000 0000 // SHIP_NOCOL
#define CROSS 0b0000 0010 0000 0000
#define CROSS_ROUTE 0b0000 1000 0000 0000 // ROBOT_NOCOL // SHIP_NOCOL
#define DELIVERY 0b0001 0000 0000 0000 // SHIP_NOCOL

#define SHIP_NOCOL 0b0001 1001 1010 1000
#define ROBOT_NOCOL 0b0000 1000 0101 1000

#define ROBOT_ABLE 0b0000 1010 0101 1001
#define SHIP_ABLE 0b0001 1011 1010 1010
*/

#define BLANK 0b0000000000000001 
#define SEA 0b0000000000000010
#define OBSTACLE 0b0000000000000100
#define BERTH 0b0000000000001000 // ROBOT_NOCOL // SJHIP_NOCOL

#define ROUTE 0b0000000000010000 // ROBOT_NOCOL
#define SEA_ROUTE 0b0000000000100000 // SHIP_NOCOL
#define ROBOT_BUYER 0b0000000001000000 // ROBOT_NOCOL
#define SHIP_BUYER 0b0000000010000000 // SHIP_NOCOL

#define BERTH_CENTER 0b0000000100000000 // SHIP_NOCOL
#define CROSS 0b0000001000000000
#define CROSS_ROUTE 0b0000100000000000 // ROBOT_NOCOL // SHIP_NOCOL
#define DELIVERY 0b0001000000000000 // SHIP_NOCOL

#define SHIP_NOCOL 0b0001100110101000
#define ROBOT_NOCOL 0b0000100001011000

#define ROBOT_ABLE 0b0000101001011001
#define SHIP_ABLE 0b0001101110101010

#define NOT_VALID_GRID(x, y) (x < 0 or x >= MAX_Line_Length or y < 0 or y >= MAX_Col_Length)
#define IS_SHIP_NOCOL(x) (x & SHIP_NOCOL)
#define IS_ROBOT_NOCOL(x) (x & ROBOT_NOCOL)
#define IS_ROBOT_ABLE(x) (x & ROBOT_ABLE)
#define IS_SHIP_ABLE(x) (x & SHIP_ABLE)

struct Grid {
    Pos pos; // 位置
    int type;
    bool robotOnIt;
    Navigator *gridDir; // 用来导航从起点到终点的路径
    int berthId; // 如果是泊位或者靠泊区,则记录泊位的 id
    int bit_type;
    int shipAble[4]; // 记录这个位置四个方向是否船是否可以停在这里

    int belongToBerth; // 这个格子是属于哪个港口的独占区，-1表示公共区域
    Grid(){
        this->pos = Pos(-1, -1);
        this->type = -1;
        this->gridDir = nullptr;
        this->robotOnIt = false;
        this->berthId = -1;
        this->bit_type = 0;
        this->belongToBerth = -1;
    }
    Grid(int x, int y, int type) : type(type){
        this->pos = Pos(x, y);
        this->gridDir = nullptr;
        this->robotOnIt = false;
        this->berthId = -1;
        this->bit_type = 1 << type;
        this->belongToBerth = -1;
    }
};

Grid *grids[MAX_Line_Length + 1][MAX_Col_Length + 1];
/**
 * @brief 检查位置是否合法
 * @param pos 表示位置
 * @return true 表示合法, false 表示不合法
*/
bool checkPos(Pos pos) {
    return pos.x >= 0 && pos.x < MAX_Line_Length && pos.y >= 0 && pos.y < MAX_Col_Length;
}
/**
 * @brief 检查机器人是否可以走这个位置
 * @param pos 表示位置
 * @return true 表示可以走, false 表示不可以走
*/
bool checkRobotAble(Pos pos) {
    if (checkPos(pos) == false) return false;
    auto type = grids[pos.x][pos.y]->type;
    return (type == 0 || type == 3 || type == 4 || type == 6 || type == 9 || type == 11);
}
/**
 * @brief 检查船是否可以走这个位置
 * @param pos 表示位置
 * @return true 表示可以走, false 表示不可以走
*/
bool checkShipAble(Pos pos) {
    if (checkPos(pos) == false) return false;
    auto type = grids[pos.x][pos.y]->type;
    return (type == 1 || type == 3 || type == 5 || type == 7 || type == 8 || type == 9 || type == 11 || type == 12);
}
/**
 * @brief 检查这个位置是否是主道路
 * @param pos 表示位置
 * @return true 表示是主道路, false 表示不是主道路
*/
bool checkRobotNoColl(Pos pos) {
    if (checkPos(pos) == false) return false;
    auto type = grids[pos.x][pos.y]->type;
    return (type == 3 || type == 4 || type == 6 || type == 11);
}
/**
 * @brief 检查这个位置是否是主航道
 * @param pos 表示位置
 * @return true 表示是主航道, false 表示不是主航道
*/
bool checkShipNoColl(Pos pos) {
    if (checkPos(pos) == false) return false;
    auto type = grids[pos.x][pos.y]->type;
    return (type == 3 || type == 5 || type == 7 || type == 8 || type == 11 || type == 12);
}

/**
 * @brief 计算船的旋转后的核心点位置
 * @param dir表示原始的方向, 0 到 3 分别对应右、左、上、下。（和机器人移动的方向表示一致）
 * @param rot表示旋转的方向, 0 表示顺时针, 1 表示逆时针
 * @return std::pair<Pos, int> 返回旋转后的核心点位置和旋转后的方向
 * 
*/
ShipPos calShipRotPos(Pos pos, int dir, int rot) {
    // 顺时针
    if (rot == 0) {
        if (dir == 0) {
            return ShipPos(pos + Pos(0, 2), 3);
        } else if (dir == 1) {
            return ShipPos(pos + Pos(0, -2), 2);
        } else if (dir == 2) {
            return ShipPos(pos + Pos(-2, 0), 0);
        } else if (dir == 3) {
            return ShipPos(pos + Pos(2, 0), 1);
        }
    } else {
        // 逆时针
        if (dir == 0) {
            return ShipPos(pos + Pos(1, 1) , 2);
        } else if (dir == 1) {
            return ShipPos(pos + Pos(-1, -1) , 3);
        } else if (dir == 2) {
            return ShipPos(pos + Pos(-1, 1) , 1);
        } else if (dir == 3) {
            return ShipPos(pos + Pos(1, -1) , 0);
        }
    }
    return ShipPos(-1, -1, -1);
}
/**
 * @brief 计算船的六个位置
 * @param pos 表示船的核心点
 * @param dir 表示船的方向
 * @return std::vector<Pos> 返回船的六个位置
*/
std::vector<Pos> getShipAllPos(Pos pos, int dir) {
    std::vector<Pos> ret;
    if (dir == 0) {
        ret.push_back(pos);
        ret.push_back(pos + Pos(0, 1));
        ret.push_back(pos + Pos(0, 2));
        ret.push_back(pos + Pos(1, 0));
        ret.push_back(pos + Pos(1, 1));
        ret.push_back(pos + Pos(1, 2));
    } else if (dir == 1) {
        ret.push_back(pos);
        ret.push_back(pos + Pos(0, -1));
        ret.push_back(pos + Pos(0, -2));
        ret.push_back(pos + Pos(-1, 0));
        ret.push_back(pos + Pos(-1, -1));
        ret.push_back(pos + Pos(-1, -2));
    } else if (dir == 2) {
        ret.push_back(pos);
        ret.push_back(pos + Pos(-1, 0));
        ret.push_back(pos + Pos(-2, 0));
        ret.push_back(pos + Pos(0, 1));
        ret.push_back(pos + Pos(-1, 1));
        ret.push_back(pos + Pos(-2, 1));
    } else if (dir == 3) {
        ret.push_back(pos);
        ret.push_back(pos + Pos(1, 0));
        ret.push_back(pos + Pos(2, 0));
        ret.push_back(pos + Pos(0, -1));
        ret.push_back(pos + Pos(1, -1));
        ret.push_back(pos + Pos(2, -1));
    }
    return ret;
}
/**
 * @brief 检查船六个点是否都可以走
 * @param pos 表示船的核心点
 * @return 0 表示不能走, 1 表示可以走(正常道路), 2 表示主航道道路
*/
int checkShipAllAble(Pos pos, int dir) {
    auto allPos = getShipAllPos(pos, dir);
    int ret = 1;
    for (auto &p : allPos) {
        if (checkShipAble(p) == false) return 0;
        if (checkShipNoColl(p)) ret = 2;
    }
    return ret;
}
void PreproShipAllAble() {
    for (int x = 0; x < MAX_Line_Length; x++) {
        for (int y = 0; y < MAX_Col_Length; y++) {
            for (int i = 0; i < 4; i++) {
                grids[x][y]->shipAble[i] = checkShipAllAble(Pos(x, y), i);
            }
        }
    }
}

int _dis_s[MAX_Line_Length + 1][MAX_Col_Length + 1][4];
ShipPos _pre_s[MAX_Line_Length + 1][MAX_Col_Length + 1][4];
int _pre_dir_s[MAX_Line_Length + 1][MAX_Col_Length + 1][4];
/**
 * @brief 计算船的最短路径
 * 2. 三方向 SPFA：直行2、顺时针0、逆时针1
 * 3. SPFA 每个点要存某个方向有没有到达。已经到达的时间，因为主航道 delay
 * 4. 存路径：直行2、顺时针0、逆时针1
*/
ShipPos _queue_ship[120010];
std::deque<int> *sovleShip(Pos origin, int direction, Pos target, bool needPath = true) {
    allPathLogger.log(nowTime, "sovleShip origin{},{} direction{} target{},{}", origin.x, origin.y, direction, target.x, target.y);
    for (int i = 0; i < MAX_Line_Length; i++) {
        for (int j = 0; j < MAX_Col_Length; j++) {
            for (int k = 0; k < 4; k++) {
                _dis_s[i][j][k] = INT_MAX;
                _pre_s[i][j][k] = ShipPos(-1, -1, -1);
                _pre_dir_s[i][j][k] = -1;
            }
        }
    }
    int start = 0;
    int end = 0;
    _queue_ship[end++] = ShipPos(origin, direction);
    _dis_s[origin.x][origin.y][direction] = 0;
    while (start != end) {
        ShipPos now = _queue_ship[start++];
        if (start == 120010) start = 0;
        // 直行
        ShipPos next = now + dir[now.direction];
        // 判断是否可以走 0 表示不能走, 1 表示可以走(正常道路), 2 表示主航道道路
        auto checkRes = grids[next.x][next.y]->shipAble[next.direction];
        if (checkRes && _dis_s[next.x][next.y][next.direction] > _dis_s[now.x][now.y][now.direction] + checkRes) {
            _dis_s[next.x][next.y][next.direction] = _dis_s[now.x][now.y][now.direction] + checkRes;
            _pre_s[next.x][next.y][next.direction] = now;
            _pre_dir_s[next.x][next.y][next.direction] = 2;
            _queue_ship[end++] = next;
            if (end == 120010) end = 0;
        }
        // 顺时针
        next = calShipRotPos(now.toPos(), now.direction, 0);
        checkRes = grids[next.x][next.y]->shipAble[next.direction];
        if (checkRes && _dis_s[next.x][next.y][next.direction] > _dis_s[now.x][now.y][now.direction] + checkRes) {
            _dis_s[next.x][next.y][next.direction] = _dis_s[now.x][now.y][now.direction] + checkRes;
            _pre_s[next.x][next.y][next.direction] = now;
            _pre_dir_s[next.x][next.y][next.direction] = 0;
            _queue_ship[end++] = next;
            if (end == 120010) end = 0;
        }
        // 逆时针
        next = calShipRotPos(now.toPos(), now.direction, 1);
        checkRes = grids[next.x][next.y]->shipAble[next.direction];
        if (checkRes && _dis_s[next.x][next.y][next.direction] > _dis_s[now.x][now.y][now.direction] + checkRes) {
            _dis_s[next.x][next.y][next.direction] = _dis_s[now.x][now.y][now.direction] + checkRes;
            _pre_s[next.x][next.y][next.direction] = now;
            _pre_dir_s[next.x][next.y][next.direction] = 1;
            _queue_ship[end++] = next;
            if (end == 120010) end = 0;
        }
    }
    std::deque<int> *result = new std::deque<int>;
    // 首先找到重点的四个方向里 dis 最小的
    ShipPos now = ShipPos(target, 0);
    for (int i = 1; i < 4; i++) {
        if (_dis_s[target.x][target.y][i] < _dis_s[now.x][now.y][now.direction]) {
            now.direction = i;
        }
    }
    // std::string allPos = std::to_string(target.x) + "," + std::to_string(target.y) + "," + std::to_string(now.direction) + "<";
    // std::string allDir = std::to_string(_pre_dir_s[now.x][now.y][now.direction]) + "<";
    // 每次找到前一个的位置
    while (true && needPath) {
        result->push_front(_pre_dir_s[now.x][now.y][now.direction]);
        now = _pre_s[now.x][now.y][now.direction];
        if (now.toPos() == origin && now.direction == direction) break;
        // allPos += std::to_string(now.x) + "," + std::to_string(now.y) + "," + std::to_string(now.direction) + "<";
        // allDir += std::to_string(_pre_dir_s[now.x][now.y][now.direction]) + "<";
    }
    // allPathLogger.log(nowTime, "allPos:{}", allPos);
    // allPathLogger.log(nowTime, "allDir:{}", allDir);
    return result;
}

Pos _arr[40010];
Navigator * sovleGrid(Pos origin) {
    Navigator * result = new Navigator;
    result->setVisited(origin.x, origin.y);
    Pos _arr[40010];
    int start = 0, end = 0;
    _arr[end++] = origin;
    while (start < end) {
        Pos &now = _arr[start++];
        for (int i = 0; i < 4; i++) {
            Pos next = now + dir[i]; // 下一个点
            if (checkRobotAble(next) == false) continue; // 不是机器人可以走的地方
            if (result->isVisited(next.x, next.y)) continue; //记录过前序, 跳过
            result->setVisited(next.x, next.y);
            result->setDir(next.x, next.y, ((i == 0 || i == 2) ? i + 1 : i - 1));
            _arr[end++] = next;
        }
    }
    return result;
}

#endif