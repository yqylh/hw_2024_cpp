rm ../upload/*
cp ./*.cpp ../upload/
cp ./*.hpp ../upload/
cp ./CMakeLists.txt ../upload/
cp ./CodeCraft_zip.sh ../upload/
../upload/CodeCraft_zip.sh
mv ../*.zip ../oldFileLog/
mv ../upload/*.zip ../
