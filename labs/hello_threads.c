#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Funcion para ejecutar cada thread de mi programa
void* funcion_thread(void* argumento){
    int id = *(int*)argumento;
    printf("Hola desde thread %d (TID: %ld)\n",
            id, pthread_self());
    sleep(5);
    return NULL;
}

int main(){
    int numero_threads = 4;
    pthread_t threads[numero_threads];
    int identificador_thread[numero_threads];

    //creamos los threads
    for(int i = 0; i < numero_threads; i++){
        identificador_thread[i] = i;
        pthread_create(&threads[i],NULL,funcion_thread,&identificador_thread[i]);
    }

    for (int j = 0; j < numero_threads; j++){
        pthread_join(threads[j],NULL);
    }

    printf("Todos los threads terminaron\n");
    return 0;
}