#!/bin/bash

SCRIPT=$(readlink -f "$0")
BASEDIR=$(dirname "$SCRIPT")
cd $BASEDIR

yesterday=`date +%m-%d--%H:%M:%S`

echo "Current date: $yesterday"
zip -9 -r $yesterday.zip *.cpp *.cc *.c *.hpp *.h CMakeLists.txt