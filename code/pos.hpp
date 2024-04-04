#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <iostream>
#include <cmath>
#include <algorithm>

// 定义二维向量结构体
struct Pos {
    int x, y;
    Pos(int x = -1, int y = -1) : x(x), y(y) {}
    Pos operator+(const Pos v)const { return Pos(x + v.x, y + v.y); }
    Pos operator-(const Pos v)const { return Pos(x - v.x, y - v.y); }
    bool operator==(const Pos v)const {// 重载等于号
        return x == v.x && y == v.y;
    }
    bool operator!=(const Pos v)const {// 重载不等于号
        return x != v.x || y != v.y;
    }
    int length(const Pos v)const {// 计算两点之间的距离
        return abs(x - v.x) + abs(y - v.y);
    }
};


namespace std {
    template <>
    struct hash<Pos> {
        std::size_t operator()(const Pos& obj) const {
            std::string temp = std::to_string(obj.x) + std::to_string(obj.y);
            return std::hash<std::string>{}(temp);
        }
    };
}

struct ShipPos {
    int x, y, direction;
    ShipPos(int x = -1, int y = -1, int direction = -1) : x(x), y(y), direction(direction) {}
    ShipPos(Pos pos, int direction) : x(pos.x), y(pos.y), direction(direction) {}
    ShipPos operator+(const Pos v)const { return ShipPos(x + v.x, y + v.y, direction); }
    bool operator==(const ShipPos v)const {
        return x == v.x && y == v.y && direction == v.direction;
    }
    bool operator!=(const ShipPos v)const {
        return x != v.x || y != v.y || direction != v.direction;
    }
    Pos toPos() {
        return Pos(x, y);
    }
};
namespace std {
    template <>
    struct hash<ShipPos> {
        std::size_t operator()(const ShipPos& obj) const {
            std::string temp = std::to_string(obj.x) + "_" + std::to_string(obj.y) + "_" + std::to_string(obj.direction);
            return std::hash<std::string>{}(temp);
        }
    };
}

#endif