#!/bin/bash
rm -rf CMakeCache.txt CMakeFiles
cmake . && make
g++ test/main.cpp -o etcdclient -I./include -I./proto -L./ -letcd_client -L/usr/local/lib -lprotobuf -pthread -lgrpc++  -std=c++11 -g -O0
