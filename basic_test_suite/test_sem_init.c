#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <semaphore.h>
struct timeval tv;

#define wait(i) ({tv.tv_sec = i;tv.tv_usec=500000;select(0,NULL,NULL,NULL,&tv);})

#define enjoy_party wait(1)
#define cleanup_party wait(2)

unsigned int thread_1_done = 0;
pthread_t thread;

void * bbq_party(void *args) {
	printf("at the party\n");
	sem_t test;
	int x = sem_init(&test,0,1);
	printf("Its id is %d\n", ((__sem_t*)(&test)->__align)->id);
	printf("Its value is %d\n", ((__sem_t*)(&test)->__align)->value);
	return NULL;
}

int main() {

	printf("Inviting friends to the party!\n");

	for (int i = 0; i < 5; i++){
		pthread_create(&thread, NULL, bbq_party, NULL);
	}
	pthread_exit(0);
}
