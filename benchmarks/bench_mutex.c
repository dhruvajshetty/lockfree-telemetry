#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sched.h>

#include "mutex_ring_buffer.h"

#define ITERATIONS 1000000

static mutex_ring_buffer_t rb;
static atomic_bool done = false;
static uint64_t *latencies;

static uint64_t now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static void pin_to_cpu(int cpu) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
}

static int cmp_u64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a;
    uint64_t y = *(const uint64_t *)b;
    return (x > y) - (x < y);
}

void *producer(void *arg) {
    pin_to_cpu(0);

    telemetry_event_t e = { .producer_id = 1, .type = 1 };

    for (int i = 0; i < ITERATIONS; i++) {
        e.timestamp_ns = now_ns();
        e.value = i;
        while (!mutex_ring_push(&rb, &e)) {}
    }

    atomic_store(&done, true);
    return NULL;
}

void *consumer(void *arg) {
    pin_to_cpu(1);

    telemetry_event_t e;
    int count = 0;

    while (!atomic_load(&done) || count < ITERATIONS) {
        if (mutex_ring_pop(&rb, &e)) {
            latencies[count++] = now_ns() - e.timestamp_ns;
        }
    }
    return NULL;
}

int main() {
    pthread_t p, c;
    latencies = malloc(sizeof(uint64_t) * ITERATIONS);
    mutex_ring_init(&rb);

    uint64_t start = now_ns();

    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);

    uint64_t end = now_ns();

    qsort(latencies, ITERATIONS, sizeof(uint64_t), cmp_u64);

    uint64_t avg = 0;
    for (int i = 0; i < ITERATIONS; i++) avg += latencies[i];
    avg /= ITERATIONS;

    printf("\n================ MUTEX RESULTS =================\n");
    printf("Average Latency : %lu ns\n", avg);
    printf("P99 Latency     : %lu ns\n", latencies[(int)(ITERATIONS * 0.99)]);
    printf("Max Latency     : %lu ns\n", latencies[ITERATIONS - 1]);
    printf("Throughput      : %.2f M events/sec\n",
           ITERATIONS / ((end - start) / 1e9));
    printf("===============================================\n\n");

    free(latencies);
    return 0;
}
