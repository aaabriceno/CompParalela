# CompParalela

Trabajos y laboratorios del curso de Computación Paralela y Distribuida.

## Contenido

- [labs/hello_threads.c](labs/hello_threads.c) — primer ejemplo de threads en C.
- [labs/lab1/](labs/lab1/) — Laboratorio 1: ¿Más procesadores significa mayor velocidad?

## Laboratorio 1

Enunciado: [Laboratorio_1_Mas_procesadores_mayor_velocidad.pdf](labs/lab1/Laboratorio_1_Mas_procesadores_mayor_velocidad.pdf)

Suma de un vector de `double` de tamaño N, comparando versión secuencial vs versión paralela con `std::thread`.

### Código fuente

- [main.cpp](labs/lab1/main.cpp) — versión secuencial.
- [main2.cpp](labs/lab1/main2.cpp) — versión paralela con `std::thread`.
- [pruebasSecuenciales.py](labs/lab1/pruebasSecuenciales.py) — automatiza corridas de `main.cpp` y calcula el tiempo base (T1) por cada tamaño N.
- [pruebasParalelas.py](labs/lab1/pruebasParalelas.py) — automatiza corridas de `main2.cpp` variando threads y N, calcula speedup y eficiencia, genera el gráfico de speedup.

### Resultados

- [resultados_secuencial.csv](labs/lab1/resultados_secuencial.csv)
- [resultados_paralelo.csv](labs/lab1/resultados_paralelo.csv)
- [grafico_speedup.png](labs/lab1/grafico_speedup.png)

### Respuestas y análisis

- [respuesta.md](labs/lab1/respuesta.md) — respuestas a las preguntas de análisis del laboratorio.
- [trabajoLab1.odt](labs/lab1/trabajoLab1.odt) — informe completo.

### Cómo correr

```bash
# Compilar
g++ -O2 main.cpp -o main
g++ -O2 -pthread main2.cpp -o main2

# Automatizar pruebas (requiere pandas y matplotlib)
python3 pruebasSecuenciales.py
python3 pruebasParalelas.py
```
