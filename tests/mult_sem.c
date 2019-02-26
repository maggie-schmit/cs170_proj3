#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#define HAMBURGER 1

void force_sleep(int seconds) {
	struct timespec initial_spec, remainder_spec;
	initial_spec.tv_sec = (time_t)seconds;
	initial_spec.tv_nsec = 0;

	int err = -1;
	while(err == -1) {
		err = nanosleep(&initial_spec,&remainder_spec);
		initial_spec = remainder_spec;
		memset(&remainder_spec,0,sizeof(remainder_spec));
	}
}

sem_t my_sem;
pthread_t thread_1;
pthread_t thread_2;
pthread_t thread_3;
pthread_t thread_4;
pthread_t thread_5;

void * ship_mates(void *args){
  printf("AHOY THERE MATEYS!\n");
}

void * bbq_party(void *args) {

	sem_wait(&my_sem);
  int r3 = 0;
	printf("Thread %u has the lock\n",(unsigned)pthread_self());
  pthread_create(&thread_3, NULL, ship_mates, NULL);
  pthread_join(thread_3, (void**)&r3);
  printf("r3: %d\n", r3);
	sem_post(&my_sem);
	return (void*)HAMBURGER;
}

int main() {

	sem_init(&my_sem,0,2);

	int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;

	pthread_create(&thread_1, NULL, bbq_party, NULL);
  pthread_create(&thread_2, NULL, bbq_party, NULL);
  pthread_create(&thread_3, NULL, bbq_party, NULL);
  pthread_create(&thread_4, NULL, bbq_party, NULL);
  pthread_create(&thread_5, NULL, bbq_party, NULL);

	pthread_join(thread_1, (void**)&r1);
  sleep(2);
  pthread_join(thread_5, (void**)&r5);
  printf("r1: %d\n", r1);
  printf("r5: %d\n", r5);

	sem_destroy(&my_sem);

	return 0;
}
