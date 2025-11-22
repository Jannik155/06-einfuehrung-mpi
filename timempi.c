#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char *argv[])
{

  struct timeval tv;
  time_t current_time;
  int micro_sec;
  char time_string[30];
  char output[80];
  char hostname[30];

  gettimeofday(&tv, NULL);
  gethostname(hostname, 30);

  current_time = tv.tv_sec;
  micro_sec = tv.tv_usec;

  int rank;
  int size;

  MPI_Init(&argc, &argv);
  /* Funktionen sorgen dafür, dass wir bei Verwendung von rank/size
   * zukünftig immer prozessspezifische Werte nutzen */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int master = size - 1;
  int is_master = (rank == master);

  strftime(time_string, 30, "%Y-%m-%d %T", localtime(&current_time));
  snprintf(output, 80, "[%d] %s // %s.%d", rank , hostname, time_string, micro_sec);

  if (!is_master){
    MPI_Send(output, strlen(output) + 1, MPI_CHAR, master, 0, MPI_COMM_WORLD);
  }

  if (is_master && size > 1)
  {
    for (int sender_rank = 0; sender_rank < master; sender_rank++)
    {
      MPI_Recv(output, 80, MPI_CHAR, sender_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("%s\n", output);
    }
  }

int end_after_synchronize = 0; // ist Teil der MPI_Bcast_Synchronisation

if(is_master){
  end_after_synchronize = 1;
}

/*
 * MPI_Bcast synchronisiert alle Prozesse:
 * - Root/master sendet den Wert
 * - Alle anderen blockieren, bis sie ihn erhalten
 */
MPI_Bcast(&end_after_synchronize, 1, MPI_INT, master, MPI_COMM_WORLD);

printf("[%d] beendet jetzt!\n", rank);

MPI_Finalize();
return 0;
}

//printf("%s\n", output);
//printf("%d\n", micro_sec);