#include <pthread.h>
#include <semaphore.h>
#include <cstdlib>

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
