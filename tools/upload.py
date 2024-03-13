import os, sys, shutil, glob
import zipfile
import argparse

def setup_args():
    parser = argparse.ArgumentParser(description='Upload a file to the server')
    parser.add_argument('--file_name',nargs='?',type=str,default=None, help='File name to upload')
    return parser.parse_args()

def create_zipfile(zip_name, files):
    with zipfile.ZipFile(zip_name, 'w') as zipf:
        for file in files:
            zipf.write(file)

def main():
    args = setup_args()
    os.chdir("..")
    if not os.path.exists("upload"):
        os.mkdir("upload")
    if not os.path.exists("oldFile"):
        os.mkdir("oldFile")

    # 复制所有的 .cpp 文件到 upload/
    for cpp_file in glob.glob('code/*.cpp'):
        shutil.copy(cpp_file, 'upload/')

    # 复制所有的 .hpp 文件到 upload/
    for hpp_file in glob.glob('code/*.hpp'):
        shutil.copy(hpp_file, 'upload/')

    # 复制 CMakeLists.txt 文件到 upload/
    shutil.copy('code/CMakeLists.txt', 'upload/')

    #默认日期为zip_name:
    if args.file_name is None:
        import datetime
        current_time = datetime.datetime.now()
        zip_name = current_time.strftime("%Y-%m-%d-%H-%M-%S") + ".zip"
    else:
        zip_name = args.file_name + ".zip" if not args.file_name.endswith(".zip") else args.file_name
        while os.path.exists(zip_name):
            zip_name = zip_name[:-4] + "_2.zip"
            
    
    # 移动旧的到 oldFile/
    old_file = glob.glob('*.zip')
    if len(old_file) > 0:
        shutil.move(old_file[0], 'oldFile/' + old_file[0])

    # 创建压缩文件
    create_zipfile(zip_name, glob.glob('upload/*'))

    # 删除 upload 文件夹
    shutil.rmtree('upload')






if __name__ == "__main__":
    main()