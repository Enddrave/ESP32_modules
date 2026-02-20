#include "storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "STORAGE";

void storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
}

bool storage_has_wifi_credentials(void)
{
    nvs_handle_t nvs;

    if (nvs_open("wifi", NVS_READONLY, &nvs) != ESP_OK)
        return false;

    size_t len = 0;
    esp_err_t err = nvs_get_str(nvs, "ssid", NULL, &len);
    nvs_close(nvs);

    if (err == ESP_OK && len > 1)
    {
        ESP_LOGI(TAG, "WiFi credentials found");
        return true;
    }

    ESP_LOGW(TAG, "No WiFi credentials stored");
    return false;
}