#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>
using namespace std;

#define VALOR 10

void sumar_parcial(double *numeros, int inicio, int fin, double *resultado_parcial){
    double suma = 0.0;
    for(int i = inicio; i < fin; i++){
        suma += numeros[i];
    }
    *resultado_parcial = suma;
}

int main(){
    int exponente = 0;
    int NUM_THREADS = 0;
    cout << "Ingrese el valor del exponente para calcular la cantidad de numeros del vector: "; cin >> exponente;
    if (exponente < 6 || exponente > 8){
        exponente = 8;
    }

    cout << "Ingrese la cantidad de threads a usar (2,4,8,16,32): "; cin >> NUM_THREADS;
    if (NUM_THREADS < 2 || NUM_THREADS > 32){
        NUM_THREADS = 32;
    }

    int cantidad_numeros = pow(VALOR,exponente);
    vector<double> numeros(cantidad_numeros);
    
    for(int i = 0; i < cantidad_numeros; i++){
        numeros[i] = i;
    }

    double respuesta = 0.0;
    thread hilos[NUM_THREADS];
    double parciales[NUM_THREADS];
    int bloque = cantidad_numeros / NUM_THREADS;

    auto inicio = chrono::steady_clock::now();

    for(int t = 0; t < NUM_THREADS; t++){
        int inicio = t * bloque;
        int fin = (t == NUM_THREADS - 1) ? cantidad_numeros : inicio + bloque;
        hilos[t] = thread(sumar_parcial, numeros.data(), inicio, fin, &parciales[t]);
    }

    for(int t = 0; t < NUM_THREADS; t++){
        hilos[t].join();
    }

    for(int t = 0; t < NUM_THREADS; t++){
        respuesta += parciales[t];
    }

    auto fin = chrono::steady_clock::now();
    auto duracion = chrono::duration_cast<chrono::microseconds>(fin - inicio);

    cout << "cantidad_numeros = " << cantidad_numeros << endl;
    cout << "Threads = " << NUM_THREADS << endl;
    cout << "Suma = " << respuesta << endl;
    cout << "Tiempo transcurrido = " << duracion.count() << " microsegundos" << endl;

    return 0;
}
