#!/bin/sh

gcc -march=native -flto -O3 *.c -lm && taskset -c 0 ./a.out
