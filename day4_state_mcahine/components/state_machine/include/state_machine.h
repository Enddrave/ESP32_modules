#pragma once
#include "device_types.h"

void state_machine_start(void);



void sm_init(void);
void sm_set_state(device_state_t state);
device_state_t sm_get_state(void);
void sm_run(void);

