#pragma once
#include <stdio.h>

typedef struct {
    uint8_t mode;
    uint32_t last_runtime;
    uint8_t brightness;
    uint32_t boot_count;
} device_state_t;

void storage_init(void);
void storage_load(device_state_t *state);
void storage_save(device_state_t *state);

