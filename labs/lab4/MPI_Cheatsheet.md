# MPI — Guía de Repaso (Cap. 3 Pacheco, "An Introduction to Parallel Programming")

## 1. Estructura básica de todo programa MPI

```c
#include <mpi.h>

int main(int argc, char** argv) {
    int my_rank, comm_sz;

    MPI_Init(&argc, &argv);                    // SIEMPRE primero, antes de esto nada de MPI
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);   // total de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);   // mi id: 0..comm_sz-1

    /* ... código ... */

    MPI_Finalize();                            // SIEMPRE último
    return 0;
}
```

- `MPI_COMM_WORLD`: comunicador default = todos los procesos lanzados con `mpiexec -n <p>`.
- **SPMD** (Single-Program Multiple-Data): un solo ejecutable, pero cada proceso puede tomar caminos distintos según `my_rank` (típico `if (my_rank == 0) {...} else {...}`).
- Compilar: `mpicc -g -Wall -o programa programa.c` (o `mpic++` para C++)
- Ejecutar: `mpiexec -n <num_procesos> ./programa`

---

## 2. Comunicación Point-to-Point (uno a uno)

```c
int MPI_Send(
    void*         msg_buf_p,   /* in  */
    int           msg_size,    /* in  */
    MPI_Datatype  msg_type,    /* in  */
    int           dest,        /* in  */
    int           tag,         /* in  */
    MPI_Comm      communicator /* in  */);

int MPI_Recv(
    void*         msg_buf_p,   /* out */
    int           buf_size,    /* in  */
    MPI_Datatype  buf_type,    /* in  */
    int           source,      /* in  */
    int           tag,         /* in  */
    MPI_Comm      communicator,/* in  */
    MPI_Status*   status_p     /* out */);
```

### Matching (cómo se emparejan send/recv)
- `send.dest == recv.source` (o `recv.source == MPI_ANY_SOURCE`)
- `send.tag == recv.tag` (o `recv.tag == MPI_ANY_TAG`)
- Mismo comunicador.

### status_p
```c
MPI_Status status;
status.MPI_SOURCE   // de quién vino (útil si usaste MPI_ANY_SOURCE)
status.MPI_TAG       // qué tag traía
MPI_Get_count(&status, MPI_INT, &count);  // cuántos elementos llegaron realmente
```

### Comportamiento y peligro de deadlock
- `MPI_Recv` **siempre bloquea** hasta recibir mensaje que haga match.
- `MPI_Send` puede:
  - copiar a buffer interno MPI y retornar inmediatamente (mensajes chicos), o
  - bloquear hasta que el `Recv` correspondiente arranque (mensajes grandes).
- Un programa que depende de que `MPI_Send` bufferee es **inseguro (unsafe)**: puede funcionar con inputs chicos y colgarse (deadlock) con inputs grandes.

**Ejemplo de deadlock clásico:** todos los procesos hacen `Send` antes que `Recv`, mensaje grande → nadie llega a hacer `Recv` → todos esperan para siempre.

**Fix 1 — alternar por paridad de rank:**
```c
if (my_rank % 2 == 0) {
    MPI_Send(...);
    MPI_Recv(...);
} else {
    MPI_Recv(...);
    MPI_Send(...);
}
```

**Fix 2 — `MPI_Sendrecv`** (hace send + recv en una sola llamada, MPI se encarga de que no cuelgue):
```c
int MPI_Sendrecv(
    void*         send_buf_p,   int send_buf_size,  MPI_Datatype send_buf_type,
    int           dest,         int send_tag,
    void*         recv_buf_p,   int recv_buf_size,  MPI_Datatype recv_buf_type,
    int           source,       int recv_tag,
    MPI_Comm      communicator, MPI_Status* status_p);
```

**Fix 3 — `MPI_Ssend`** (synchronous send, bloquea garantizado hasta que el recv arranque — útil para *forzar* detectar bugs de safety durante desarrollo):
```c
MPI_Ssend(msg_buf_p, msg_size, msg_type, dest, tag, communicator);
```

