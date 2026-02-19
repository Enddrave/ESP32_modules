#pragma once
#include "device_types.h"

void storage_save_state(device_state_t state);
device_state_t storage_load_state(void);
void storage_init(void); 