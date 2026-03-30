#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void inicializaCadena(char *cadena, int n){
    int i;
    for(i=0; i<n/2; i++) cadena[i] = 'A';
    for(i=n/2; i<3*n/4; i++) cadena[i] = 'C';
    for(i=3*n/4; i<9*n/10; i++) cadena[i] = 'G';
    for(i=9*n/10; i<n; i++) cadena[i] = 'T';
}

int MPI_BinomialBcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int rank, numprocs;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &numprocs);
    int i = 1;
    while ((1 << (i - 1)) < numprocs) {
        int powers = (1 << (i - 1));
        if (rank < powers) {
            int target = rank + powers;
            if (target < numprocs) {
                MPI_Send(buffer, count, datatype, target, 0, comm);
            }
        } else if (rank < (1 << i)) {
            int source = rank - powers;
            MPI_Recv(buffer, count, datatype, source, 0, comm, MPI_STATUS_IGNORE);
        }
        i++;
    }
    return MPI_SUCCESS;
}

int MPI_FlattreeColectiva(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
    int rank, numprocs;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &numprocs);
    if (rank == root) {
        int total = *(int*)sendbuf;
        for (int i = 0; i < numprocs; i++) {
            if (i != root) {
                int temp;
                MPI_Recv(&temp, count, datatype, i, 0, comm, MPI_STATUS_IGNORE);
                total += temp;
            }
        }
        *(int*)recvbuf = total;
    } else {
        MPI_Send(sendbuf, count, datatype, root, 0, comm);
    }
    return MPI_SUCCESS;
}

int main (int argc, char *argv[]){
    MPI_Init(&argc, &argv);
    int numprocesos, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &numprocesos);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int n, i, local_count = 0, total_count = 0;
    char L, *cadena = NULL;

    if (rank == 0){ 
        if(argc != 3){
            printf("Uso: %s n L\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        n = atoi(argv[1]);
        L = *argv[2];
    }

    MPI_BinomialBcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_BinomialBcast(&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    cadena = (char *) malloc(n * sizeof(char));
    if (cadena == NULL) MPI_Abort(MPI_COMM_WORLD, 1);

    inicializaCadena(cadena, n);

    for(i = rank; i < n; i += numprocesos){
        if(cadena[i] == L) local_count++;
    }
    
    MPI_FlattreeColectiva(&local_count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("El numero de apariciones de la letra %c es %d\n", L, total_count);
    }

    free(cadena);
    MPI_Finalize();
    return 0;
}