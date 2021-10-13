#include <stdio.h>

#include "mpi.h"

int
main(int argc, char **argv)
{
	printf("before init\n");
	MPI_Init(&argc, &argv);
	printf("after init\n");
	MPI_Finalize();
	printf("after finalize\n");
	return 0;
}
