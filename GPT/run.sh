#! /usr/bin/env bash

dd if=/dev/zero of=image.bin bs=1M count=20
gcc ./GPT.c
./a.out ./image.bin 10
fdisk -l image.bin
