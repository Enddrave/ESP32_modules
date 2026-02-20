#include "state_machine.h"
#include "storage.h"
#include "wifi_manager.h"
#include "esp_log.h"

static const char *TAG = "SM";
static device_state_t current;

void sm_init(void)
{
    storage_init();
    wifi_manager_init();

    if (!storage_has_wifi_credentials())
    {
        current = DEVICE_STATE_UNCONFIGURED;
        ESP_LOGW(TAG, "No credentials → AP mode");
        wifi_manager_start_softap();
    }
    else
    {
        current = DEVICE_STATE_CONNECTING;
        ESP_LOGI(TAG, "Credentials found → connect STA");
        wifi_manager_connect_sta();
    }
}

void sm_process(void)
{
    switch(current)
    {
        case DEVICE_STATE_UNCONFIGURED:
            ESP_LOGI(TAG, "Waiting for user provisioning");
            break;

        case DEVICE_STATE_CONNECTING:
            ESP_LOGI(TAG, "Connecting...");
            break;

        case DEVICE_STATE_ONLINE:
            ESP_LOGI(TAG, "Online");
            break;

        case DEVICE_STATE_RECOVERY:
            ESP_LOGW(TAG, "Recovery AP");
            wifi_manager_start_softap();
            break;
    }
}