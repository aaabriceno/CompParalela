#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    char hostname[MPI_MAX_PROCESSOR_NAME];
    int name_len;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(hostname, &name_len);

    int variable_local = rank * 100;

    printf("Proceso %d - %s - direccion: %p - valor: %d\n",
           rank, hostname, (void*)&variable_local, variable_local);

    MPI_Finalize();
    return 0;
}