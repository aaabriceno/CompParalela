#include <stdio.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) printf("Error: se requieren al menos 2 procesos\n");
        MPI_Finalize();
        return 1;
    }

    char message[100];
    if (rank == 0) {
        strcpy(message, "Hola desde rank 0");
        MPI_Send(message, strlen(message) + 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        printf("Rank %d: Enviado '%s' a rank 1\n", rank, message);

        MPI_Recv(message, 100, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank %d: Recibido '%s' de rank 1\n", rank, message);
    } else if (rank == 1) {
        MPI_Recv(message, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank %d: Recibido '%s' de rank 0\n", rank, message);

        strcpy(message, "Respuesta desde rank 1");
        MPI_Send(message, strlen(message) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        printf("Rank %d: Enviada '%s' a rank 0\n", rank, message);
    }

    MPI_Finalize();
    return 0;
}