import argparse
from audioop import avg
import os
import sys
import pickle
import random
import shutil
import subprocess
from time import sleep
import matplotlib.pyplot as plt
from tqdm import trange

from multiprocessing import Pool
from functools import partial

if sys.platform.startswith('linux'):
    system = 'linux'
elif sys.platform.startswith('win'):
    system = 'win'
elif sys.platform.startswith('darwin'):
    system = 'mac'


para_input = [
    [150,16,3,0.7,1700,2000,700],
    [150,16,3,0.76,800,1700,1000],
    [160,80,3,0.7,800,1600,1000],
    [150,16,3,0.7,1700,2000,500],
]
score_input = [
    255136,
    240418,
    250954,
    241682,
]

def del_files_win():
    if os.path.isfile('main.exe'):
        remove_file('main.exe')
    if os.path.exists('replay'):
        os.rmdir('replay')

def del_files_linux():
    if os.path.isfile('main'):
        os.remove('main')
    if os.path.exists('main.dSYM'):
        shutil.rmtree('main.dSYM')
    if os.path.exists('replay'):
        os.rmdir('replay')

def del_files():
    if system == 'win':
        del_files_win()
    else:
        del_files_linux()

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
    Win_Cmd = f'%CD%/../judge/SemiFinalJudge.exe -s {str(random_seed)} -m ../{args.map_folder}/{args.map} -r ./{args.map}{str(random_seed)}%Y-%m-%d.%H.%M.%S.rep ./main'
    Linux_Cmd = f'../judge/SemiFinalJudge -s {str(random_seed)} -m ../{args.map_folder}/{args.map} -r ./{args.map}{str(random_seed)}%Y-%m-%d.%H.%M.%S.rep ./main'
    
    if not os.path.exists('../log'):
        os.makedirs('../log')
    
    process = subprocess.Popen(Linux_Cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
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
    

def run_one_all_seed(args, now_para, seed_list):
    save_para(now_para)

    with Pool(processes=6) as pool:
        func = partial(run_one, args)
        score_list = pool.map(func, seed_list)
    return score_list
    
def result_with_seed(args, all_paras, seed_list):
    all_scores = []
    
    for i in trange(len(all_paras)):
        now_para = all_paras[i]
        score_list = run_one_all_seed(args, now_para, seed_list)
        all_scores.append(score_list)
    
    return all_paras, all_scores


def cal_mse(source, target):
    mse = 0
    for i in range(len(source)):
        mse += (source[i] - target[i]) ** 2
    return mse

# 交换输入的维度
def swap_input_dim(input_list):
    swap_list = []
    for i in range(len(input_list[0])):
        now_list = []
        for j in range(len(input_list)):
            now_list.append(input_list[j][i])
        swap_list.append(now_list)
    return swap_list

# tar_score: 5~6个参数的真实分数
# all_scores: 5~6个参数的分数，每个里面有随机种子个数个分数
# swaped_all_scores: 所有随机种子的分数，每个里面有5~6个参数的分数
def select_seed(tar_scores, all_scores, all_seeds):
    swaped_all_scores = swap_input_dim(all_scores)
    min_mse = 1e9
    best_seed = -1
    for i in range(len(all_seeds)):
        now_mse = cal_mse(tar_scores, swaped_all_scores[i])
        if now_mse < min_mse:
            min_mse = now_mse
            best_seed = all_seeds[i]
    return best_seed, min_mse
        
            
    
    
def main():
    # random.seed(990321)
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
        cmd = "g++-13 main.cpp -o main -std=c++17 -O2 -DTUNE"

        if os.system(cmd) == 0:
            print("Compile success")
        else:
            print("Compile failed")
            return 
    except Exception as e:
        print("Compile failed")
        print(e)
        return

    seed_list = [random.randint(0, 32767) for i in range(args.test_times)]
    all_paras, all_scores = result_with_seed(args, para_input, seed_list)
        
    tune_res_file_pickle = 'seed_res.pkl'
    with open(tune_res_file_pickle, 'wb') as f:
        pickle.dump(seed_list, f)
        pickle.dump(all_scores, f)
        
    best_seed, min_mse = select_seed(score_input, all_scores, seed_list)
    
    print(f'Best seed: {best_seed}, min mse: {min_mse}')
        
    
    del_files()

if __name__ == "__main__":
    main()