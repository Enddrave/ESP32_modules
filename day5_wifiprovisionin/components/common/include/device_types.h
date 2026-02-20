#pragma once

typedef enum
{
    DEVICE_STATE_UNCONFIGURED = 0,   // No WiFi credentials exist
    DEVICE_STATE_CONNECTING,         // Trying STA connection
    DEVICE_STATE_ONLINE,             // Got IP
    DEVICE_STATE_RECOVERY            // Failed connection → fallback AP
} device_state_t;