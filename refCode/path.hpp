#ifndef _PATH_H_
#define _PATH_H_

struct Path{
    int buyWorktableId;
    int sellWorktableId;
    int robotId;
    double earnedMoney;
    double time;
    double parameters;
    Path(int buyWorktableId, int sellWorktableId, int robotId, double earnedMoney, double time) {
        this->buyWorktableId = buyWorktableId;
        this->sellWorktableId = sellWorktableId;
        this->robotId = robotId;
        this->earnedMoney = earnedMoney;
        this->time = time;
        this->parameters = earnedMoney / time;
    }
};

#endif