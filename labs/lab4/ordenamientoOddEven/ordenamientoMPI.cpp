#include <mpi.h>
#include <vector>
#include <cstdio>
#include <cstdlib>

using namespace std;

// Odd-Even Transposition Sort secuencial, usado para ordenar el bloque LOCAL
// de cada proceso. Mismo algoritmo que la versión secuencial pura (O(n^2)),
// para que la comparación de tiempos entre secuencial y MPI sea justa
// (ambos usan el mismo método de ordenamiento, solo cambia el tamaño de n).
void Odd_even_sort_local(vector<int>& a, int n) {
    int temporal;
    for (int fase = 0; fase < n; fase++) {
        if (fase % 2 == 0) {
            for (int i = 1; i < n; i += 2) {
                if (a[i-1] > a[i]) {
                    temporal = a[i]; a[i] = a[i-1]; a[i-1] = temporal;
                }
            }
        } else {
            for (int i = 1; i < n-1; i += 2) {
                if (a[i] > a[i+1]) {
                    temporal = a[i]; a[i] = a[i+1]; a[i+1] = temporal;
                }
            }
        }
    }
}

// Calcula el rank del proceso vecino ("partner") en esta fase.
// Si el partner queda fuera de rango, devuelve MPI_PROC_NULL:
// el Sendrecv con MPI_PROC_NULL no hace nada, así el proceso queda "idle" sin if extra.
int Compute_partner(int phase, int my_rank, int comm_sz) {
    int partner;
    if (phase % 2 == 0) {
        if (my_rank % 2 != 0) partner = my_rank - 1;
        else                  partner = my_rank + 1;
    } else {
        if (my_rank % 2 != 0) partner = my_rank + 1;
        else                  partner = my_rank - 1;
    }
    if (partner == -1 || partner == comm_sz) partner = MPI_PROC_NULL;
    return partner;
}

// Combina mis claves locales (ya ordenadas) con las del vecino (ya ordenadas)
// y me quedo solo con la mitad inferior (las local_n menores).
void Merge_low(vector<int>& my_keys, vector<int>& recv_keys, vector<int>& temp_keys, int local_n) {
    int m_i = 0, r_i = 0, t_i = 0;
    while (t_i < local_n) {
        if (my_keys[m_i] <= recv_keys[r_i]) {
            temp_keys[t_i] = my_keys[m_i];
            t_i++; m_i++;
        } else {
            temp_keys[t_i] = recv_keys[r_i];
            t_i++; r_i++;
        }
    }
    for (int i = 0; i < local_n; i++) my_keys[i] = temp_keys[i];
}

// Igual que Merge_low, pero me quedo con la mitad superior (las local_n mayores).
// Recorremos ambas listas de atrás hacia adelante.
void Merge_high(vector<int>& my_keys, vector<int>& recv_keys, vector<int>& temp_keys, int local_n) {
    int m_i = local_n - 1, r_i = local_n - 1, t_i = local_n - 1;
    while (t_i >= 0) {
        if (my_keys[m_i] >= recv_keys[r_i]) {
            temp_keys[t_i] = my_keys[m_i];
            t_i--; m_i--;
        } else {
            temp_keys[t_i] = recv_keys[r_i];
            t_i--; r_i--;
        }
    }
    for (int i = 0; i < local_n; i++) my_keys[i] = temp_keys[i];
}

int main(int argc, char** argv) {
    int rank, comm_sz;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    char nombreNodo[MPI_MAX_PROCESSOR_NAME];
    int longitudNombre;
    MPI_Get_processor_name(nombreNodo, &longitudNombre);
    printf("Rank %d ejecutando en nodo %s\n", rank, nombreNodo);

    // N por argumento (./ordenamientoMPI 10000), 10000 si no se especifica
    // N debe ser divisible por comm_sz (limitación del algoritmo, ver comentario en README)
    int N = (argc > 1) ? atoi(argv[1]) : 10000;
    bool modoSilencioso = (argc > 2);
    int local_n = N / comm_sz;

    vector<int> claves;      // solo se llena en rank 0
    vector<int> local_keys(local_n);

    if (rank == 0) {
        claves.resize(N);
        for (int i = 0; i < N; i++) claves[i] = N - i; // orden inverso, mismo caso que el secuencial
    }

    MPI_Scatter(rank == 0 ? claves.data() : nullptr, local_n, MPI_INT,
                local_keys.data(), local_n, MPI_INT,
                0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double inicioTiempo = MPI_Wtime();

    // Ordenar localmente antes de empezar las fases (mismo algoritmo que el secuencial)
    Odd_even_sort_local(local_keys, local_n);

    vector<int> recv_keys(local_n);
    vector<int> temp_keys(local_n);

    for (int phase = 0; phase < comm_sz; phase++) {
        int partner = Compute_partner(phase, rank, comm_sz);

        if (partner != MPI_PROC_NULL) {
            MPI_Sendrecv(local_keys.data(), local_n, MPI_INT, partner, 0,
                         recv_keys.data(), local_n, MPI_INT, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (rank < partner) {
                Merge_low(local_keys, recv_keys, temp_keys, local_n);
            } else {
                Merge_high(local_keys, recv_keys, temp_keys, local_n);
            }
        }
    }

    double finTiempo = MPI_Wtime();
    double tiempoLocal = finTiempo - inicioTiempo;
    double tiempoMax;
    MPI_Reduce(&tiempoLocal, &tiempoMax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    vector<int> resultado;
    if (rank == 0) resultado.resize(N);

    MPI_Gather(local_keys.data(), local_n, MPI_INT,
               rank == 0 ? resultado.data() : nullptr, local_n, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("N: %d\n", N);
        printf("Procesos: %d\n", comm_sz);
        printf("Tiempo: %f milisegundos\n", tiempoMax * 1000.0);
        if (!modoSilencioso) {
            printf("Numeros ordenados mediante Odd-Even Sort (MPI):\n");
            for (int i = 0; i < N; i++) printf("%d ", resultado[i]);
            printf("\n");
        }
    }

    MPI_Finalize();
    return 0;
}