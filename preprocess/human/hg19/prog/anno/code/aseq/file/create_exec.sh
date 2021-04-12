#!/bin/bash

g++ create_xlib.cpp -o create_xlib.exe -std=c++11 -O2 -lz -llz4 -lbz2 -llzma -static-libgcc -static-libstdc++
g++ create_anno.cpp -o create_anno.exe -std=c++11 -O2 -lz -llz4 -lbz2 -llzma -lpthread -static-libgcc -static-libstdc++
