#! /usr/bin/env bash

dd if=/dev/zero of=image.bin bs=1M count=10
gcc ./mbr.c
./a.out ./image.bin
fdisk -l image.bin
