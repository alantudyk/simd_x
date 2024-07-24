#!/bin/sh

clang -march=native -flto -O3 *.c && taskset -c 0 ./a.out
