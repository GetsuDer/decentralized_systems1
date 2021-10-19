#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
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
	int size;
	int fd;
	MPI_Request request;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

// enter_critical_section

// send message to other processes
	struct timeval cur_time;
	gettimeofday(&cur_time, NULL);
	memcpy(buff, &cur_time, sizeof(cur_time));

	for (int other = 0; other < size; other++)
	{
		if (other != rank)
		{
			memcpy(buff + sizeof(struct timeval), &rank, sizeof(rank));
			// non-blocking send
			MPI_Isend(buff, MSG_SIZE, MPI_BYTE, other, TAG_ASK, MPI_COMM_WORLD, &request);
			printf("%d asked %d\n", rank, other);
		}
	}

	printf("%d asked all\n", rank);
	// self-permissioning
	int permissions_number = 1;
	MPI_Status status;
	struct timeval other_time;
	int other_proc;

	// Array to remember waiting procs
	int waiting_procs[size];
	for (int i = 0; i < size; i++)
	{
		waiting_procs[i] = 0;
	}

    while (permissions_number != size)
	{
		MPI_Recv(buff, MSG_SIZE, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("%d recv answer\n", rank);
		switch(status.MPI_TAG)
		{
			// permission granted from some other proc
			case TAG_OK:
				printf("%d recv OK from %d\n", rank, status.MPI_SOURCE);
				permissions_number++;
				break;

			// other proc asking for permission
			case TAG_ASK:
				memcpy(&other_time, buff, sizeof(struct timeval));
				other_proc = status.MPI_SOURCE;
				printf("%d recv ask from %d\n", rank, other_proc);
				if (timercmp(&cur_time, &other_time, >))
				{
					printf("%d recv earlier ask from %d, answer OK\n", rank, other_proc);
					// Other process asked earlier, answer OK to him
					MPI_Send(buff, MSG_SIZE, MPI_BYTE, other_proc, TAG_OK, MPI_COMM_WORLD);
				}
				else
				{
					// Other process asked later, remember him
					printf("%d recv later ask from %d, remember\n", rank, other_proc);
					waiting_procs[other_proc] = 1;
				}
				break;
			default:
				printf("WTF? recv tag %d\n", status.MPI_TAG);
				MPI_Abort(MPI_COMM_WORLD, MPI_ERR_TAG);
		}
	}
	printf("%d starting critical section \n", rank);	
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
		int time_to_sleep = random() % MAX_SLEEP_TIME;
		printf("%d sleep for %d\n", rank, time_to_sleep);
		sleep(time_to_sleep);
		if (close(fd))
		{
			printf("Error: can not close file %s by process %d\n", critical_file, rank);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_FILE);
		}
		if (remove(critical_file))
		{
			printf("Error: can not remove file %s by process %d\n", critical_file, rank);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_FILE);
		}
	}
	printf("%d ended critical section\n", rank);
// exit_critical_section
// Answer OK to all remembered procs
	for (int i = 0; i < size; i++)
	{
		if (waiting_procs[i])
		{
			printf("%d send OK to waiting %d\n", rank, i);
			MPI_Isend(buff, MSG_SIZE, MPI_BYTE, i, TAG_OK, MPI_COMM_WORLD, &request);
		}
	}
	
	printf("Process %d sended all OKS\n", rank);
	while (1)
	{
		MPI_Recv(buff, MSG_SIZE, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (status.MPI_TAG == TAG_ASK)
		{
			printf("%d recv ask from %d, answering OK\n", rank, status.MPI_SOURCE);
			MPI_Isend(buff, MSG_SIZE, MPI_BYTE, status.MPI_SOURCE, TAG_OK, MPI_COMM_WORLD, &request);
		}	
	}
	MPI_Finalize();
	return 0;
}
