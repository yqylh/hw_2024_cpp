#ifndef __ITEM_H__
#define __ITEM_H__
#include "config.hpp"

struct Item {
    int x, y;
    int value;
    Item(int x, int y, int value) : x(x), y(y), value(value) {}
    Item() {}
};
std::list<Item> unsolvedItems;
std::list<Item> solvingItems;

#endif