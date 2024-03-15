import os,sys,shutil
import argparse
import subprocess
from time import sleep

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
    config = {}
    parser = argparse.ArgumentParser(description='Upload a file to the server')
    parser.add_argument('--stdout', '-s', nargs='?',type=str,default="False", help='File name to upload')
    parser.add_argument('--file_name', '-f', nargs='?',type=str,default=None, help='File name to upload')
    parser.add_argument('--debug', '-d', nargs='?',type=str,default="False", help='Destination on the server')
    parser.add_argument('--map', '-m', nargs='?',type=str,default='map1.txt', help='Map to use')
    parser.add_argument('--random_seed', nargs='?',type=int,default=123, help='Random seed to use')
    args = parser.parse_args()
    config['file_name'] = args.file_name
    # to boolean
    config['debug'] = (args.debug == 'True' or args.debug == 'true' or args.debug == '1')
    config['map'] = args.map
    config['random_seed'] = args.random_seed
    config['stdout'] = (args.stdout == 'True' or args.stdout == 'true' or args.stdout == '1')
    return argparse.Namespace(**config)

def Do_cmd(args):
    if system == 'win':
        win_cmd(args)
    else:
        linux_cmd(args)

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
    
def win_cmd(args):
    Win_Cmd = '%CD%/../judge/PreliminaryJudge.exe -s ' + str(args.random_seed) + ' -m ../allMaps/' + args.map +' -r ./{}%Y-%m-%d.%H.%M.%S.rep ./main'.format(args.map)
    print(Win_Cmd)
    
    # check if '../log' exists
    if not os.path.exists('../log'):
        os.makedirs('../log')
    
    if args.stdout:
        os.system(Win_Cmd)
    else:
        process = subprocess.Popen(Win_Cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = process.communicate()
        
        if stdout:
            with open('../log/judger_output.txt', 'w') as f:
                f.write("stdout:")
                f.write(stdout.decode('utf-8'))
        
        if stderr:
            with open('../log/judger_output.txt', 'a') as f:
                f.write("stderr:")
                f.write(stderr.decode('utf-8'))
    
    for files in os.listdir('./replay'):
        if files.endswith('.rep'):
            shutil.move('replay/' + files, '../judge/replay/' + files)
    # sleep(1)
    if os.path.isfile('main.exe'):
        remove_file('main.exe')
    if os.path.exists('replay'):
        os.rmdir('replay')


def linux_cmd(args):
    Linux_Cmd = '../judge/PreliminaryJudge -s ' + str(args.random_seed) + ' -m ../allMaps/' + args.map +' -r ./{}%Y-%m-%d.%H.%M.%S.rep ./main'.format(args.map)
    print(Linux_Cmd)
    
    # check if '../log' exists
    if not os.path.exists('../log'):
        os.makedirs('../log')
        
    if args.stdout:
        os.system(Linux_Cmd)
    else:
        process = subprocess.Popen(Linux_Cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = process.communicate()
        
        if stdout:
            with open('../log/judger_output.txt', 'w') as f:
                f.write("stdout:")
                f.write(stdout.decode('utf-8'))
        
        if stderr:
            with open('../log/judger_output.txt', 'a') as f:
                f.write("stderr:")
                f.write(stderr.decode('utf-8'))
            
    for files in os.listdir('./replay'):
        if files.endswith('.rep'):
            shutil.move('./replay/' + files, '../judge/replay/' + files)
    if os.path.isfile('main'):
        os.remove('main')
    if os.path.exists('main.dSYM'):
        shutil.rmtree('main.dSYM')
    if os.path.exists('replay'):
        os.rmdir('replay')

def compile_fmt():
    fmt_lib_path = "../dcode/libfmt.a"
    if not os.path.exists(fmt_lib_path):
        # 如果静态库不存在，先编译fmt库
        print("Compiling fmt library...")
        fmt_compile_cmd = "g++ -c -o ../dcode/fmt.o ../dcode/src/format.cc -I../dcode/include"
        if os.system(fmt_compile_cmd) == 0:
            print("fmt library compile success")
        else:
            print("fmt library compile failed")
            raise Exception("fmt library compile failed")
        ar_cmd = "ar rcs " + fmt_lib_path + " ../dcode/fmt.o"
        os.system(ar_cmd)
    else:
        print("fmt library exists")
        
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
        if system == 'win' or system == 'linux':
            cmd = "g++ main.cpp -o main -std=c++17 -O3"
        else:
            cmd = "g++-13 main.cpp -o main -std=c++17 -O3"
            
        if args.debug:
            compile_fmt()
            cmd += " -g -DDEBUG -DEBUG"
            # cmd = cmd + " ../dcode/src/format.cc -I../dcode/include"
            cmd += " -L../dcode -lfmt -I../dcode/include"
        print(cmd)
        if os.system(cmd) == 0:
            print("Compile success")
        else:
            print("Compile failed")
            return 
    except Exception as e:
        print("Compile failed")
        print(e)
        return

    try:
        Do_cmd(args)
    except Exception as e:
        print("Run failed")
        print(e)
        return
    
    print("Run success")

if __name__ == "__main__":
    main()