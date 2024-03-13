seed=123
map=map1
debug=1
# 如果是 python,上面几个参数改成默认值. debug可以改成是否包含'debug' word
compile= ;
if [ $debug -eq 1 ] ; then
    compile="-g -DEBUG"
fi

# 如果文件夹不存在
if [ ! -d "../judge/replay" ]; then
    mkdir ../judge/replay
fi
if g++ main.cpp -o main -std=c++17 -O3 $compile; then
    ../judge/PreliminaryJudge -s $seed -m ../allMaps/$map.txt -r ./$map%Y-%m-%d.%H.%M.%S.rep ./main
    mv ./replay/* ../judge/replay/
    if [ -f "main" ]; then
        rm main
    fi
    if [ -d "main.dSYM" ]; then
        rm -r ./main.dSYM
    fi
    if [ -d "replay" ]; then
        rm -r ./replay
    fi
else
    echo "compile error"
fi
