#include "storage.h"
#include "esp_log.h"    
#include "nvs_flash.h"
#include "nvs.h"        

static const char *TAG = "Storage";

void storage_init(void)
{
    nvs_flash_init();
}

void storage_save_state(device_state_t state)
{
    nvs_handle_t nvs;
    nvs_open("sys", NVS_READWRITE, &nvs);
    nvs_set_i32(nvs, "device_state", state);
    nvs_commit(nvs);
    nvs_close(nvs);
    ESP_LOGI(TAG, "Saved state: %d", state);
}

device_state_t storage_load_state(void)
{
    nvs_handle_t nvs;
    int32_t value = DEVICE_STATE_FACTORY;   // default state

    if (nvs_open("sys", NVS_READONLY, &nvs) == ESP_OK)
    {
        if (nvs_get_i32(nvs, "device_state", &value) == ESP_OK)
        {
            ESP_LOGI(TAG, "Loaded state: %lu", value);
        }
        else
        {
            ESP_LOGW(TAG, "No state found, using FACTORY");
        }

        nvs_close(nvs);
    }
    else
    {
        ESP_LOGW(TAG, "NVS open failed, using FACTORY");
    }

    return (device_state_t)value;
}