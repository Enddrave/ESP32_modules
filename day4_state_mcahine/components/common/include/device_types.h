#pragma once

typedef enum
{
    DEVICE_STATE_FACTORY = 0,
    DEVICE_STATE_PROVISIONING,
    DEVICE_STATE_NORMAL,
    DEVICE_STATE_RECOVERY
} device_state_t;