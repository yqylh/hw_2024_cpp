#ifndef __ITEM_H__
#define __ITEM_H__
#include "config.hpp"

struct Item {
    Pos pos;
    int value;
    int beginTime;
    Item(int x, int y, int value) : value(value) {
        this->pos = Pos(x, y);
    }
    Item() {}
    bool checkDead() {
        return nowTime - beginTime > Item_Continue_Time;
    }
};
std::list<Item> unsolvedItems;

#endif