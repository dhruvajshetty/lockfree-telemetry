#ifndef MUTEX_RING_BUFFER_H
#define MUTEX_RING_BUFFER_H

#include <pthread.h>
#include <stdbool.h>
#include "telemetry_event.h"

#define MUTEX_RING_SIZE 1024

typedef struct {
    telemetry_event_t buffer[MUTEX_RING_SIZE];
    size_t head;
    size_t tail;
    pthread_mutex_t lock;
} mutex_ring_buffer_t;

void mutex_ring_init(mutex_ring_buffer_t *rb);
bool mutex_ring_push(mutex_ring_buffer_t *rb, const telemetry_event_t *event);
bool mutex_ring_pop(mutex_ring_buffer_t *rb, telemetry_event_t *event);

#endif
