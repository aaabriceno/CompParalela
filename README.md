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
