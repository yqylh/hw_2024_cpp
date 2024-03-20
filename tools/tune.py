import argparse
from audioop import avg
import os
import pickle
import random
import shutil
import subprocess
from time import sleep
import matplotlib.pyplot as plt
from tqdm import trange

from multiprocessing import Pool
from functools import partial


'''
160
80
0.85
10

para_input = [
    [10, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200], # MAX_Berth_Control_Length, 10~200, 5
    [80], # MAX_Berth_Merge_Length, 1~200, 5
    [0.85], # Sell_Ration,  0.5~1
    [10] # Min_Next_Berth_Goods,  0~100
]


para_input = [
    [160], # MAX_Berth_Control_Length, 10~200, 5
    [1, 5, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200], # MAX_Berth_Merge_Length, 1~200, 5
    [0.85], # Sell_Ration,  0.5~1
    [10] # Min_Next_Berth_Goods,  0~100
]



para_input = [
    [160], # MAX_Berth_Control_Length, 10~200, 5
    [80], # MAX_Berth_Merge_Length, 1~200, 5
    [0.5, 0.6, 0.7, 0.8, 0.9, 1.0], # Sell_Ration,  0.5~1
    [10] # Min_Next_Berth_Goods,  0~100
]


para_input = [
    [160], # MAX_Berth_Control_Length, 10~200, 5
    [80], # MAX_Berth_Merge_Length, 1~200, 5
    [0.85], # Sell_Ration,  0.5~1
    [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100] # Min_Next_Berth_Goods,  0~100
]
'''

''' 在这里设置参数的可选项 '''
para_input = [
    [5, 10, 15, 40, 80, 160, 200], # MAX_Berth_Control_Length, 10~200, 5
    [1], # MAX_Berth_Merge_Length, 1~200, 5
    [0.7], # Sell_Ration,  0.5~1
    [800] # Min_Next_Berth_Value,  0~100
]

''' 如果要进行可视化，-1是要可视化的参数 '''
para_select_input = [
    -1,
    1,
    0.7,
    800
]

def del_files():
    if os.path.isfile('main.exe'):
        remove_file('main.exe')
    if os.path.exists('replay'):
        os.rmdir('replay')

def setup_args():
    parser = argparse.ArgumentParser(description='Upload a file to the server')
    parser.add_argument('--map', '-m', nargs='?', type=str, default='map1.txt', help='Map to use')
    parser.add_argument('--test_times', '-t', nargs='?', type=int, default=5, help='Random times')
    parser.add_argument('--map_folder', '-f', nargs='?', type=str, default="allMaps", help='Random seed to use')

    return parser.parse_args()

def save_para(params):
    with open('para.txt', 'w') as f:
        for para in params:
            f.write(str(para) + '\n')

def remove_file(file_path, attempts=10):
    for i in range(attempts):
        try:
            # check file or folder
            if os.path.isfile(file_path):
                os.remove(file_path)
            else:
                shutil.rmtree(file_path)
            break
        except Exception as e:
            if i == attempts - 1:
                print(e)
            else:
                sleep(0.5)
                

