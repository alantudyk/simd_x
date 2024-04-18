#!/bin/sh

gcc -march=native -flto -O3 *.c -o test && ./test
