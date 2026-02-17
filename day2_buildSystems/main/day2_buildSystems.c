/*Project: Self-Healing Smart Device Core- self_recovering_device

Device internally simulates various failure scenarios and demonstrates the self-healing capabilities of the system:
___________________________________________________________________
Feature	                  |     Without hardware
-------------------------------------------------------------------
Device configuration	  |     struct stored in NVS
User change settings	  |     serial command
Power cut	              |     forced reset
Corruption	              |     manual flash corruption command
Factory reset	          |     auto detect CRC fail
Build proof	              |     binary hash check
--------------------------------------------------------------------
*/

#include <stdio.h>
#include "storage.h"
#include "esp_log.h"

static const char *TAG = "main";

device_config_t g_cfg;


void app_main(void)
{
    storage_init();
    storage_load(&g_cfg);

    g_cfg.boot_count++;
    storage_save(&g_cfg);

    ESP_LOGI(TAG, "Name: %s", g_cfg.name);
    ESP_LOGI(TAG, "Mode: %ld", g_cfg.mode);
    ESP_LOGI(TAG, "Boot count: %ld", g_cfg.boot_count);
}
