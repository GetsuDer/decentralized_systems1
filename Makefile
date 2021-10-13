SRCDIR=src

all: main
	mpiexec -n 3 ./main

main:
	mpicc $(SRCDIR)/main.c -o main
