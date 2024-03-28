#include "config.hpp"
#include "input.hpp"

int main() {
#ifdef TUNE
    std::ifstream fin("para.txt");
    fin >> MAX_Berth_Control_Length;
    fin >> MAX_Berth_Merge_Length;
    fin >> Worst_Rate;
    fin >> Sell_Ration;
    fin >> Min_Next_Berth_Value;
    fin >> Only_Run_On_Berth_with_Ship;
    fin >> lastRoundRuningTime;
#endif
    inputMap();
    while (1) {
        inputFrame();
        if (inputFlag == false) break;
        solveFrame();
    }
    return 0;
}
