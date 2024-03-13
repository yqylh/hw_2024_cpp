#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <iostream>
#include <cmath>
#include <algorithm>

// 定义二维向量结构体
struct Pos {
    int x, y;
    int cmp;
    Pos(int x = 0, int y = 0) : x(x), y(y) {}
    Pos operator+(const Pos v)const { return Pos(x + v.x, y + v.y); }
    Pos operator-(const Pos v)const { return Pos(x - v.x, y - v.y); }
    bool operator<(const Pos v)const {// 重载小于号
        return cmp < v.cmp;
    }
    bool operator==(const Pos v)const {// 重载等于号
        return x == v.x && y == v.y;
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
#endif