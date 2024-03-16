# 如果没有upload文件夹，就创建一个
if [ ! -d "../upload" ]; then
  mkdir ../upload
fi
# 如果没有oldFileLog文件夹，就创建一个
if [ ! -d "../oldFile" ]; then
  mkdir ../oldFile
fi
# 删除upload文件夹下的所有文件
rm ../upload/*
# 复制当前文件夹下的所有代码到upload文件夹
cp ./*.cpp ../upload/
cp ./*.hpp ../upload/
cp ./CMakeLists.txt ../upload/

# 打包
cd ../upload/
yesterday=`date +%m-%d--%H:%M:%S`
echo "Current date: $yesterday"
zip -9 -r $yesterday.zip *.cpp *.cc *.c *.hpp *.h CMakeLists.txt
# 存储旧的代码到oldFileLog文件夹
mv ../*.zip ../oldFile/
# 将打包好的代码移动到上一级目录
mv ./*.zip ../
# 清除无用文件
cd ..
rm -r upload