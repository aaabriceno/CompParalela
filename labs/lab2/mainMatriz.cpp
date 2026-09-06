#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cstdlib>
using namespace std;

#define N 1000

int main(){
    double A[1000][1000];

    for (int i = 0; i < N;i++){
        for (int j = 0; j < N; j++){
            A[i][j] = i * 0.5 * j;
        }
    }

    //Recorrido por filas
    auto tiempo1 = chrono::steady_clock::now();
    
    for (int i = 0; i < N;i++){
        for (int j = 0; j < N; j++){
            A[i][j] *= 2.0;
        }
    }

    auto tiempo2 = chrono::steady_clock::now();
    auto tiempoFilas = chrono::duration_cast<chrono::microseconds>(tiempo2 - tiempo1);

    //Recorrido por columnas
    auto tiempo3 = chrono::steady_clock::now();

    for (int j = 0; j < N; j++){
        for (int i = 0; i < N; i++){
            A[i][j] *= 2.0;
        }
    }

    auto tiempo4 = chrono::steady_clock::now();
    auto tiempoColumnas = chrono::duration_cast<chrono::microseconds>(tiempo4 - tiempo3);
    /*
    cout << "Matriz A:\n";
    for (int i = 0; i < N;i++){
        for (int j = 0; j < N; j++){
            cout << A[i][j] << " ";
        }
        cout <<"\n";
    }
    */

    cout << "Tamaño de la matriz cuadrada: " << N << endl;
    cout << "Tiempo transcurrido para filas = " << tiempoFilas.count() << " microsegundos\n";
    cout << "Tiempo transcurrido para columnas = " << tiempoColumnas.count() << " microsegundos\n";
    return 0;
}
