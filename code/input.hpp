#ifndef __INPUT_H__
#define __INPUT_H__
#include "config.hpp"
#include "grid.hpp"
#include "robot.hpp"
#include "ship.hpp"
#include "berth.hpp"
#include "item.hpp"
#include "precode_grid.hpp"

void inputMap(){
    for (int i = 0; i < MAX_Line_Length; i++) {
        std::string line;
        getline(std::cin, line);
        for (int j = 0; j < MAX_Col_Length; j++) {
            if (line[j] == '.') {
                grids[i][j] = new Grid(i, j, 0);
            } else if (line[j] == '*') {
                grids[i][j] = new Grid(i, j, 1);
            } else if (line[j] == '#') {
                grids[i][j] = new Grid(i, j, 2);
            } else if (line[j] == 'A') {
                robotNum++;
                robots[robotNum] = new Robot(robotNum, i, j);
                grids[i][j] = new Grid(i, j, 0);
            } else if (line[j] == 'B') {
                grids[i][j] = new Grid(i, j, 3);
            }
        }
    }
    for (int i = 0; i < MAX_Berth_Num; i++) {
        int id, x, y, time, velocity;
        scanf("%d%d%d%d%d", &id, &x, &y, &time, &velocity);
        berths[i] = new Berth(id, x, y, time, velocity);
        TESTOUTPUT(fout << "Berth" << i << " :" << id << " " << x << " " << y << " " << time << " " << velocity << std::endl;)
    }
    std::cin >> MAX_Capacity;
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    for (int i = 0; i < MAX_Ship_Num; i++) ships[i] = new Ship(i);
    shipNum = MAX_Ship_Num - 1;
    sovle_grid();
    puts("OK");
    // TESTOUTPUT(fout << "inputMap() OK" << std::endl;)
    // TESTOUTPUT(fout << "sovle_grid() OK" << std::endl;)
    // for(int i = 0; i<MAX_Line_Length; i++){
    //     for(int j = 0; j<MAX_Col_Length; j++){
    //         TESTOUTPUT(fout << reachable_grids[i][j]->type << " ");
    //     }
    //     TESTOUTPUT(fout << std::endl;)
    // }
    // for(int i = 0; i<MAX_Line_Length; i++){
    //     for(int j = 0; j<MAX_Col_Length; j++){
    //         TESTOUTPUT(fout << min_dis[i][j][0] << " ");
    //     }
    //     TESTOUTPUT(fout << std::endl;)
    // }
    fflush(stdout);
}


bool inputFrame() {
    if (scanf("%d%d",&nowTime, &money ) == EOF) {
        inputFlag = false;
        return false;
    }
    int K;
    scanf("%d", &K);
    for (int i = 1; i <= K; i++) {
        int x, y, value;
        scanf("%d%d%d", &x, &y, &value);
        unsolvedItems.emplace_back(x, y, value);
        unsolvedItems.back().beginTime = nowTime;
    }
    for (int i = 0; i <= robotNum; i++) {
        scanf("%d%d%d%d", &robots[i]->bring, &robots[i]->pos.x, &robots[i]->pos.y, &robots[i]->status);
    }
    for (int i = 0; i <= shipNum; i++) {
        scanf("%d%d", &ships[i]->status, &ships[i]->berthId);
    }
    std::string line;
    while(getline(std::cin, line) && line != "OK");
    return true;
}

void solveFrame() {
    TESTOUTPUT(fout <<"当前帧数="<< nowTime << std::endl;)
    for (int i = 0; i <= robotNum; i++) robots[i]->action();
    for (int i = 0; i <= robotNum; i++) robots[i]->move();
    for (int i = 0; i <= shipNum; i++) ships[i]->action();

    puts("OK");
    fflush(stdout);
}
#endif