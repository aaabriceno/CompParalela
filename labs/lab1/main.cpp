#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

using namespace std;
#define VALOR 10


int main(){
    int exponente = 0;

    cout << "Ingrese el valor del exponente para calcular la cantidad de numeros del vector: "; cin >> exponente;
    if (exponente < 6 || exponente > 8){
        exponente = 8;
    }

    int cantidad_numeros = pow(VALOR,exponente);
    vector<double> numeros(cantidad_numeros);
    
    double respuesta = 0.0;

    for (int i = 0; i < cantidad_numeros; i++){
        numeros[i] = i;
    }

    /*uso de la libreria chronos para poder determinar el tiempo 
      que transcurrió a la hora de sumar los elementos del vector*/
    auto inicio = chrono::steady_clock::now();
    
    for (int i = 0; i < cantidad_numeros; i++){
        respuesta = respuesta + numeros[i];
    }
    
    auto fin = chrono::steady_clock::now();
    auto duracion = chrono::duration_cast<chrono::microseconds>(fin - inicio);

    cout << "N = " << cantidad_numeros << "\n";
    cout << "Suma = " << respuesta << "\n";
    cout << "Tiempo transcurrido  = " << duracion.count() << " microsegundos\n";
    return 0;
}