#include <mpi.h>
#include <vector>
#include <cstdio>
#include <cstdlib>

using namespace std;

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // N por argumento (./mulMatrizMPI 4096), 1024 si no se especifica
    int N = (argc > 1) ? atoi(argv[1]) : 1024;
    // Si se pasa un 2do argumento "quiet", no imprime el vector resultado completo
    bool modoSilencioso = (argc > 2);

    char nombreNodo[MPI_MAX_PROCESSOR_NAME];
    int longitudNombre;
    MPI_Get_processor_name(nombreNodo, &longitudNombre);
    printf("Rank %d ejecutando en nodo %s\n", rank, nombreNodo);

    vector<int> matrizNumeros;
    vector<int> vectorNumeros(N);

    if (rank == 0) {
        matrizNumeros.resize(N * N);
        for (int i = 0; i < N * N; i++) {
            matrizNumeros[i] = i + 1;
        }
        for (int i = 0; i < N; i++) {
            vectorNumeros[i] = i + 1;
        }
    }

    // Vector es pequeño: todos lo necesitan completo
    MPI_Bcast(vectorNumeros.data(), N, MPI_INT, 0, MPI_COMM_WORLD);

    // Reparto de filas entre procesos (soporta N no divisible por size)
    vector<int> sendcounts(size), displs(size);
    int base = N / size, resto = N % size;
    int offset = 0;
    for (int p = 0; p < size; p++) {
        int filas = base + (p < resto ? 1 : 0);
        sendcounts[p] = filas * N;
        displs[p] = offset;
        offset += sendcounts[p];
    }

    int myRows = sendcounts[rank] / N;
    vector<int> matrizLocal(sendcounts[rank]);

    MPI_Scatterv(rank == 0 ? matrizNumeros.data() : nullptr,
                 sendcounts.data(), displs.data(), MPI_INT,
                 matrizLocal.data(), sendcounts[rank], MPI_INT,
                 0, MPI_COMM_WORLD);

    // Medir solo el cómputo local (sincronizado, para comparar de forma justa)
    MPI_Barrier(MPI_COMM_WORLD);
    double inicioTiempo = MPI_Wtime();

    // Cálculo local: cada proceso multiplica sus filas por el vector
    vector<long long> resultadoLocal(myRows, 0);
    for (int i = 0; i < myRows; i++) {
        long long suma = 0;
        for (int j = 0; j < N; j++) {
            suma += (long long)matrizLocal[i * N + j] * vectorNumeros[j];
        }
        resultadoLocal[i] = suma;
    }

    double finTiempo = MPI_Wtime();
    double tiempoLocal = finTiempo - inicioTiempo;
    double tiempoMax;
    MPI_Reduce(&tiempoLocal, &tiempoMax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Recolectar resultado (conteos/desplazamientos en filas, no en N)
    vector<int> recvcounts(size), rdispls(size);
    int roffset = 0;
    for (int p = 0; p < size; p++) {
        recvcounts[p] = base + (p < resto ? 1 : 0);
        rdispls[p] = roffset;
        roffset += recvcounts[p];
    }

    vector<long long> resultado;
    if (rank == 0) resultado.resize(N);

    MPI_Gatherv(resultadoLocal.data(), myRows, MPI_LONG_LONG,
                rank == 0 ? resultado.data() : nullptr,
                recvcounts.data(), rdispls.data(), MPI_LONG_LONG,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("N: %d\n", N);
        printf("Procesos: %d\n", size);
        printf("Tiempo: %f milisegundos\n", tiempoMax * 1000.0);
        if (!modoSilencioso) {
            for (int i = 0; i < N; i++) {
                printf("resultado[%d] = %lld\n", i, resultado[i]);
            }
        }
    }

    MPI_Finalize();
    return 0;
}