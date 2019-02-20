#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
// #include <mutex>
#include <threads.cpp>

sem_t mutex;

void* thread2(void* arg){
  lock();
  printf("AHOY THERE\n");
  sleep(4);
  printf("AHOY THERE\n");
  sleep(4);
  unlock();
}


void* thread(void* arg)
{
    //wait
    // sem_wait(&mutex);
    printf("\nEntered..\n");
    pthread_t t2;
    pthread_create(&t2,NULL,thread2,NULL);
    // pthread_join(t2, NULL);

    //critical section
    sleep(4);

    //signal
    printf("\nJust Exiting...\n");
    // sem_post(&mutex);
}



int main()
{
    pthread_t t1;
    pthread_create(&t1,NULL,thread,NULL);
    sleep(2);

    return 0;
}