#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


void* thread2(void* arg){
  printf("AHOY THERE\n");
  return 0;
}


void* thread(void* arg)
{
    //wait
    // sem_wait(&mutex);
    printf("\nEntered..\n");
    pthread_t t2;
    void** retval;
    pthread_create(&t2,NULL,thread2,NULL);
    int result = pthread_join(t2, retval);
    if(result >= 0){
      printf("retval is: %p\n", retval);
    }
    printf("back in business bb\n");
    //critical section
    sleep(4);

    //signal
    printf("\nJust Exiting...\n");
    // sem_post(&mutex);
}



int main()
{
    // tests retval stuff
    // sem_init(&mutex, 0, 1);
    pthread_t t1;
    pthread_create(&t1,NULL,thread,NULL);
    void** retval;
    pthread_join(t1, retval);
    sleep(2);
    printf("retval is: %p\n", *retval);


    // pthread_join(t1,NULL);
    // pthread_join(t2,NULL);
    // sem_destroy(&mutex);
    return 0;
}
