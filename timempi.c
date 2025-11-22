#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
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

  strftime(time_string, 30, "%Y-%m-%d %T", localtime(&current_time));
  snprintf(output, 80, "[%d] %s // %s.%d", rank , hostname, time_string, micro_sec);

  if (rank < size - 1 && size > 1){
    /* Paramether:
    * IN buf: Adresse der output-Daten (IN=lesend),
    * IN count: Anzahl der Elemente von output,
    * IN datatype: Datentyp von output,
    * IN dest: Rang des Empfängers(Prozesses),
    * IN tag: Nachrichtenerkennung,
    * IN comm: Kommunikator */
    MPI_Send(output, 80, MPI_CHAR, size - 1, 0, MPI_COMM_WORLD);
  }

  if (size > 1 && rank == size - 1)
  {
    for (int sender_rank = 0; sender_rank < size - 1; sender_rank++)
    {
      /* Paramether:
       * OUT buf: Adresse der output-Daten (OUT=schreibend),
       * IN count: Anzahl der Elemente von output + 1 für \0...(\0 ist immer Ende des Strings),
       * IN datatype: Datentyp von output,
       * IN source: Rang des Senders(Prozesses) wird durch for-Schleife bestimmt,
       * IN tag: Nachrichtenerkennung,
       * IN comm: Kommunikator
       * OUT status: Ergebnis des Empfangens */
      MPI_Recv(output, 80, MPI_CHAR, sender_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("%s\n", output);
      fflush(stdout); // Sorgt für sofortige Ausgabe
    }
  }

int end_after_synchronize;

if(rank == size-1){
  end_after_synchronize = 1;
}

/* Paramether:
 * INOUT void *buffer: root sendet Signal und alle anderen empfangen Signal,
 * IN int count: wie viele Elemente,
 * IN MPI_Datatype datatype: Datentyp von buffer,
 * IN int root: der Prozess-Rang, der SENDEN soll,
 * IN MPI_Comm comm: der Kommunikator (MPI_COMM_WORLD) */
MPI_Bcast(&end_after_synchronize, 1, MPI_INT, size - 1, MPI_COMM_WORLD);

printf("[%d] beendet jetzt!\n", rank);
fflush(stdout); // Sorgt für sofortige Ausgabe
MPI_Finalize();
return 0;
  }