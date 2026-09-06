#include <mpi.h>
#include <iostream>
#include <vector>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv){
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int edadRecibida;
    MPI_Request solicitudRecepcion;

    // todos los procesos que no son el rank 0 dejan lista su recepcion,
    // sin bloquearse, usando MPI_Irecv
    if (rank != 0){
        MPI_Irecv(&edadRecibida, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &solicitudRecepcion);
    }

    if (rank == 0){
        vector<int> edades = {15, 25, 8, 40, 17, 60};
        vector<MPI_Request> solicitudesEnvio(size);

        // el envio tampoco bloquea: MPI_Isend retorna de inmediato para cada proceso
        for (int p = 1; p < size; p++){
            MPI_Isend(&edades[p-1], 1, MPI_INT, p, 0, MPI_COMM_WORLD, &solicitudesEnvio[p]);
        }

        // como los envios no bloquean, antes de terminar hay que confirmar
        // que cada uno realmente completo, usando MPI_Wait
        for (int p = 1; p < size; p++){
            MPI_Wait(&solicitudesEnvio[p], MPI_STATUS_IGNORE);
        }
        cout << "Rank 0: confirmado que todos los envios se completaron\n";

    } else if (rank % 2 == 0){
        // procesos con rank par: esperan el dato de forma bloqueante con MPI_Wait
        MPI_Wait(&solicitudRecepcion, MPI_STATUS_IGNORE);
        cout << "Proceso " << rank << " (uso Wait) recibio edad " << edadRecibida << "\n";

    } else {
        // procesos con rank impar: consultan repetidamente con MPI_Test
        // si el dato ya llego, sin bloquearse, y mientras tanto pueden hacer otra cosa
        int yaLlego = 0;
        int intentos = 0;
        while (!yaLlego){
            MPI_Test(&solicitudRecepcion, &yaLlego, MPI_STATUS_IGNORE);
            intentos++;
            if (!yaLlego){
                usleep(1000); // simula trabajo util mientras se espera el dato
            }
        }
        cout << "Proceso " << rank << " (uso Test, " << intentos
             << " intentos) recibio edad " << edadRecibida << "\n";
    }

    MPI_Finalize();
    return 0;
}
