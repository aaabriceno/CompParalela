#include <mpi.h>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char**argv){
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    int temperaturaRecibida;
    MPI_Request miSolicitud;

    if (rank != 0){
        MPI_Irecv(&temperaturaRecibida,1,MPI_INT,0,0,MPI_COMM_WORLD,&miSolicitud);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0){
        vector <int> temperaturas = {12,34,45,56,22,80};
        int elementosVectorTemperaturas = temperaturas.size();
        for (int p = 1; p < size;p++){
            MPI_Rsend(&temperaturas[p-1],1,MPI_INT,p,0,MPI_COMM_WORLD);
        }
    }

    if ( rank != 0){
        MPI_Wait(&miSolicitud,MPI_STATUS_IGNORE);
        
        if (temperaturaRecibida < 10){
            cout << "La tempetatura con valor " << temperaturaRecibida << "° es Fria\n";
        }
        if (temperaturaRecibida >=10 && temperaturaRecibida < 30){
            cout << "La tempetatura con valor " << temperaturaRecibida << "° es Normal\n";
        }
        else{
            cout << "La tempetatura con valor " << temperaturaRecibida << "° Caliente\n";
        }
    }

    MPI_Finalize();
    return 0;
}