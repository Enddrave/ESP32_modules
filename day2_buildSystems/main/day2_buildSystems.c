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
#ifdef APP_DEBUG_BUILD
   // ESP_LOGI(TAG, "DEBUG BUILD");
#endif
#ifdef APP_RELEASE_BUILD
    //ESP_LOGW(TAG, "RELEASE BUILD");
#endif
void app_main(void)
{
    storage_init();
    storage_load(&g_cfg);

    g_cfg.boot_count++;
    storage_save(&g_cfg);

    ESP_LOGI(TAG, "Name: %s", g_cfg.name);
    ESP_LOGE(TAG, "Version: %lu", g_cfg.version);
    ESP_LOGW(TAG, "Mode: %ld", g_cfg.mode);
    ESP_LOGE(TAG, "Boot count: %ld", g_cfg.boot_count);
    ESP_LOGI(TAG, "CRC: %lu", g_cfg.crc);
}
