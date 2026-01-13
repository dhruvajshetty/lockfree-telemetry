#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdatomic.h>
#include <sched.h>

#include "ring_buffer.h"

#define ITERATIONS 1000000
static uint64_t *latencies;
static ring_buffer_t rb;
static atomic_bool done = false;

static uint64_t now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
static void pin_to_cpu(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);

    pthread_t thread = pthread_self();
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}

void *producer(void *arg) {
    pin_to_cpu(0);
    telemetry_event_t e = {
        .producer_id = 1,
        .type = 1,
        .value = 0
    };

    for (int i = 0; i < ITERATIONS; i++) {
        e.timestamp_ns = now_ns();
        e.value = i;

        while (!ring_buffer_push(&rb, &e)) {
            // busy-wait
        }
    }

    atomic_store(&done, true);
    return NULL;
}

void *consumer(void *arg) {
    pin_to_cpu(1);

    telemetry_event_t e;
    uint64_t latency_sum = 0;
    int count = 0;

    while (!atomic_load(&done) || count < ITERATIONS) {
        if (ring_buffer_pop(&rb, &e)) {
            uint64_t now = now_ns();
            uint64_t latency = now - e.timestamp_ns;

            latencies[count] = latency;
            latency_sum += latency;
            count++;
        }
    }

    uint64_t avg_latency = latency_sum / count;
    *(uint64_t *)arg = avg_latency;
    return NULL;
}

static void print_results(uint64_t latency_ns, double throughput) {
    printf("\n================ Benchmark Results ================\n");
    printf("Average Latency : %lu ns (%.2f µs)\n",
           latency_ns, latency_ns / 1000.0);
    printf("Throughput      : %.2f million events/sec\n",
           throughput / 1e6);
    printf("===================================================\n\n");
}
static int cmp_u64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a;
    uint64_t y = *(const uint64_t *)b;
    return (x > y) - (x < y);
}

int main() {
    pthread_t prod, cons;
    uint64_t avg_latency = 0;

    latencies = malloc(sizeof(uint64_t) * ITERATIONS);
    if (!latencies) {
        perror("malloc");
        return 1;
    }

    uint64_t start = now_ns();

    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, &avg_latency);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    uint64_t end = now_ns();

    qsort(latencies, ITERATIONS, sizeof(uint64_t), cmp_u64);

    uint64_t p99 = latencies[(int)(ITERATIONS * 0.99)];
    uint64_t max = latencies[ITERATIONS - 1];

    double seconds = (end - start) / 1e9;
    double throughput = ITERATIONS / seconds;

    printf("\n================ Benchmark Results ================\n");
    printf("Average Latency : %lu ns (%.2f µs)\n",
           avg_latency, avg_latency / 1000.0);
    printf("P99 Latency     : %lu ns (%.2f µs)\n",
           p99, p99 / 1000.0);
    printf("Max Latency     : %lu ns (%.2f µs)\n",
           max, max / 1000.0);
    printf("Throughput      : %.2f million events/sec\n",
           throughput / 1e6);
    printf("===================================================\n\n");

    free(latencies);
    return 0;
}
