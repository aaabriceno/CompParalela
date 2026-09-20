#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>

using namespace std;

void Odd_even_sort(vector<int> &a, int n){
    int fase, i,temporal;

    for (fase = 0; fase < n;fase++){
        if (fase % 2 == 0){
            for (i = 1; i < n; i += 2){
                if (a[i-1] > a[i]){
                    temporal = a[i];
                    a[i] = a[i-1];
                    a[i-1] = temporal;
                }
            }
        }
        else {
            for (i = 1; i < n-1; i += 2){
                if (a[i] > a[i+1]){
                    temporal = a[i];
                    a[i] = a[i+1];
                    a[i+1] = temporal;                    
                }
            }
        }
    }
}

int main(int argc, char ** argv){
    int N = (argc > 1) ? atoi(argv[1]) : 10000;
    bool modoSilencioso = (argc > 2);

    vector<int> numeros;
    for (int i = N; i > 0; i--){
        numeros.push_back(i);
    }
    int n = numeros.size();

    auto tiempoInicio = chrono::steady_clock::now();
    Odd_even_sort(numeros,n);
    auto tiempoFin = chrono::steady_clock::now();

    chrono::duration<double, milli> tiempoTotal = tiempoFin - tiempoInicio;

    cout << "N: " << n << "\n";
    cout << "Tiempo: " << tiempoTotal.count() << " milisegundos\n";
    if (!modoSilencioso) {
        cout << "Numeros ordenados mediante el ordenamiento Odd Even:\n";
        for (int i = 0; i < n; i++){
            cout << numeros[i] << " ";
        }
        cout << "\n";
    }
   
    
    return 0;
}