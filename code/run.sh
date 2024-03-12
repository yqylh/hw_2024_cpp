rm ../judge-1/replay/*
g++ main.cpp -o main -std=c++17 -O3 -pthread -DCREATE;
echo "[蓝方=我, 红方=我]"
../judge/Robot -s $1 -m ../judge-1/maps/1.txt -r ../judge-1/replay/map1%Y-%m-%d.%H.%M.%S.rep -f ./main
if [ -f "main" ]; then
    rm main
fi
if [ -d "main.dSYM" ]; then
    rm -r ./main.dSYM
fi
if [ -d "replay" ]; then
    rm -r ./replay
fi
