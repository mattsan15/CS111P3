##!/bin/bash

default:
	gcc -Wall -Wextra -o lab3a lab3a.c
dist:
	tar -czvf lab3a-004639538.tar.gz lab3a.c Makefile README
clean:
	rm -f lab3a *.tar.gz
