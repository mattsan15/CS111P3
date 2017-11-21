##!/bin/bash

default:
	-gcc ext2_fs.h
	gcc -o lab3a lab3a.c
dist:
	tar -cfvz lab3a-004639538.tar.gz lab3a Makefile README
clean:
	rm -f lab3a *.tar.gz
