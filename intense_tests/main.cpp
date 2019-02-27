#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>


// #define CATCH_CONFIG_MAIN
// #include "catch.hpp"

#include "queue.hpp"

/*
	write into a queue of size 1, 1000 times, should be fine right? :) 
*/
struct StringPusherArg {
	Queue *queue;
	char *string;
};

void *thread_string_pusher(void *arg) {
	StringPusherArg *unpacked = (StringPusherArg *)arg;

	for (int i = 1; i < 100; ++i) {
		char *message = strdup(unpacked->string);
		queue_put(unpacked->queue, &message);
	}

	return NULL;
}

void* ay_there(void* arg){
	printf("AY BB\n");
	sleep(1);
}

void *reader_thread(void *arg) {
	Queue *q = (Queue *)arg;
	for (int i = 1; i < 100; ++i) {
		char *tmp;
		queue_get(q, &tmp); // we ignore the output, but we expect to get it 100 times
	}

	int *retval = (int *)malloc(sizeof(int));
	*retval = 100;
	return (void *)retval;
}
int main(){
		Queue *queue = (Queue *)malloc(sizeof(Queue));
	queue_init(queue, sizeof(char *), 10000);

	// spawn worker threads
	std::vector<pthread_t> threads;
	for (int i = 0; i < 10; ++i) {
		fprintf(stdout, "spawned thread no %d\n", i);

		StringPusherArg *arg = (StringPusherArg *)malloc(sizeof(StringPusherArg));
		arg->queue = queue;
		arg->string = (char *)malloc(sizeof(char) * 1000);
		sprintf(arg->string, "thread %d pushed this :)\n", i);
		pthread_t thread_id; 
		pthread_create(&thread_id, NULL, thread_string_pusher, (void *)arg);
		threads.push_back(thread_id);
	}

	// spawn reader threads
	std::vector<pthread_t> reader_threads;
	for (int i = 0; i < 10; ++i) {
		pthread_t thread_id; 
		pthread_create(&thread_id, NULL, reader_thread, (void *)queue);
		reader_threads.push_back(thread_id);
	}

	// REQUIRE(threads.size() == 10);
	// REQUIRE(reader_threads.size() == 10);

	// test pthread join
	for (int i = 0; i < threads.size(); ++i) {
		pthread_join(threads[i], NULL);
	}

	// int times = 0;
	for (int i = 0; i < threads.size(); ++i) {
		pthread_join(reader_threads[i], NULL);
	}
}



// TEST_CASE( "write into a queue of size 1, 1000 times", "[q1x1000]" ) {
// 	Queue queue;
// 	printf("doing queue init\n");
// 	queue_init(&queue, sizeof(char *), 1);
// 	printf("doing threads\n");
// 	// spawn worker threads
// 	std::vector<pthread_t> threads;
// 	for (int i = 0; i < 10; ++i) {
// 		printf("in here\n");
// 		StringPusherArg *arg = (StringPusherArg *)malloc(sizeof(StringPusherArg));
// 		arg->queue = &queue;
// 		arg->string = (char *)malloc(sizeof(char) * 1000);
// 		sprintf(arg->string, "thread %d pushed this :)\n", i);
// 		pthread_t thread_id; 
// 		printf("trying to get pthread create to work\n");
// 		pthread_create(&thread_id, NULL, thread_string_pusher, (void *)arg);
// 		threads.push_back(thread_id);
// 		printf("pthread created\n");

// 	}

// 	printf("out of loop to create threads\n");
// 	int times = 0;
// 	while (times < 1000) {
// 		char *message;
// 		queue_get(&queue, (void *)&message);
// 		// fprintf(stdout, "QUEUE SAYS: %s\n", message);
// 		times++;
// 	}

// 	printf("starting pthread join\n");
// 	printf("threads size is: %d\n", threads.size());
// 	// test pthread join
// 	for (int i = 0; i < threads.size(); ++i) {
// 		printf("joining with %d!\n", threads[i]);
// 		pthread_join(threads[i], NULL);
// 		printf(" pthread join works\n");
// 	}

// 	REQUIRE(times == 1000);

// 	queue_free(&queue);
// }

// TEST_CASE( "write into a queue of size 10, 1000 times", "[q1x1000]" ) {
// 	Queue queue;
// 	queue_init(&queue, sizeof(char *), 10);

// 	// spawn worker threads
// 	std::vector<pthread_t> threads;
// 	for (int i = 0; i < 10; ++i) {
// 		StringPusherArg *arg = (StringPusherArg *)malloc(sizeof(StringPusherArg));
// 		arg->queue = &queue;
// 		arg->string = (char *)malloc(sizeof(char) * 1000);
// 		sprintf(arg->string, "thread %d pushed this :)\n", i);
// 		pthread_t thread_id; 
// 		pthread_create(&thread_id, NULL, thread_string_pusher, (void *)arg);
// 		threads.push_back(thread_id);
// 	}

