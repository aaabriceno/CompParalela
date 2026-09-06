#include <mpi.h>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char** argv){
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0){
        vector<int> edades = {15, 25, 8, 40, 17, 60};

        for (int p = 1; p < size; p++){
            // MPI_Ssend no retorna hasta que el proceso destino ya haya
            // empezado a ejecutar su MPI_Recv correspondiente (rendezvous real).
            MPI_Ssend(&edades[p-1], 1, MPI_INT, p, 0, MPI_COMM_WORLD);
        }

        int resultadoRecibido;
        for (int p = 1; p < size; p++){
            MPI_Recv(&resultadoRecibido, 1, MPI_INT, p, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (resultadoRecibido == 1){
                cout << "Proceso " << p << ": es mayor de edad\n";
            } else {
                cout << "Proceso " << p << ": es menor de edad\n";
            }
        }

    } else {
        int edadRecibida;
        MPI_Recv(&edadRecibida, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int esMayorDeEdad = (edadRecibida >= 18) ? 1 : 0;

        MPI_Ssend(&esMayorDeEdad, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}