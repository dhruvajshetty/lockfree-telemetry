#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "telemetry_event.h"

#define RING_SIZE 1024  // must be power of 2

typedef struct {
    atomic_size_t head;
    atomic_size_t tail;
    telemetry_event_t buffer[RING_SIZE];
} ring_buffer_t;

bool ring_buffer_push(ring_buffer_t *rb, const telemetry_event_t *event);
bool ring_buffer_pop(ring_buffer_t *rb, telemetry_event_t *event);

#endif
