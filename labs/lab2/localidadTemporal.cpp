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

    // Solo la columna 0 de una matriz N x N importa para este experimento;
    // usar un vector de tamaño N evita reservar N*N memoria innecesaria.
    // esta variable es volatile esto quiere decir que el compilador no puede eliminar
    // las escrituras de él
    volatile double sumidero = 0.0;

    // Patrón 1: reutiliza A[i][0] tres veces seguidas (localidad temporal alta)
    vector<double> A1(N, 0.0);
    auto t1 = chrono::steady_clock::now();
    for (int i = 0; i < N; i++){
        A1[i] += 1;
        A1[i] += 1;
        A1[i] += 1;
    }
    auto t2 = chrono::steady_clock::now();
    auto tiempoReutilizado = chrono::duration_cast<chrono::microseconds>(t2 - t1);
    for (int i = 0; i < N; i++) sumidero += A1[i];

    // Patrón 2: tres pasadas completas separadas sobre la misma columna (localidad temporal baja)
    vector<double> A2(N, 0.0);
    auto t3 = chrono::steady_clock::now();
    for (int k = 0; k < 3; k++){
        for (int i = 0; i < N; i++){
            A2[i] += 1;
        }
    }
    auto t4 = chrono::steady_clock::now();
    auto tiempoSeparado = chrono::duration_cast<chrono::microseconds>(t4 - t3);
    for (int i = 0; i < N; i++) sumidero += A2[i];

    cout << "N," << N
         << ",reutilizado_us," << tiempoReutilizado.count()
         << ",separado_us," << tiempoSeparado.count() << "\n";

    return 0;
}
