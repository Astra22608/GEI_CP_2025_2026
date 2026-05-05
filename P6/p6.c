#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define DEBUG 0

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4*/

#define M 1000000 // Number of sequences
#define N 200 // Number of bases per sequence

unsigned int g_seed = 0;

int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16) % 5;
}

int base_distance(int base1, int base2){
  if((base1 == 4) || (base2 == 4)) return 3;
  if(base1 == base2) return 0;
  if((base1 == 0 && base2 == 3) || (base1 == 3 && base2 == 0)) return 1;
  if((base1 == 1 && base2 == 2) || (base1 == 2 && base2 == 1)) return 1;
  return 2;
}

int main(int argc, char *argv[] ) {

  int i,j;
  int *data1 = NULL, *data2 = NULL;
  int *result = NULL;
  int *recvbuff1, *recvbuff2;
  int *local_result;
  struct timeval tv1, tv2;
  int numprocs, rank, t_comm = 0, t_comp = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int mlocal = M/numprocs;
  
  recvbuff1 = (int *) malloc(mlocal*N*sizeof(int));
  recvbuff2 = (int *) malloc(mlocal*N*sizeof(int));
  local_result = (int *) malloc(mlocal*sizeof(int));

  if(rank == 0){
    data1 = (int *) malloc(M*N*sizeof(int));
    data2 = (int *) malloc(M*N*sizeof(int));
    result = (int *) malloc(M*sizeof(int));
    
  /* Initialize Matrices */
    for(i=0;i<M;i++){
      for(j=0;j<N;j++){
        /* random with 20% gap proportion */
        data1[i*N+j] = fast_rand();
        data2[i*N+j] = fast_rand();
      }
    }
  }

  gettimeofday(&tv1, NULL);
  
  MPI_Scatter(data1, mlocal*N, MPI_INT,recvbuff1, mlocal*N, MPI_INT,0, MPI_COMM_WORLD);
  MPI_Scatter(data2, mlocal*N, MPI_INT,recvbuff2, mlocal*N, MPI_INT,0, MPI_COMM_WORLD);

  gettimeofday(&tv2, NULL);

  t_comm += (tv2.tv_usec - tv1.tv_usec) + 1000000*(tv2.tv_sec - tv1.tv_sec);

  gettimeofday(&tv1, NULL);

  for(int i=0;i<mlocal;i++){
    local_result[i] = 0;
    for(int j=0;j<N;j++){
      local_result[i] += base_distance(recvbuff1[i*N+j],recvbuff2[i*N+j]);
    }
  }

  gettimeofday(&tv2, NULL);

  t_comp += (tv2.tv_usec - tv1.tv_usec) + 1000000*(tv2.tv_sec - tv1.tv_sec);

  gettimeofday(&tv1, NULL);

  MPI_Gather(local_result, mlocal, MPI_INT,result, mlocal, MPI_INT,0, MPI_COMM_WORLD);

  gettimeofday(&tv2, NULL);

  t_comm += (tv2.tv_usec - tv1.tv_usec) + 1000000*(tv2.tv_sec - tv1.tv_sec);

  if(rank == 0){
    gettimeofday(&tv1, NULL);

    for(int i = mlocal*numprocs; i < M; i++){
        result[i] = 0;
        for(int j=0;j<N;j++){
            result[i] += base_distance(data1[i*N+j], data2[i*N+j]);
        }
    }

    gettimeofday(&tv2, NULL);

    t_comp += (tv2.tv_usec - tv1.tv_usec) + 1000000 * (tv2.tv_sec - tv1.tv_sec);
  }

  double t_total = (double)t_comm/1E6 + (double)t_comp/1E6;
  double t_total_max;
  
  printf("Proceso %d, Comunication Time (seconds): %lf, Computational Time (seconds): %lf, Total Time (seconds): %lf\n", rank, (double)t_comm/1E6, (double)t_comp/1E6, t_total);
  
  MPI_Reduce(&t_total, &t_total_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  
  if(rank == 0){
    printf("TIEMPO MAYOR\n");
    printf("Time (seconds) = %lf\n", t_total_max);

    if (DEBUG == 1) {
      int checksum = 0;
      for(int i=0;i<M;i++) checksum += result[i];
      printf("Checksum: %d\n", checksum);
    } else if (DEBUG == 2) {
      for(int i=0;i<M;i++) printf("%d \t", result[i]);
    }
    free(data1);
    free(data2);
    free(result);
  }

  free(recvbuff1);
  free(recvbuff2);
  free(local_result);

  MPI_Finalize();
  return 0;
}
