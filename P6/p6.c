#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define DEBUG 0

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4*/

#define M  1000000 // Number of sequences
#define N  200  // Number of bases per sequence

unsigned int g_seed = 0;

int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16) % 5;
}

// The distance between two bases
int base_distance(int base1, int base2){
  if((base1 == 4) || (base2 == 4)){
    return 3;
  }
  if(base1 == base2) {
    return 0;
  }
  if((base1 == 0) && (base2 == 3)) {
    return 1;
  }
  if((base2 == 0) && (base1 == 3)) {
    return 1;
  }
  if((base1 == 1) && (base2 == 2)) {
    return 1;
  }
  if((base2 == 2) && (base1 == 1)) {
    return 1;
  }
  return 2;
}

int main (int argc, char *argv[]){
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double t_comm = 0, t_comp = 0, start;

    int rows_per_proc = M / size;
    int remainder = M % size;
    int *send_counts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));
    int *send_counts_res = malloc(size * sizeof(int));
    int *displs_res = malloc(size * sizeof(int));

    int offset = 0;
    for (int i = 0; i < size; i++) {
        int rows = rows_per_proc + (i < remainder ? 1 : 0);
        send_counts[i] = rows * N;
        displs[i] = offset * N;
        send_counts_res[i] = rows;
        displs_res[i] = offset;
        offset += rows;
    }

    int my_rows = send_counts_res[rank];
    int *data1_local = malloc(my_rows * N * sizeof(int));
    int *data2_local = malloc(my_rows * N * sizeof(int));
    int *result_local = malloc(my_rows * sizeof(int));

    int *data1_full = NULL, *data2_full = NULL, *result_full = NULL;

    if (rank == 0) {
        data1_full = malloc(M * N * sizeof(int));
        data2_full = malloc(M * N * sizeof(int));
        result_full = malloc(M * sizeof(int));
        for (int i = 0; i < M * N; i++) {
            data1_full[i] = fast_rand();
            data2_full[i] = fast_rand();
        }
    }

    start = MPI_Wtime();
    MPI_Scatterv(data1_full, send_counts, displs, MPI_INT, data1_local, my_rows * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(data2_full, send_counts, displs, MPI_INT, data2_local, my_rows * N, MPI_INT, 0, MPI_COMM_WORLD);
    t_comm += (MPI_Wtime() - start);

    start = MPI_Wtime();
    for (int i = 0; i < my_rows; i++) {
        result_local[i] = 0;
        for (int j = 0; j < N; j++) {
            result_local[i] += base_distance(data1_local[i * N + j], data2_local[i * N + j]);
        }
    }
    t_comp += (MPI_Wtime() - start);

    start = MPI_Wtime();
    MPI_Gatherv(result_local, my_rows, MPI_INT, result_full, send_counts_res, displs_res, MPI_INT, 0, MPI_COMM_WORLD);
    t_comm += (MPI_Wtime() - start);

    printf("Rank %d: Comm_Time = %f, Comp_Time = %f\n", rank, t_comm, t_comp);

    if (rank == 0) {
        free(data1_full); free(data2_full); free(result_full);
    }

    free(data1_local); free(data2_local); free(result_local);
    free(send_counts); free(displs); free(send_counts_res); free(displs_res);

    MPI_Finalize();
    return 0;
}
