#include <stdio.h>
#include <stdlib.h>
#include <math.h> // ¿esto sobra?
#include <mpi.h>

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

int main (int argc, char *argv[]){
    MPI_Init(&argc, &argv); // Inicializa el entorno MPI

    int numprocesos, rank, namelen;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocesos); // Obtiene el número total de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtiene el rango del proceso

    int n, i,local_count = 0;
    char L, *cadena = NULL;

    if (rank == 0){ 
        if(argc != 3){
            //Verificamos que solo pasamos 2 elementos.
            printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
            exit(1); // ¿Ponemos exir(1) o MPI_Abort(MPI_COMM_WORLD,1)?
        }
        n = atoi(argv[1]); // Pasamos el primer argumento a entero.
        L = *argv[2]; // Toma el primer caracter del segundo argumento.
    }
    if (rank == 0) {
        // El proceso 0 envía n y L a todos los demás
        for(int i = 1; i < numprocesos; i++) {
            MPI_Send(&n, 1, MPI_INT,  i, 0, MPI_COMM_WORLD);
            MPI_Send(&L, 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
    } 
    else {
        // Los demás procesos reciben n y L, los procesos trabajadores reciben los datos.
        MPI_Recv(&n, 1, MPI_INT,  0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&L, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Todos los procesos crean y llenan la misma cadena completa.
    cadena = (char *) malloc(n*sizeof(char));
    if (cadena == NULL){
        printf("Error al reservar memoria\n");
        exit(1); // Lo mismo, ponemos exit(1) o MPI_Abort(MPI_COMM_WORLD,1)?
    }

    inicializaCadena(cadena, n);

    // Cada proceso cuenta solo las posiciones que le corresponden.
    for(i= rank; i<n; i+=numprocesos){
        if(cadena[i] == L){
            local_count++;
        }
    }
    
    if (rank == 0) {
        int total_count = local_count;   // el proceso 0 empieza con su parte

        // Recibe los conteos de todos los demás procesos.
        for(int i = 1; i < numprocesos; i++) {
            int recv_count;
            MPI_Recv(&recv_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_count += recv_count;
        }

        // Imprimimos el resultado final.
        printf("El numero de apareciones de la letra %c es %d\n", L, total_count);
    }
    else {
        // Los demás procesos envían su cuenta al proceso 0
        MPI_Send(&local_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    free(cadena); // Liberamos memoria.
    MPI_Finalize(); // Finaliza el entorno MPI
    exit(0);
}
