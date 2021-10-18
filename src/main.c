#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "main.h"
#include "mpi.h"


int
main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	
	srand(time(NULL));
	int rank;
	int fd;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

// enter_critical_section
//<проверка наличия файла “critical.txt”>;
	if (!access(critical_file, F_OK)) 
	{
		printf("Error: file %s exists during process %d critical section\n", critical_file, rank);
		MPI_Abort(MPI_COMM_WORLD, MPI_ERR_FILE_EXISTS);
	}
	else
	{
		fd = open(critical_file, O_CREAT, S_IRWXU);
		if (!fd)
		{
			printf("Error: can not create file %s by process %d\n", critical_file, rank);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_FILE);
		}
		sleep(random() % MAX_SLEEP_TIME);
		if (close(fd))
		{
			printf("Error: can not close file %s by process %d\n", critical_file, rank);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_FILE);
		}
		if (!remove(critical_file))
		{
			printf("Error: can not remove file %s by process %d\n", critical_file, rank);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_FILE);
		}
	}
// exit_critical_section
	MPI_Finalize();
	return 0;
}