---

## 3. Comunicación Colectiva (todos los procesos participan)

**Regla de oro:** TODOS los procesos del comunicador deben llamar la MISMA función colectiva, con argumentos "compatibles", en el MISMO orden. No usan tags — el matching es por orden de llamada. Mezclar una colectiva con un `Send/Recv` point-to-point es erróneo (cuelga o crashea).

### Broadcast — uno manda, todos reciben lo mismo
```c
int MPI_Bcast(
    void*         data_p,       /* in/out */
    int           count,        /* in */
    MPI_Datatype  datatype,     /* in */
    int           source_proc,  /* in */
    MPI_Comm      comm          /* in */);
```

### Reduce — combina datos de todos, resultado en UN proceso
```c
int MPI_Reduce(
    void*         input_data_p,   /* in  */
    void*         output_data_p,  /* out, solo válido en dest_process */
    int           count,          /* in  */
    MPI_Datatype  datatype,       /* in  */
    MPI_Op        operator,       /* in  */
    int           dest_process,   /* in  */
    MPI_Comm      comm            /* in  */);
```

Operadores predefinidos:

| Operador | Significado |
|---|---|
| `MPI_MAX` | Máximo |
| `MPI_MIN` | Mínimo |
| `MPI_SUM` | Suma |
| `MPI_PROD` | Producto |
| `MPI_LAND` | AND lógico |
| `MPI_BAND` | AND bit a bit |
| `MPI_LOR` | OR lógico |
| `MPI_BOR` | OR bit a bit |
| `MPI_LXOR` | XOR lógico |
| `MPI_BXOR` | XOR bit a bit |
| `MPI_MAXLOC` | Máximo + su ubicación |
| `MPI_MINLOC` | Mínimo + su ubicación |

⚠️ **Nota importante (trampa de examen):** el matching de llamadas a `MPI_Reduce` se hace por **orden de llamada**, no por nombre de variable. Si proceso A hace `Reduce(&a,...)` seguido de `Reduce(&c,...)`, y proceso B hace `Reduce(&c,...)` seguido de `Reduce(&a,...)`, los valores se mezclan según el ORDEN, no el nombre.

### Allreduce — como Reduce pero el resultado va a TODOS
```c
int MPI_Allreduce(
    void*         input_data_p,
    void*         output_data_p,  /* out, válido en TODOS */
    int           count,
    MPI_Datatype  datatype,
    MPI_Op        operator,
    MPI_Comm      comm);
```
Internamente es más eficiente que Reduce+Bcast por separado (usa estructura tipo "butterfly", log(p) pasos).

### Scatter — reparte pedazos distintos desde un proceso a todos
```c
int MPI_Scatter(
    void*         send_buf_p,   /* in, solo relevante en src_proc */
    int           send_count,   /* in */
    MPI_Datatype  send_type,    /* in */
    void*         recv_buf_p,   /* out */
    int           recv_count,   /* in */
    MPI_Datatype  recv_type,    /* in */
    int           src_proc,     /* in */
    MPI_Comm      comm          /* in */);
```

### Gather — inverso del Scatter, junta pedazos en UN proceso
```c
int MPI_Gather(
    void*         send_buf_p,
    int           send_count,
    MPI_Datatype  send_type,
    void*         recv_buf_p,   /* out, solo relevante en dest_proc */
    int           recv_count,
    MPI_Datatype  recv_type,
    int           dest_proc,
    MPI_Comm      comm);
```

### Allgather — como Gather pero TODOS reciben el resultado completo
```c
int MPI_Allgather(
    void*         send_buf_p,
    int           send_count,
    MPI_Datatype  send_type,
    void*         recv_buf_p,   /* out, válido en TODOS */
    int           recv_count,
    MPI_Datatype  recv_type,
    MPI_Comm      comm);
```
Concatena el `send_buf_p` de cada proceso en el `recv_buf_p` de todos.

