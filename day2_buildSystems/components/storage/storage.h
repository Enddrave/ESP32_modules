#pragma once
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define CONFIG_VERSION 2

typedef struct {
    uint32_t version;
    char name[32];
    uint32_t mode;
    uint32_t boot_count;
    uint32_t crc;
    
} device_config_t;

bool storage_init(void);
bool storage_load(device_config_t *cfg);
bool storage_save( device_config_t *cfg);
void storage_factory_reset(device_config_t *cfg);
