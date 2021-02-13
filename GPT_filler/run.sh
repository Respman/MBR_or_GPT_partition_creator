#! /usr/bin/env bash

dd if=/dev/zero of=image.bin bs=1M count=10
gcc ./GPT.c
./a.out ./image.bin ./image1.bin ./1.txt
fdisk -l image.bin