### Barrier — sincroniza a todos en un punto
```c
int MPI_Barrier(MPI_Comm comm);
```
Nadie retorna de esta llamada hasta que TODOS los procesos del comunicador la hayan alcanzado. Uso típico: sincronizar antes de medir tiempo.

---

## 4. Ejemplo clásico: Regla del Trapecio en MPI

Pseudocódigo paralelo (patrón general aplicable a muchos problemas):

```
1. Get a, b, n
2. h = (b-a)/n
3. local_n = n / comm_sz
4. local_a = a + my_rank * local_n * h
5. local_b = local_a + local_n * h
6. local_integral = Trap(local_a, local_b, local_n, h)
7. if (my_rank != 0)
       Send local_integral a proceso 0
   else
       total_integral = local_integral
       for proc = 1 hasta comm_sz-1:
           Receive local_integral de proc
           total_integral += local_integral
8. if (my_rank == 0) print total_integral
```

Este mismo patrón (dividir → calcular local → combinar) es la base de casi todo problema con MPI: multiplicación matriz-vector, suma de vectores, sorting, etc.

---

## 5. Multiplicación Matriz-Vector (ejemplo completo aplicado)

```
A es matriz m×n, x es vector de n componentes
y = Ax es vector de m componentes
y[i] = A[i][0]*x[0] + A[i][1]*x[1] + ... + A[i][n-1]*x[n-1]
     = producto punto de la fila i de A con x
```

Almacenamiento en C (row-major, linealizado):
```
A[i][j]  se guarda como  A[i*n + j]
```

### Versión serial
```c
void Mat_vect_mult(double A[], double x[], double y[], int m, int n) {
    int i, j;
    for (i = 0; i < m; i++) {
        y[i] = 0.0;
        for (j = 0; j < n; j++)
            y[i] += A[i*n+j] * x[j];
    }
}
```

### Versión paralela con Allgather (patrón del libro)
Cada proceso ya tiene su porción `local_x` del vector (viene de un cálculo anterior o de un Scatter previo). Se usa `Allgather` para que todos tengan el vector `x` completo antes de calcular:

```c
void Mat_vect_mult(
        double local_A[], double local_x[], double local_y[],
        int local_m, int n, int local_n, MPI_Comm comm) {
    double* x;
    int local_i, j;

    x = malloc(n * sizeof(double));
    MPI_Allgather(local_x, local_n, MPI_DOUBLE,
                   x,       local_n, MPI_DOUBLE, comm);

    for (local_i = 0; local_i < local_m; local_i++) {
        local_y[local_i] = 0.0;
        for (j = 0; j < n; j++)
            local_y[local_i] += local_A[local_i*n+j] * x[j];
    }
    free(x);
}
```

**Nota:** si el vector `x` completo ya existe centralizado en un solo proceso (rank 0) desde el inicio, se usa `Bcast` en vez de `Allgather` — es más simple y directo para ese caso. `Allgather` tiene sentido cuando el vector nace repartido entre procesos (ej. multiplicaciones encadenadas).

---

## 6. Derived Datatypes (tipos derivados)

Sirven para mandar en una sola llamada una colección de datos de tipos mixtos (ej. 2 doubles + 1 int), evitando 3 llamadas `Bcast`/`Send` separadas.

```c
MPI_Get_address(location_p, &address_p);
// da la dirección de memoria de una variable; MPI_Aint = tipo entero
// suficientemente grande para guardar una dirección
```

```c
int MPI_Type_create_struct(
    int            count,                  /* in  — num de elementos */
    int            array_of_blocklengths[],/* in  — cuántos de c/tipo */
    MPI_Aint       array_of_displacements[],/* in — offset de c/uno */
    MPI_Datatype   array_of_types[],       /* in  — tipo de c/uno */
    MPI_Datatype*  new_type_p              /* out */);

MPI_Type_commit(&new_type_p);   // obligatorio antes de usar el tipo
MPI_Type_free(&old_type_p);     // liberar cuando ya no se necesita
```

