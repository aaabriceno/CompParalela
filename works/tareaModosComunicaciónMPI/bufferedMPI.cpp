#include <mpi.h>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char** argv){
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // MPI_Bsend necesita un buffer propio, separado del que maneja MPI internamente.
    // Aqui reservamos ese espacio y se lo entregamos a MPI con MPI_Buffer_attach.
    int tamanoBuffer = MPI_BSEND_OVERHEAD + sizeof(int);
    char* miBuffer = new char[tamanoBuffer];
    MPI_Buffer_attach(miBuffer, tamanoBuffer);

    if (rank == 0){
        vector<int> precios = {100, 250, 80, 500, 30, 999};

        for (int p = 1; p < size; p++){
            MPI_Bsend(&precios[p-1], 1, MPI_INT, p, 0, MPI_COMM_WORLD);
        }

        int precioConDescuentoRecibido;
        for (int p = 1; p < size; p++){
            MPI_Recv(&precioConDescuentoRecibido, 1, MPI_INT, p, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            cout << "Precio con descuento recibido del proceso " << p << ": " << precioConDescuentoRecibido << "\n";
        }

    } else {
        int precioOriginal;
        MPI_Recv(&precioOriginal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int precioConDescuento = precioOriginal - (precioOriginal * 20 / 100);

        MPI_Bsend(&precioConDescuento, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    // liberar el buffer usado por MPI_Bsend antes de finalizar
    MPI_Buffer_detach(&miBuffer, &tamanoBuffer);
    delete[] miBuffer;

    MPI_Finalize();
    return 0;
}
