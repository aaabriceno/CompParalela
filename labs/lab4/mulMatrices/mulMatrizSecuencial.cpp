#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>
using namespace std;

int main(int argc, char** argv){
    int N = (argc > 1) ? atoi(argv[1]) : 1024;
    bool modoSilencioso = (argc > 2);

    vector<int> matrizNumeros;
    vector<int> vectorNumeros(N);
    vector<long long> matrizRespuesta(N);


    matrizNumeros.resize(N*N);
    for (int i = 0; i < N*N; i++){
        matrizNumeros[i] = i+1;
    }

    for (int i = 0; i < N; i++){
        vectorNumeros[i] = i+1;
    }

    auto inicioTiempo = chrono::steady_clock::now();
    for (int k = 0; k < N;k++){
        long long suma = 0;
        for (int l = 0; l < N; l++){
            suma += (long long)matrizNumeros[k * N+ l] * vectorNumeros[l];
        }
        matrizRespuesta[k] = suma;
    }
    auto FinTiempo = chrono::steady_clock::now();

    chrono::duration<double, milli> TiempoTotal = FinTiempo - inicioTiempo;

    cout << "N: " << N << "\n";
    cout << "Tiempo: " << TiempoTotal.count() << " milisegundos\n";
    if (!modoSilencioso) {
        for (int z = 0; z < N; z++){
            cout << matrizRespuesta[z] << "\n";
        }
    }

    return 0;  
}