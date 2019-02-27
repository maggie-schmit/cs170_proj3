#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <cstdlib>
// #include <mutex>
#include <vector>

struct Queue {
	int head;
	int tail;
	int size;
	int size_bytes;
	int elem_size;
	sem_t space;
	sem_t items;
	sem_t mutex;
	void *elements;
};

typedef struct Queue Queue;

/*
	Shared Circular Queue Methods
*/
int queue_init(Queue *queue, int elem_size, int size) {
	queue->head = queue->tail = 0;
	queue->size = size;
	queue->elem_size = elem_size;
	queue->size_bytes = size * elem_size;

	sem_init(&(queue->space), 0, size);
	sem_init(&(queue->items), 0, 0);
	sem_init(&(queue->mutex), 0, 1);

	queue->elements = malloc(elem_size * size);
	if (queue->elements == NULL)
		return -1;
	return 0;
}

void queue_free(Queue *queue) {
	sem_destroy(&(queue->space));
	sem_destroy(&(queue->items));
	sem_destroy(&(queue->mutex));
	free(queue->elements);
}

void queue_put(Queue *queue, void *value) {
	sem_wait(&(queue->space));
	sem_wait(&(queue->mutex));
	memcpy(queue->elements + queue->tail, value, queue->elem_size);
	queue->tail += queue->elem_size;
	if (queue->tail >= queue->size_bytes)
		queue->tail = 0;
	sem_post(&(queue->mutex));
	sem_post(&(queue->items));
}

void queue_get(Queue *queue, void *result) {
	sem_wait(&(queue->items));
	sem_wait(&(queue->mutex));
	memcpy(result, queue->elements + queue->head, queue->elem_size);
	queue->head += queue->elem_size;
	if (queue->head >= queue->size_bytes)
		queue->head = 0;
	sem_post(&(queue->mutex));
	sem_post(&(queue->space));
}


struct StringPusherArg {
	Queue *queue;
	char *string;
};

void *thread_string_pusher(void *arg) {
	StringPusherArg *unpacked = (StringPusherArg *)arg;

	for (int i = 0; i < 2; ++i) {
		fprintf(stdout, "pushing my string...\n");
		queue_put(unpacked->queue, strdup(unpacked->string));
	}

	return NULL;
}


int main() 
{

	Queue queue;
	queue_init(&queue, sizeof(char *), 10);

	// spawn worker threads
	for (int i = 0; i < 2; ++i) {
		fprintf(stdout, "spawned thread no %d\n", i);

		StringPusherArg *arg = (StringPusherArg *)malloc(sizeof(StringPusherArg));
		arg->queue = &queue;
		arg->string = (char *)malloc(sizeof(char) * 1000);
		sprintf(arg->string, "thread %d pushed this :)\n", i);
		pthread_t thread_id; 
		pthread_create(&thread_id, NULL, thread_string_pusher, (void *)arg);
	}


	// TODO: test joining threads :P 
	while (1) {
		fprintf(stdout, "TRYING TO GET MESSAGE FROM THE QUEUE!");
		char *message;
		queue_get(&queue, (void *)&message);
		fprintf(stdout, "GOT FROM QUEUE: %s\n", message);
	}

    return 0; 
} 