### Ejemplo completo: mandar a,b,n (double,double,int) en 1 sola llamada
```c
void Build_mpi_type(double* a_p, double* b_p, int* n_p, MPI_Datatype* input_mpi_t_p) {
    int array_of_blocklengths[3] = {1, 1, 1};
    MPI_Datatype array_of_types[3] = {MPI_DOUBLE, MPI_DOUBLE, MPI_INT};
    MPI_Aint a_addr, b_addr, n_addr;
    MPI_Aint array_of_displacements[3] = {0};

    MPI_Get_address(a_p, &a_addr);
    MPI_Get_address(b_p, &b_addr);
    MPI_Get_address(n_p, &n_addr);
    array_of_displacements[1] = b_addr - a_addr;
    array_of_displacements[2] = n_addr - a_addr;

    MPI_Type_create_struct(3, array_of_blocklengths,
        array_of_displacements, array_of_types, input_mpi_t_p);
    MPI_Type_commit(input_mpi_t_p);
}

// Uso:
MPI_Datatype input_mpi_t;
Build_mpi_type(a_p, b_p, n_p, &input_mpi_t);
MPI_Bcast(a_p, 1, input_mpi_t, 0, MPI_COMM_WORLD);  // manda los 3 juntos
MPI_Type_free(&input_mpi_t);
```

---

## 7. Medición de rendimiento (Performance Evaluation)

### Tiempo transcurrido
```c
double start, finish;
start = MPI_Wtime();
/* código a medir */
finish = MPI_Wtime();
printf("Tiempo = %e segundos\n", finish - start);
```

### Medición correcta con Barrier (sincronizar antes de medir)
```c
double local_start, local_finish, local_elapsed, elapsed;

MPI_Barrier(comm);                    // todos arrancan al mismo tiempo
local_start = MPI_Wtime();
/* código a medir */
local_finish = MPI_Wtime();
local_elapsed = local_finish - local_start;

// tomar el peor caso (el proceso más lento) como tiempo real
MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

if (my_rank == 0)
    printf("Tiempo transcurrido = %e segundos\n", elapsed);
```

### Fórmulas clave (muy probable en examen)

```
Speedup:     S(n,p) = T_serial(n) / T_parallel(n,p)

Efficiency:  E(n,p) = S(n,p) / p  =  T_serial(n) / (p * T_parallel(n,p))
```

- **Speedup ideal** = p (lineal). En la práctica siempre S(n,p) ≤ p.
- **Efficiency** siempre entre 0 y 1 (0% a 100%).

### Escalabilidad (Scalability)
- **Strongly scalable**: mantiene eficiencia constante SIN necesidad de aumentar el tamaño del problema al aumentar p.
- **Weakly scalable**: mantiene eficiencia constante SI el tamaño del problema aumenta a la misma tasa que p.

Ejemplo de tabla típica de examen (tiempos en segundos, matriz-vector):

| comm_sz | n=1024 | n=2048 | n=4096 | n=8192 | n=16384 |
|---|---|---|---|---|---|
| 1 | 4.1 | 16.0 | 64.0 | 270 | 1100 |
| 2 | 2.3 | 8.5 | 33.0 | 140 | 560 |
| 4 | 2.0 | 5.1 | 18.0 | 70 | 280 |
| 8 | 1.7 | 3.3 | 9.8 | 36 | 140 |
| 16 | 1.7 | 2.6 | 5.9 | 19 | 71 |

De esta tabla se derivan speedup y efficiency dividiendo cada celda por la de comm_sz=1 (para speedup) y luego por p (para efficiency). Nota cómo la eficiencia cae fuerte para n chico con p grande (n=1024, p=16 → efficiency=0.15) pero se mantiene alta para n grande (n=16384, p=16 → efficiency=0.97): esto es exactamente lo que significa que el problema "escala mejor" con matrices grandes.

---

## 8. Sorting paralelo — Odd-Even Transposition Sort

Algoritmo de ejemplo típico de examen para ilustrar comunicación par-impar entre vecinos.

