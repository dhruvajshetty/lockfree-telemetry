#include "mutex_ring_buffer.h"

void mutex_ring_init(mutex_ring_buffer_t *rb) {
    rb->head = rb->tail = 0;
    pthread_mutex_init(&rb->lock, NULL);
}

bool mutex_ring_push(mutex_ring_buffer_t *rb, const telemetry_event_t *event) {
    pthread_mutex_lock(&rb->lock);

    size_t next = (rb->head + 1) % MUTEX_RING_SIZE;
    if (next == rb->tail) {
        pthread_mutex_unlock(&rb->lock);
        return false;
    }

    rb->buffer[rb->head] = *event;
    rb->head = next;

    pthread_mutex_unlock(&rb->lock);
    return true;
}

bool mutex_ring_pop(mutex_ring_buffer_t *rb, telemetry_event_t *event) {
    pthread_mutex_lock(&rb->lock);

    if (rb->tail == rb->head) {
        pthread_mutex_unlock(&rb->lock);
        return false;
    }

    *event = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % MUTEX_RING_SIZE;

    pthread_mutex_unlock(&rb->lock);
    return true;
}
