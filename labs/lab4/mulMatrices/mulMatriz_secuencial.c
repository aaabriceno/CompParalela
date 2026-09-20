#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

int main(int argc, char** argv) {
    int N = 1024;
    if (argc > 1) N = atoi(argv[1]);

    int* A = malloc((size_t)N * N * sizeof(int));
    int* x = malloc(N * sizeof(int));
    long long* y = malloc(N * sizeof(long long));

    for (int i = 0; i < N * N; i++) A[i] = i + 1;
    for (int i = 0; i < N; i++) x[i] = i + 1;

    double start = get_time();

    for (int i = 0; i < N; i++) {
        long long suma = 0;
        for (int j = 0; j < N; j++) {
            suma += (long long)A[i * N + j] * x[j];
        }
        y[i] = suma;
    }

    double finish = get_time();

    printf("N = %d\n", N);
    printf("Tiempo secuencial = %f segundos\n", finish - start);
    printf("y[0] = %lld\n", y[0]);
    printf("y[%d] = %lld\n", N - 1, y[N - 1]);

    free(A);
    free(x);
    free(y);
    return 0;
}
