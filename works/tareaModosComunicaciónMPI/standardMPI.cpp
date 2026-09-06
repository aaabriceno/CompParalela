#include <mpi.h>
#include <iostream>
#include <vector>

using namespace std;
#define N 12

int main(int argc, char ** argv){
    int rank, size; //declaracion de variables usadas en MPI
    MPI_Init(&argc, &argv);
    
    /*Devuelve el identificador del proceso como tal en este caso rank*/
    MPI_Comm_rank(MPI_COMM_WORLD,&rank); 
    /*Devuelve el tam o numero de procesos de un comunicador
    en este caso el comunicador es MPI_COMM_WORLD*/
    MPI_Comm_size(MPI_COMM_WORLD,&size); 

    
    int chunk = N / (size-1);
    if (rank == 0){
        vector<int>numeros;
        for (int i = 1; i <= N; i++){
            numeros.push_back(i);
        }

        for (int p = 1; p < size; p++){
            MPI_Send(&numeros[(p-1)*chunk],chunk, MPI_INT,p,0,MPI_COMM_WORLD);
        }

        long long multiplicacionTotal = 1;
        for (int p = 1; p < size; p++){
            long long multiplicacionParcial = 1;
            MPI_Recv(&multiplicacionParcial,1,MPI_LONG_LONG,p,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            multiplicacionTotal *= multiplicacionParcial;
        }
        cout << "Suma total: " << multiplicacionTotal << "\n";
    } else {
        vector<int> miParte(chunk);
        MPI_Recv(miParte.data(),chunk,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        long long multiplicacionParcial = 1;
        for (int v: miParte){
            multiplicacionParcial *= v;
        }
        MPI_Send(&multiplicacionParcial,1,MPI_LONG_LONG,0,1,MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}