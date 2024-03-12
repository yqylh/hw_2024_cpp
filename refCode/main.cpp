#include <iostream>
#include <fstream>
#include <thread>
#include "config.hpp"
#include "robot.hpp"
#include "worktable.hpp"
#include "input.hpp"

int main() {
    inputMap();
    while (1) {
        std::thread tInput(inputFrame);
        tInput.join();
        if (inputFlag == false) break;
        std::thread t1(solveFrame);
        t1.join();
    }
    TESTOUTPUT(quit();)
    return 0;
}
