#define _GNU_SOURCE 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "mpi.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                         exit(EXIT_FAILURE))

#define MAX_SLEEP 4
#define MIN_SLEEP 1

#define BUFF_SIZE 32

#define COORDS_SEPARATOR ','

typedef struct {
	int x;
	int y;
} location;

int main(int argc, char* argv[]) {
	int rank, size, n=4, k=3, num=0, i, blockcounts[1], total_wrecks, found_wrecks=0, sep_index;
	location* locs;
	location* rec;
	char buf[BUFF_SIZE];
	MPI_Datatype locationtype, oldtypes[1];
	MPI_Aint offsets[1];
	
	/* Initialize the environment */
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );

	n = atoi(argv[1]);
	k = atoi(argv[2]);

	/* Rejestracja typu */
	offsets[0] = 0;
	blockcounts[0] = 2;
	oldtypes[0] = MPI_INT;
	MPI_Type_struct(1, blockcounts, offsets, oldtypes, &locationtype);
	MPI_Type_commit(&locationtype);

	if( (rec = (location*) calloc(k, sizeof(location))) == NULL) {
		ERR("calloc");
	}

	/* Ustawianie seeda na pid */
	srand(getpid());

	if(rank == 0) {
		if( (locs = (location*) calloc(n * k, sizeof(location))) == NULL) {
			ERR("calloc");
		}
		/* Czytanie mapy z pliku */
		FILE* file;
		file = fopen(argv[3], "r");

		i = 0;
		while(fgets(buf, BUFF_SIZE, file) != NULL) {
			
			sep_index = (int) (strchr(buf, COORDS_SEPARATOR) - buf);
			printf("(%d, %d)\n", atoi(buf), atoi(buf + sep_index + 1));
			locs[i].x = atoi(buf);
			locs[i++].y = atoi(buf + sep_index + 1);
		}

		fclose(file);
	}

	/* Roze�lij lokacje do proces�w */
	MPI_Scatter(locs, k, locationtype, rec, k, locationtype, 0, MPI_COMM_WORLD);

	/* Przetwarzanie swoich lokacji */
	for(i = 0; i < k; i++) {
		printf("Poszukiwacz#%d: sprawdzam (%d, %d)...\n", rank, rec[i].x, rec[i].y);
		sleep(rand() % (MAX_SLEEP - MIN_SLEEP) + MIN_SLEEP);
		if( rand() % 5 != 0) {
			printf("Poszukiwacz#%d: brak wraku w lokacji (%d, %d)\n", rank, rec[i].x, rec[i].y);
			rec[i].x = 0;
			rec[i].y = 0;
		} else {
			printf("Poszukiwacz#%d, znaleziono wrak w lokacji: (%d, %d)\n", rank, rec[i].x, rec[i].y);
			++found_wrecks;
		}
		
	}
	
	/* Zbieranie informacji o ilosci znalezionych wrak�w */
	MPI_Reduce(&found_wrecks, &total_wrecks, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	
	MPI_Gather(rec, k, locationtype, locs, k, locationtype, 0, MPI_COMM_WORLD);

	/* Poni�sza bariera jest u�yta tylko do lepszego wypisywania - mo�na zakomentowa� */
	MPI_Barrier(MPI_COMM_WORLD);
	
	/* Proces dow�dcy wypisuje znalezione wraki */
	if( rank == 0) {
		printf("Znaleziono wraków: %d\n", total_wrecks);
		for(i = 0; i < n*k; i++) {
			if(!(locs[i].x == 0 && locs[i].y == 0)) {
				printf("Wrak %d: (%d, %d)\n", ++num, locs[i].x, locs[i].y);
			}
		}
	}

	/* Zwolnienei typu i zaalokowanej pamieci */
	MPI_Type_free(&locationtype);	
	free(rec);
	if(rank == 0) {
		free(locs);
	}
	MPI_Finalize();
	return 0;
}
