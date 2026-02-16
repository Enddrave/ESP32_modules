#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "storage.h"    
#include "esp_log.h"

static const char *TAG = "smart_device";
device_state_t device_state;



int boot_count =0;


void app_main(void)
{
    storage_init();
    storage_load(&device_state);

    device_state.boot_count++;
    storage_save(&device_state);

    ESP_LOGI(TAG, "Device booted %lu times", device_state.boot_count);
    ESP_LOGI(TAG, "Current mode: %d", device_state.mode);
    ESP_LOGI(TAG, "Last runtime: %lu seconds", device_state.last_runtime);
    ESP_LOGI(TAG, "Brightness: %d", device_state.brightness); 

    while(1){
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "Device running...");
    }
}
