#!/bin/sh

gcc -march="skylake-avx512" -flto -O3 *.c
