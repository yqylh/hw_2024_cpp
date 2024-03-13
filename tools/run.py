import os,sys,shutil
import argparse

if sys.platform.startswith('linux'):
    print("System: Linux")
    system = 'linux'
elif sys.platform.startswith('win'):
    print("System: Windows")
    system = 'win'
elif sys.platform.startswith('darwin'):
    print("System: Mac")
    system = 'mac'

def setup_args():
    parser = argparse.ArgumentParser(description='Upload a file to the server')
    parser.add_argument('--file_name',nargs='?',type=str,default=None, help='File name to upload')
    parser.add_argument('--debug',nargs='?',type=bool,default=False, help='Destination on the server')
    parser.add_argument('--map',nargs='?',type=str,default='map1.txt', help='Map to use')
    parser.add_argument('--random_seed',nargs='?',type=int,default=123, help='Random seed to use')
    return parser.parse_args()

def Do_cmd(args):
    if system == 'win':
        win_cmd(args)
    else:
        linux_cmd(args)

def win_cmd(args):
    Win_Cmd = '%CD%/../judge/PreliminaryJudge.exe -s ' + str(args.random_seed) + ' -m ../allMaps/' + args.map +' -r ./$map%Y-%m-%d.%H.%M.%S.rep main.exe'
    print(Win_Cmd)
    os.system(Win_Cmd)
    for files in os.listdir('./replay'):
        if files.endswith('.rep'):
            shutil.move('replay/' + files, '../judge/replay/' + files)
    if os.path.isfile('main.exe'):
        os.remove('main.exe')
    if os.path.exists('replay'):
        os.rmdir('replay')


def linux_cmd(args):
    Linux_Cmd = '../judge/PreliminaryJudge -s ' + str(args.random_seed) + ' -m ../allMaps/' + args.map +' -r ./$map%Y-%m-%d.%H.%M.%S.rep ./main'
    os.system(Linux_Cmd)
    for files in os.listdir('./replay'):
        if files.endswith('.rep'):
            shutil.move('./replay/' + files, '../judge/replay/' + files)
    if os.path.isfile('main'):
        os.remove('main')
    if os.path.isfile('main.dSYM'):
        os.remove('main.dSYM')
    if os.path.exists('replay'):
        os.rmdir('replay')

def main():
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
        cmd = "g++ main.cpp -o main -std=c++17 -O3"
        cmd = cmd + " -g -DDEBUG" if args.debug else cmd
        print(cmd)
        os.system(cmd)
        Do_cmd(args)
        print("Compile success")
    except Exception as e:
        print("Compile failed")
        print(e)
        return

if __name__ == "__main__":
    main()