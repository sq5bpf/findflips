# Makefile for findflips

CC=gcc
COPTS=-O2

default: findflips

findflips: findflips.c
	$(CC) $(COPTS) findflips.c -o findflips

clean:
	rm findflips
