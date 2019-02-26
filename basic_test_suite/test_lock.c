#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include "lock.h"
struct timeval tv;

#define wait(i) ({tv.tv_sec = i;tv.tv_usec=500000;select(0,NULL,NULL,NULL,&tv);})

#define enjoy_party wait(1)
#define cleanup_party wait(2)


unsigned int thread_1_done = 0;
pthread_t thread;

void * bbq_party(void *args) {
	lock();
	printf("Locked. About to go to sleep...\n");
	printf("I am friend number %u...\n", pthread_self());
	wait(1);
	printf("I'm awake now! Unlocking.\n");
	unlock();
	thread_1_done++;
	return NULL;
}

int main() {

	printf("Inviting friends to the party!\n");

	for (int i = 0; i < 5; i++){
		pthread_create(&thread, NULL, bbq_party, NULL);
	}
	pthread_exit(0);
}
