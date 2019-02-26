#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
struct timeval tv;

#define wait(i) ({tv.tv_sec = i;tv.tv_usec=500000;select(0,NULL,NULL,NULL,&tv);})



void* thread4(void* arg){
  printf("I should exit before my brother wakes up\n");
  pthread_exit((void*)4);
}


void* thread3(void* arg){
  wait(1);
  wait(1);
  wait(1);
  printf("My brother should be done by now\n");
  pthread_exit((void*)3);
}


void* thread2(void* arg){
    pthread_t t3, t4;
    void *point1;
    void *point2;
    pthread_create(&t3,NULL,thread3,NULL);
    pthread_create(&t4,NULL,thread4,NULL);
    //printf("\nJoining new thread \n");
    pthread_join(t3, &point1);
    //printf("\nJoining finished \n");
    pthread_join(t4, &point2);
    //printf("back in business bb\n");
    printf("my boi thread3 told me to tell you %d\n", (int)point1);
    printf("my boi thread4 told me to tell you %d\n", (int)point2);
    //critical section

    // sem_post(&mutex);
    pthread_exit((void*)4);
}


void* thread(void* arg)
{
    //wait
    // sem_wait(&mutex);
    //printf("\nEntered..\n");
    pthread_t t2;
    void *point;
    pthread_create(&t2,NULL,thread2,NULL);
    //printf("\nJoining new thread \n");
    pthread_join(t2, &point);
    //printf("back in business bb\n");
    printf("my boi thread2 told me to tell you %d\n", (int)point);
    //critical section


    // sem_post(&mutex);
    pthread_exit((void*)4);
}



int main()
{
    // sem_init(&mutex, 0, 1);
    pthread_t t1;
    void *main_point;
    pthread_create(&t1,NULL,thread,NULL);
    pthread_join(t1, &main_point);
    //printf("Main thread returning\n");
    printf("my boi thread1 told me to tell you %d\n", (int)main_point);
    // pthread_join(t1,NULL);
    // pthread_join(t2,NULL);
    // sem_destroy(&mutex);
    pthread_exit((void*)1);
}