### Idea serial
- Fases pares: compara-intercambia (a[0],a[1]), (a[2],a[3]), (a[4],a[5])...
- Fases impares: compara-intercambia (a[1],a[2]), (a[3],a[4])...
- Se repite n veces (garantiza quedar ordenado).

```c
void Odd_even_sort(int a[], int n) {
    int phase, i, temp;
    for (phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) {           // fase par
            for (i = 1; i < n; i += 2)
                if (a[i-1] > a[i]) { temp=a[i]; a[i]=a[i-1]; a[i-1]=temp; }
        } else {                          // fase impar
            for (i = 1; i < n-1; i += 2)
                if (a[i] > a[i+1]) { temp=a[i]; a[i]=a[i+1]; a[i+1]=temp; }
        }
    }
}
```

### Versión paralela (cada proceso tiene un bloque de claves)
Patrón general:
```
Ordenar claves locales (ej. con qsort)
for (phase = 0; phase < comm_sz; phase++) {
    partner = Compute_partner(phase, my_rank);
    if (no estoy inactivo) {
        Enviar mis claves a partner;
        Recibir claves de partner;
        if (my_rank < partner)  quedarme con las menores;
        else                     quedarme con las mayores;
    }
}
```

`Compute_partner`: en fase par, ranks pares intercambian con rank+1, ranks impares con rank-1. En fase impar, al revés. Si el partner calculado es -1 o comm_sz → usar `MPI_PROC_NULL` (proceso "fantasma" que hace que Send/Recv no hagan nada, sin necesidad de un `if` extra).

Este algoritmo usa `MPI_Sendrecv` típicamente para evitar deadlock al intercambiar con el vecino.

---

## 9. Tabla-resumen rápida (para repaso exprés antes del examen)

| Función | Qué hace | Tipo |
|---|---|---|
| `MPI_Init` / `MPI_Finalize` | Setup / cleanup | Obligatorio |
| `MPI_Comm_size` / `MPI_Comm_rank` | Total procesos / mi id | Consulta |
| `MPI_Send` / `MPI_Recv` | Mandar/recibir uno a uno | Point-to-point |
| `MPI_Sendrecv` | Send+Recv en 1 call, sin deadlock | Point-to-point |
| `MPI_Ssend` | Send síncrono (bloquea garantizado) | Point-to-point |
| `MPI_Bcast` | 1 proceso → todos (mismo dato) | Colectiva |
| `MPI_Reduce` | Todos → 1 proceso (combinado con op) | Colectiva |
| `MPI_Allreduce` | Todos → todos (combinado con op) | Colectiva |
| `MPI_Scatter` | 1 proceso → todos (pedazos distintos) | Colectiva |
| `MPI_Gather` | Todos → 1 proceso (junta pedazos) | Colectiva |
| `MPI_Allgather` | Todos → todos (junta pedazos) | Colectiva |
| `MPI_Barrier` | Sincroniza a todos en un punto | Colectiva |
| `MPI_Wtime` | Tiempo actual (para medir performance) | Utilidad |
| `MPI_Type_create_struct` + `MPI_Type_commit` | Crear tipo derivado (datos mixtos) | Datatype |
| `MPI_Get_address` | Dirección de memoria de una variable | Datatype |

**Reglas de oro para no perder puntos en examen:**
1. Colectivas: TODOS los procesos del comunicador deben llamarlas, mismo orden, argumentos compatibles.
2. `MPI_Recv` siempre bloquea; `MPI_Send` puede o no — no confiar en que bufferee (programa "unsafe" si depende de eso).
3. Row-major en C: `A[i][j]` → `A[i*n+j]`.
4. Cuidado con `n` no divisible por `comm_sz`: repartir con `n/comm_sz` + resto extra a los primeros procesos (ver Scatterv/Gatherv si el reparto no es uniforme).
5. Medir tiempo real: `MPI_Barrier` antes de `MPI_Wtime`, y `MPI_Reduce` con `MPI_MAX` al final para tomar el peor caso entre todos los procesos.
