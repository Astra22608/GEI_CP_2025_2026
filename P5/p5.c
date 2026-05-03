#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>

void inicializaCadena(char *cadena, int n){
  int i;
  for(i=0; i<n/2; i++){
    cadena[i] = 'A';
  }
  for(i=n/2; i<3*n/4; i++){
    cadena[i] = 'C';
  }
  for(i=3*n/4; i<9*n/10; i++){
    cadena[i] = 'G';
  }
  for(i=9*n/10; i<n; i++){
    cadena[i] = 'T';
  }
}

int MPI_FlattreeColectiva(void *buf,void *recvbuff, int count,MPI_Datatype datatype,MPI_Op op,int root,MPI_Comm comm){
  int rank,numprocs,i;
  MPI_Comm_size(comm,&numprocs);
  MPI_Comm_rank(comm,&rank);
  if(datatype != MPI_INT || op != MPI_SUM){
    return -1;
  }
  if(rank==root){
    int *recv = (int *)recvbuff;
    int *send = (int *)buf;
    for(i = 0; i < count; i++){
      recv[i] = send[i];
    }
    int *temp = malloc(count * sizeof(int));
    for(i=0;i<numprocs;i++){
      if(i!=root){
        MPI_Recv(temp,count,datatype,i,0,comm,MPI_STATUS_IGNORE);
        for(int j=0;j<count;j++){
          recv[j]+=temp[j];
        }
      }
    }
    free(temp);
  }
  else{
    MPI_Send(buf,count,datatype,root,0,comm);
  }
  return MPI_SUCCESS;
}

int MPI_BinomialColectiva(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
    int rank, numprocs, res;

    if (buf == NULL && count > 0) return MPI_ERR_ARG; 
    if (count < 0) return MPI_ERR_ARG;
    if (root != 0) return MPI_ERR_ROOT; // El algoritmo actual solo soporta root 0

    res = MPI_Comm_size(comm, &numprocs);
    if (res != MPI_SUCCESS) return res;
    
    res = MPI_Comm_rank(comm, &rank);
    if (res != MPI_SUCCESS) return res;

    int paso = 1;
    while(paso < numprocs){
        if(rank < paso){
            int dest = rank + paso;
            if(dest < numprocs){
                res = MPI_Send(buf, count, datatype, dest, 0, comm);
                if (res != MPI_SUCCESS) return res;
            }
        } else if(rank < 2 * paso){
            int source = rank - paso;
            res = MPI_Recv(buf, count, datatype, source, 0, comm, MPI_STATUS_IGNORE);
            if (res != MPI_SUCCESS) return res;
        }
        paso *= 2;
    }

    return MPI_SUCCESS;
}

int main(int argc,char *argv[]){
    int numprocs,rank,n,i;
    char L;
    char *cadena;

    MPI_Init(&argc,&argv);

    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if(rank==0){
      n = atoi(argv[1]);
      L = *argv[2];
    }

    if(rank==0){//solo proceso 0 inicializa la cadena
      
      cadena = (char *) malloc(n*sizeof(char));
      inicializaCadena(cadena, n);
    }

    MPI_BinomialColectiva(&n,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_BinomialColectiva(&L,1,MPI_CHAR,0,MPI_COMM_WORLD);
    if(rank != 0){
      cadena = (char *)malloc(n*sizeof(char));
    }
    MPI_BinomialColectiva(cadena, n, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    int count = 0;
    for(i=rank; i<n; i+=numprocs){
    if(cadena[i] == L){
        count++;
      }
    }
    int total = 0;
    MPI_FlattreeColectiva(&count, &total,1, MPI_INT, MPI_SUM,0,MPI_COMM_WORLD);
    if(rank == 0){
        printf("El numero de apariciones de la letra %c es %d\n", L, total);
    }
    free(cadena);

    MPI_Finalize();
  }