// 	int times = 0;
// 	while (times < 1000) {
// 		char *message;
// 		queue_get(&queue, (void *)&message);
// 		// fprintf(stdout, "QUEUE SAYS: %s\n", message);
// 		times++;
// 	}

// 	// test pthread join
// 	for (int i = 0; i < threads.size(); ++i) {
// 		pthread_join(threads[i], NULL);
// 	}

// 	REQUIRE(times == 1000);

// 	queue_free(&queue);
// }

// TEST_CASE( "write into a queue of size 10000, 1000 times", "[q1x1000]" ) {
// 	Queue queue;
// 	queue_init(&queue, sizeof(char *), 10000);

// 	// spawn worker threads
// 	std::vector<pthread_t> threads;
// 	for (int i = 0; i < 10; ++i) {
// 		StringPusherArg *arg = (StringPusherArg *)malloc(sizeof(StringPusherArg));
// 		arg->queue = &queue;
// 		arg->string = (char *)malloc(sizeof(char) * 1000);
// 		sprintf(arg->string, "thread %d pushed this :)\n", i);
// 		pthread_t thread_id; 
// 		pthread_create(&thread_id, NULL, thread_string_pusher, (void *)arg);
// 		threads.push_back(thread_id);
// 	}

// 	int times = 0;
// 	while (times < 1000) {
// 		char *message;
// 		queue_get(&queue, (void *)&message);
// 		// fprintf(stdout, "QUEUE SAYS: %s\n", message);
// 		times++;
// 	}

// 	// test pthread join
// 	for (int i = 0; i < threads.size(); ++i) {
// 		pthread_join(threads[i], NULL);
// 	}

// 	REQUIRE(times == 1000);

// 	queue_free(&queue);
// }

// TEST_CASE( "write into a queue of size 10, 1000 times with sleep(1) to ensure saturation", "[q1x1000]" ) {
// 	Queue queue;
// 	queue_init(&queue, sizeof(char *), 10000);

// 	// spawn worker threads
// 	std::vector<pthread_t> threads;
// 	for (int i = 0; i < 10; ++i) {
// 		StringPusherArg *arg = (StringPusherArg *)malloc(sizeof(StringPusherArg));
// 		arg->queue = &queue;
// 		arg->string = (char *)malloc(sizeof(char) * 1000);
// 		sprintf(arg->string, "thread %d pushed this :)\n", i);
// 		pthread_t thread_id; 
// 		pthread_create(&thread_id, NULL, thread_string_pusher, (void *)arg);
// 		threads.push_back(thread_id);
// 	}

// 	sleep(1); // zzzzzzz

// 	int times = 0;
// 	while (times < 1000) {
// 		char *message;
// 		queue_get(&queue, (void *)&message);
// 		// fprintf(stdout, "QUEUE SAYS: %s\n", message);
// 		times++;
// 	}

// 	// test pthread join
// 	for (int i = 0; i < threads.size(); ++i) {
// 		pthread_join(threads[i], NULL);
// 	}

// 	REQUIRE(times == 1000);

// 	queue_free(&queue);
// }

// TEST_CASE( "write into a queue of size 10, 1000 times with 10 readers", "[q1x1000]" ) {
// 	Queue *queue = (Queue *)malloc(sizeof(Queue));
// 	queue_init(queue, sizeof(char *), 10000);

// 	// spawn worker threads
// 	std::vector<pthread_t> threads;
// 	for (int i = 0; i < 10; ++i) {
// 		fprintf(stdout, "spawned thread no %d\n", i);

// 		StringPusherArg *arg = (StringPusherArg *)malloc(sizeof(StringPusherArg));
// 		arg->queue = queue;
// 		arg->string = (char *)malloc(sizeof(char) * 1000);
// 		sprintf(arg->string, "thread %d pushed this :)\n", i);
// 		pthread_t thread_id; 
// 		pthread_create(&thread_id, NULL, thread_string_pusher, (void *)arg);
// 		threads.push_back(thread_id);
// 	}

// 	// spawn reader threads
// 	std::vector<pthread_t> reader_threads;
// 	for (int i = 0; i < 10; ++i) {
// 		pthread_t thread_id; 
// 		pthread_create(&thread_id, NULL, reader_thread, (void *)queue);
// 		reader_threads.push_back(thread_id);
// 	}

// 	REQUIRE(threads.size() == 10);
// 	REQUIRE(reader_threads.size() == 10);

// 	// test pthread join
// 	for (int i = 0; i < threads.size(); ++i) {
// 		pthread_join(threads[i], NULL);
// 	}

// 	// int times = 0;
// 	for (int i = 0; i < threads.size(); ++i) {
// 		pthread_join(reader_threads[i], NULL);
// 	}
// }
