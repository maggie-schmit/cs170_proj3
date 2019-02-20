#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex;

void* thread2(void* arg){
  printf("AHOY THERE\n");
  sleep(4);
  printf("AHOY THERE\n");
  sleep(4);
}


void* thread(void* arg)
{
    //wait
    // sem_wait(&mutex);
    printf("\nEntered..\n");
    pthread_t t2;
    pthread_create(&t2,NULL,thread2,NULL);
    pthread_join(t2, NULL);

    //critical section
    sleep(4);

    //signal
    printf("\nJust Exiting...\n");
    // sem_post(&mutex);
}



int main()
{
    // sem_init(&mutex, 0, 1);
    pthread_t t1;
    pthread_create(&t1,NULL,thread,NULL);
    sleep(2);

    // pthread_join(t1,NULL);
    // pthread_join(t2,NULL);
    // sem_destroy(&mutex);
    return 0;
}
