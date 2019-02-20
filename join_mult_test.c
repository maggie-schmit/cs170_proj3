#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex;
int x = 0;

void* thread3(void* arg){
  printf("in thread 3!\n");
  x = x+1;
}

void* thread2(void* arg){
  printf("AHOY THERE\n");
  pthread_t t3, t4;
  pthread_create(&t3, NULL, thread3, NULL);
  pthread_create(&t4, NULL, thread3, NULL);
  while(x == 0){
    sleep(1);
  }
  printf("AHOY THERE 2\n");
}


void* thread(void* arg)
{
    //wait
    // sem_wait(&mutex);
    printf("\nEntered..\n");
    pthread_t t2;
    pthread_create(&t2,NULL,thread2,NULL);
    pthread_join(t2, NULL);
    printf("back in business bb\n");
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
