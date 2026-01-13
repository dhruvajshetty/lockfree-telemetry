#ifndef TELEMETRY_EVENT_H
#define TELEMETRY_EVENT_H

#include <stdint.h>

typedef struct {
    uint64_t timestamp_ns;
    uint32_t producer_id;
    uint16_t type;
    uint16_t reserved;
    uint64_t value;
} telemetry_event_t;

#endif
