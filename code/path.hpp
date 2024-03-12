#ifndef __PATH_H__
#define __PATH_H__

#include "config.hpp"

struct Path {
    std::vector<Pos> path;
    Pos begin;
    Pos end;
    Path() {
        this->begin = Pos(-1, -1);
        this->end = Pos(-1, -1);
    }
};

#endif