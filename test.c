#include <stdio.h>
#include <pthread.h>

int counter = 0; // Variable partagée

void *increment_routine(void *arg) {
    for (int i = 0; i < 800; i++) {
        counter++; // Le point critique !
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment_routine, NULL);
    pthread_create(&t2, NULL, increment_routine, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Counter = %d\n", counter);
    return 0;
}