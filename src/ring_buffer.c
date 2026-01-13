#include "ring_buffer.h"

bool ring_buffer_push(ring_buffer_t *rb, const telemetry_event_t *event) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t next = (head + 1) & (RING_SIZE - 1);

    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    if (next == tail) {
        return false; // buffer full
    }

    rb->buffer[head] = *event;

    atomic_store_explicit(&rb->head, next, memory_order_release);
    return true;
}

bool ring_buffer_pop(ring_buffer_t *rb, telemetry_event_t *event) {
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);

    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    if (tail == head) {
        return false; // buffer empty
    }

    *event = rb->buffer[tail];

    atomic_store_explicit(&rb->tail,
        (tail + 1) & (RING_SIZE - 1),
        memory_order_release);

    return true;
}
