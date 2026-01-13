#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ring_buffer.h"

static uint64_t now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main() {
    ring_buffer_t rb = {0};
    telemetry_event_t e = {
        .timestamp_ns = now_ns(),
        .producer_id = 1,
        .type = 42,
        .value = 12345
    };

    if (!ring_buffer_push(&rb, &e)) {
        printf("push failed\n");
        return 1;
    }

    telemetry_event_t out;
    if (!ring_buffer_pop(&rb, &out)) {
        printf("pop failed\n");
        return 1;
    }

    printf("event value = %lu\n", out.value);
    return 0;
}
