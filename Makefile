SRCDIR=src
INCLUDEDIR=include
MPI_CC=mpicc

.PHONY: all clean main

all: main
	mpiexec -n 3 ./main

main: clean
	$(MPI_CC) $(SRCDIR)/main.c -o main -I$(INCLUDEDIR)

clean:
	rm -f main critical.txt
