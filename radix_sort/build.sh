#!/bin/sh

gcc -mavx512f -flto -O3 *.c
