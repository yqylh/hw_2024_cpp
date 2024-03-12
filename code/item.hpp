#ifndef __ITEM_H__
#define __ITEM_H__
#include "config.hpp"

struct Item {
    Pos pos;
    int value;
    Item(int x, int y, int value) : value(value) {
        this->pos = Pos(x, y);
    }
    Item() {}
};
std::list<Item> unsolvedItems;
std::list<Item> solvingItems;

#endif