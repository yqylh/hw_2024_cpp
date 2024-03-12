#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <iostream>
#include <cmath>
#include <algorithm>

// 定义二维向量结构体
struct Vector2D {
    double x, y;
    bool status;
    Vector2D(double x = 0, double y = 0) : x(x), y(y) { status = true; }
    Vector2D operator+(Vector2D v) { return Vector2D(x + v.x, y + v.y); }
    Vector2D operator-(Vector2D v) { return Vector2D(x - v.x, y - v.y); }
    Vector2D operator*(double s) { return Vector2D(x * s, y * s); }
    Vector2D operator/(double s) { return Vector2D(x / s, y / s); }
    double operator*(Vector2D v) { return x * v.x + y * v.y; } // 点积
    double operator^(Vector2D v) { return x * v.y - y * v.x; } // 叉积
    double length() { return sqrt(x * x + y * y); } // 向量长度
    void normalize() { *this = *this / length(); } // 向量单位化
    double angle(Vector2D v) { return acos((*this * v) / (length() * v.length())); } // 向量夹角
    bool operator<(const Vector2D v)const { return x < v.x || (x == v.x && y < v.y); } // 重载小于号
    bool operator==(const Vector2D v)const {// 重载等于号
        return std::abs(x - v.x) < 0.000001 && std::abs(y - v.y) < 0.000001;
    } 
};


#endif