#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]){
    if (argc < 2){
        cerr << "Uso: " << argv[0] << " <N>\n";
        return 1;
    }
    int N = atoi(argv[1]);

    vector<double> A(static_cast<size_t>(N) * N);

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            A[i*N+j] = i * 0.5 * j;
        }
    }

    // Parte C: recorrido por filas
    auto tiempo1 = chrono::steady_clock::now();
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            A[i*N+j] *= 2.0;
        }
    }
    auto tiempo2 = chrono::steady_clock::now();
    auto tiempoFilas = chrono::duration_cast<chrono::microseconds>(tiempo2 - tiempo1);

    // Parte D: recorrido por columnas
    auto tiempo3 = chrono::steady_clock::now();
    for (int j = 0; j < N; j++){
        for (int i = 0; i < N; i++){
            A[i*N+j] *= 2.0;
        }
    }
    auto tiempo4 = chrono::steady_clock::now();
    auto tiempoColumnas = chrono::duration_cast<chrono::microseconds>(tiempo4 - tiempo3);

    volatile double sumidero = A[N*N-1];

    cout << "N," << N
         << ",filas_us," << tiempoFilas.count()
         << ",columnas_us," << tiempoColumnas.count()
         << ",chk," << sumidero << "\n";

    return 0;
}
