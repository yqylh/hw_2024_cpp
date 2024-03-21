- allMaps: 存储地图文件
- code：代码文件
- tools：可视化、编译、upload等工具
- refCode：可参考的无关代码文件



Python Env:

```
conda create -n cc24 python=3.7.3
conda activate cc24
pip install numpy==1.19.4 matplotlib tqdm opencv-python pygame 
```

tune.py怎么用：

首先在main.cpp把所有要调的参数按顺序码好

然后在tune.py里面找到"在这里设置参数的可选项"，按顺序在para_input中设置好参数的可选项

要是只调一项参数的话，是可以进行可视化的：

把要可视化的参数可选项填好，其它参数可选项填成一个值，然后在下面的para_select_input把要可视化的的那一项设置为-1，其他项和para_input相等

然后运行tune.py，会生成tune.png，就是可视化结果

不进行可视化的：

所有参数的可选项可以随便填任意多个（注意时间），然后tune会搜索所有的组合，输出最好的结果，结果也会保存在tune_res.pkl，可以读取进行进一步的分析