#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int* matrizNumeros = NULL;
    int* vectorNumeros = malloc(N * sizeof(int));

    if (rank == 0) {
        matrizNumeros = malloc((size_t)N * N * sizeof(int));
        for (int i = 0; i < N * N; i++) {
            matrizNumeros[i] = i + 1;
        }
        for (int i = 0; i < N; i++) {
            vectorNumeros[i] = i + 1;
        }
    }

    // Vector es pequeño: todos lo necesitan completo
    MPI_Bcast(vectorNumeros, N, MPI_INT, 0, MPI_COMM_WORLD);

    // Reparto de filas entre procesos (soporta N no divisible por size)
    int* sendcounts = malloc(size * sizeof(int));
    int* displs = malloc(size * sizeof(int));
    int base = N / size, resto = N % size;
    int offset = 0;
    for (int p = 0; p < size; p++) {
        int filas = base + (p < resto ? 1 : 0);
        sendcounts[p] = filas * N;
        displs[p] = offset;
        offset += sendcounts[p];
    }

    int myRows = sendcounts[rank] / N;
    int* matrizLocal = malloc(sendcounts[rank] * sizeof(int));

    MPI_Scatterv(rank == 0 ? matrizNumeros : NULL,
                 sendcounts, displs, MPI_INT,
                 matrizLocal, sendcounts[rank], MPI_INT,
                 0, MPI_COMM_WORLD);

    // Cálculo local: cada proceso multiplica sus filas por el vector
    long long* resultadoLocal = malloc(myRows * sizeof(long long));
    for (int i = 0; i < myRows; i++) {
        long long suma = 0;
        for (int j = 0; j < N; j++) {
            suma += (long long)matrizLocal[i * N + j] * vectorNumeros[j];
        }
        resultadoLocal[i] = suma;
    }

    // Recolectar resultado (conteos/desplazamientos en filas, no en N)
    int* recvcounts = malloc(size * sizeof(int));
    int* rdispls = malloc(size * sizeof(int));
    int roffset = 0;
    for (int p = 0; p < size; p++) {
        recvcounts[p] = base + (p < resto ? 1 : 0);
        rdispls[p] = roffset;
        roffset += recvcounts[p];
    }

    long long* resultado = NULL;
    if (rank == 0) resultado = malloc(N * sizeof(long long));

    MPI_Gatherv(resultadoLocal, myRows, MPI_LONG_LONG,
                rank == 0 ? resultado : NULL,
                recvcounts, rdispls, MPI_LONG_LONG,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (int i = 0; i < N; i++) {
            printf("resultado[%d] = %lld\n", i, resultado[i]);
        }
    }

    free(vectorNumeros);
    free(sendcounts);
    free(displs);
    free(matrizLocal);
    free(resultadoLocal);
    free(recvcounts);
    free(rdispls);
    if (rank == 0) {
        free(matrizNumeros);
        free(resultado);
    }

    MPI_Finalize();
    return 0;
}
