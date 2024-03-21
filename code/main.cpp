#include "config.hpp"
#include "input.hpp"

int main() {
#ifdef TUNE
    std::ifstream fin("para.txt");
    fin >> MAX_Berth_Control_Length;
    fin >> MAX_Berth_Merge_Length;
    fin >> Sell_Ration;
    fin >> Min_Next_Berth_Value;
#endif
    inputMap();
    while (1) {
        inputFrame();
        if (inputFlag == false) break;
        solveFrame();
    }
    return 0;
}
