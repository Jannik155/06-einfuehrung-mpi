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

  /* resv-buffer muss auch für Worker initialisiert sein.
   * Worker dürfen resv_buffer = NULL haben.
   * MPI ignoriert ihre recvbuf-Argumente. */
  int *resv_buffer = NULL; // Array-Speicher für alle Microsekunden der Prozesse(außer master)
  if (is_master){
    resv_buffer = (int *)malloc((size) * sizeof(int));
  }

  if (size > 1){
    /* Gather sammelt Mikrosekunden aller Prozesse, für spätere Min- und Max-Suche */
    MPI_Gather(&micro_sec, 1, MPI_INT, resv_buffer, 1, MPI_INT, master, MPI_COMM_WORLD);
  }

  if (is_master){

    int min_micro_sec = resv_buffer[0];
    int max_micro_sec = resv_buffer[0];

    for (int i = 1; i < size - 1; i++)
    {
      if (resv_buffer[i] < min_micro_sec){
        min_micro_sec = resv_buffer[i]; // Findet min_micro_sec
      }

      if (resv_buffer[i] > max_micro_sec){
        max_micro_sec = resv_buffer[i]; // Findet max_micro_sec
      }
    }

    printf("[%d] Kleinster MS - Anteil : %d\n", rank, min_micro_sec);
    printf("[%d] Größte Differenz : %d\n", rank, (max_micro_sec - min_micro_sec));
    
    free(resv_buffer);
  }

  int end_after_synchronize = 0; // ist Teil der MPI_Bcast_Synchronisation

  if (is_master)
  {
    end_after_synchronize = 1;
  }
/*
 * MPI_Bcast synchronisiert alle Prozesse:
 * - Root/master sendet den Wert
 * - Alle anderen blockieren, bis sie ihn erhalten
 */
MPI_Bcast(&end_after_synchronize, 1, MPI_INT, master, MPI_COMM_WORLD);

printf("[%d] beendet jetzt!\n", rank);

// Barrier als Synchronisation auch möglich
// MPI_Barrier(MPI_COMM_WORLD);
MPI_Finalize();
return 0;
}