def run_one(args, random_seed):
    Win_Cmd = f'%CD%/../judge/PreliminaryJudge.exe -s {str(random_seed)} -m ../{args.map_folder}/{args.map} -r ./{args.map}{str(random_seed)}%Y-%m-%d.%H.%M.%S.rep ./main'

    if not os.path.exists('../log'):
        os.makedirs('../log')
    
    process = subprocess.Popen(Win_Cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()

    for files in os.listdir('./replay'):
        if files.endswith('.rep'):
            # remove it
            remove_file(f'./replay/{files}')
            
    if stdout:
        score_str = stdout.decode('utf-8').split(':')[2].split('}')[0]
        score_str = score_str.strip()  # Remove leading/trailing whitespace
        score_str = score_str.split('\r\n')[0]  # Remove additional characters after the numeric value
        score = int(score_str)
        return score
    else:
        return 0

def gen_all_paras(para_list):
    if len(para_list) == 1:
        return [[i] for i in para_list[0]]
    else:
        sub_paras = gen_all_paras(para_list[1:])
        result = []
        for para in para_list[0]:
            for sub_para in sub_paras:
                result.append([para] + sub_para)
        return result
    

def run_one_all_seed(args, now_para, seed_list):
    save_para(now_para)
    '''
    score_list = []
    for j in range(args.test_times):
        res = run_one(args, seed_list[j])
        score_list.append(res)
    '''


    with Pool(processes=6) as pool:
        func = partial(run_one, args)
        score_list = pool.map(func, seed_list)
    return score_list
    
def tune(args, para_list, seed_list):
    all_paras = gen_all_paras(para_list)
    all_scores = []
    
    for i in trange(len(all_paras)):
        now_para = all_paras[i]
        score_list = run_one_all_seed(args, now_para, seed_list)
        all_scores.append(score_list)
    
    return all_paras, all_scores


def draw_one_param(para_select, all_paras, all_scores):
    selected_para_values = []
    selected_scores = []
    for i, para in enumerate(all_paras):
        will_select = True
        select_pos = 0
        for j, select_value in enumerate(para_select):
            if select_value != -1 and para[j] != select_value:
                will_select = False
                break
            if select_value == -1:
                select_pos = j
        if will_select:
            selected_para_values.append(para[select_pos])
            selected_scores.append(all_scores[i])
    min_score = []
    max_score = []
    avg_scores = []
    for scores in selected_scores:
        min_score.append(min(scores))
        max_score.append(max(scores))
        avg_scores.append(sum(scores) / len(scores))
    print(selected_para_values)
    print(avg_scores)
    # plot node with selected_para_values and selected_scores
    plt.figure()
    for i, para in enumerate(selected_para_values):
        plt.scatter([para] * len(selected_scores[i]), selected_scores[i], c='b', marker='o', alpha=0.5)
    
    
    plt.plot(selected_para_values, min_score, label='min')
    plt.plot(selected_para_values, max_score, label='max')
    plt.plot(selected_para_values, avg_scores, label='avg')
    plt.legend()
    plt.savefig('tune.png')
    plt.show()
    plt.close()

def select_best_para(all_paras, all_scores):
    avg_scores = []
    min_scores = []
    max_scores = []
    for i, para in enumerate(all_paras):
        score = all_scores[i]
        avg_scores.append(sum(score) / len(score))
        min_scores.append(min(score))
        max_scores.append(max(score))
    max_avg_index = avg_scores.index(max(avg_scores))
    print("平均最佳:", all_paras[max_avg_index], avg_scores[max_avg_index], min_scores[max_avg_index], max_scores[max_avg_index])
    max_min_index = min_scores.index(max(min_scores))
    print("最小最佳:", all_paras[max_min_index], avg_scores[max_min_index], min_scores[max_min_index], max_scores[max_min_index])
    max_max_index = max_scores.index(max(max_scores))
    print("最大最佳:", all_paras[max_max_index], avg_scores[max_max_index], min_scores[max_max_index], max_scores[max_max_index])
    
def main():
    random.seed(990321)
    args = setup_args()
    print(args)
    now_path = os.getcwd()
    if not now_path.endswith("tools"):
        os.chdir("tools")

    if not os.path.exists("../judge/replay"):
        os.makedirs("../judge/replay")
        print("Created replay folder")
    
    try:
        os.chdir("../code")
        cmd = "g++ main.cpp -o main -std=c++17 -O2 -DTUNE"

        if os.system(cmd) == 0:
            print("Compile success")
        else:
            print("Compile failed")
            return 
    except Exception as e:
        print("Compile failed")
        print(e)
        return

    seed_list = [random.randint(0, 1024) for i in range(args.test_times)]
    all_paras, all_scores = tune(args, para_input, seed_list)
    
    tune_res_file_pickle = 'tune_res.pkl'
    with open(tune_res_file_pickle, 'wb') as f:
        pickle.dump(all_paras, f)
        pickle.dump(all_scores, f)
        
    draw_one_param(para_select_input, all_paras, all_scores)
    select_best_para(all_paras, all_scores)
    
    del_files()

if __name__ == "__main__":
    main()