# CompParalela

Trabajos, laboratorios y ejercicios del curso **CS3P1 — Computación Paralela y Distribuida**, Escuela Profesional de Computación, Universidad Católica San Pablo (UCSP). 8vo semestre, 2026-II.

Sílabo completo: [SIlaboParalela.pdf](SIlaboParalela.pdf)

## Sobre el curso

La computación paralela y distribuida estudia la ejecución simultánea de múltiples procesos, cuyas operaciones pueden entrelazarse de forma compleja. El curso cubre concurrencia y ejecución en paralelo, consistencia y manejo de memoria compartida, comunicación por paso de mensajes, y tolerancia a fallos en sistemas distribuidos.

**Objetivos generales:**
- Crear aplicaciones paralelas de mediana complejidad aprovechando máquinas con múltiples núcleos.
- Comparar aplicaciones secuenciales y paralelas.
- Convertir aplicaciones secuenciales a paralelas de forma eficiente.

### Unidades del curso

1. **Bases teóricas de la Computación Paralela y Distribuida** — procesadores multinúcleo, memoria compartida vs distribuida, SIMD, GPU/coprocesamiento, taxonomía de Flynn, coherencia de caché, NUMA, topologías de interconexión, ley de Amdahl.
2. **Sistemas distribuidos** — fallos de red y de nodo, envío de mensajes distribuido (sockets, marshalling), compensaciones de diseño (latencia vs rendimiento, teorema CAP), diseño de servicios distribuidos, algoritmos de elección/descubrimiento.
3. **Comunicación y coordinación** — memoria compartida y consistencia, paso de mensajes (blocking/non-blocking), atomicidad, condiciones de carrera, deadlocks, semáforos, monitores, consensos, variables de condición, algoritmos paralelos segmentados.
4. **Programación para el Procesamiento Masivamente Paralelo** — GPU, SIMD/vectorización, gestión dinámica de memoria (malloc/free, garbage collection), consumo de energía en cómputo paralelo.

**Bibliografía:**
- Pacheco, Peter S. (2011). *An Introduction to Parallel Programming*. Morgan Kaufmann.
- Kirk, David B. and Wen-mei W. Hwu (2013). *Programming Massively Parallel Processors: A Hands-on Approach*. 2nd ed. Morgan Kaufmann.

## Contenido del repositorio

- [labs/hello_threads.c](labs/hello_threads.c) — primer ejemplo de threads en C.
- [labs/lab1/](labs/lab1/) — Laboratorio 1: ¿Más procesadores significa mayor velocidad?

## Laboratorio 1 — ¿Más procesadores significa mayor velocidad?

Enunciado: [Laboratorio_1_Mas_procesadores_mayor_velocidad.pdf](labs/lab1/Laboratorio_1_Mas_procesadores_mayor_velocidad.pdf)

Suma de un vector de `double` de tamaño N, comparando versión secuencial vs versión paralela con `std::thread`, midiendo speedup y eficiencia para distintas cantidades de threads y tamaños de N.

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
