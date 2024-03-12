#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <iostream>
#include <cmath>
#include <algorithm>

// 定义二维向量结构体
struct Pos {
    int x, y;
    Pos(int x = 0, int y = 0) : x(x), y(y) {}
    Pos operator+(Pos v) { return Pos(x + v.x, y + v.y); }
    Pos operator-(Pos v) { return Pos(x - v.x, y - v.y); }
    bool operator==(const Pos v)const {// 重载等于号
        return x == v.x && y == v.y;
    } 
};


#endif