# Laboratorio 1 — ¿Más procesadores significa mayor velocidad?

## Parte A

### 10⁸ numeros
```
80% 2026-08-19 22:25:39 ⌚  AnthonyDesktop in ~/Documentos/Semestre 2026-2/Paralela/labs/lab1
○ → ./main
Ingrese el valor del exponente para calcular la cantidad de numeros del vector: 8
N = 100000000
Suma = 5e+15
Tiempo transcurrido  = 239222 microsegundos
```
### 10⁷ numeros
```
80% 2026-08-19 22:27:33 ⌚  AnthonyDesktop in ~/Documentos/Semestre 2026-2/Paralela/labs/lab1
○ → ./main
Ingrese el valor del exponente para calcular la cantidad de numeros del vector: 7
N = 10000000
Suma = 5e+13
Tiempo transcurrido  = 32546 microsegundos

```

### 10⁶ numeros
```
80% 2026-08-19 22:27:42 ⌚  AnthonyDesktop in ~/Documentos/Semestre 2026-2/Paralela/labs/lab1
○ → ./main
Ingrese el valor del exponente para calcular la cantidad de numeros del vector: 6
N = 1000000
Suma = 5e+11
Tiempo transcurrido  = 3261 microsegundos
```

## Parte B

**¿Qué operaciones pueden realizarse independientemente?**
Cada suma parcial del vector, osea sumar un pedazo del arreglo (por ejemplo del indice 0 al 100) no depende de sumar otro pedazo (del 100 al 200). Cada thread puede ir sumando su propio pedazo sin necesitar el resultado de los demas.

**¿Qué información debe combinarse al finalizar el procesamiento?**
Al final hay que juntar las sumas que obtenemos de cada thread en una sola suma total (la reducción). Osea cada thread da un resultado y despues se suman todos esos resultados para tener el resultado final.

**¿Qué problema aparecería si varios threads modificaran directamente una misma variable de suma?**
Habria una condicion de carrera (race condition). Si dos threads leen el valor de la variable al mismo tiempo y despues escriben el resultado, uno de los dos incrementos se puede perder porque se sobreescribe el valor del otro thread. Eso hace que el resultado final sea incorrecto y ademas cambie cada vez que se ejecuta el programa.

## Parte C

## Parte D

## Parte E