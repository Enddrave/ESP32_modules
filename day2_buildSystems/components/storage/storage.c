#include "storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>


static const char *TAG = "storage";
static nvs_handle_t s_nvs;


/* ------- CRC FUNCTION ------- */
/* TODO: Replace with real CRC32 later */

static uint32_t simple_crc32(uint8_t *data, size_t len)
{
    uint32_t crc =0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
    }
    return crc; 
}   

/*----INIT----*/
bool storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    return nvs_open("devcfg", NVS_READWRITE, &s_nvs) == ESP_OK;
}

/*---- factory defaults----*/
void storage_factory_reset(device_config_t *cfg)
{
    
    cfg->version = CONFIG_VERSION;
    strcpy(cfg->name, "default_device");
    cfg->mode = 0; // default mode
    cfg->boot_count = 0;
}

/* --- save ---- */
bool storage_save( device_config_t *cfg)
{

        cfg->crc = simple_crc32((uint8_t*)cfg, sizeof(device_config_t)-4);

        nvs_set_blob(s_nvs, "cfg", cfg, sizeof(device_config_t));
        nvs_commit(s_nvs);

        ESP_LOGI(TAG, "Saved config (CRC=%lu)", cfg->crc);

        device_config_t bad = *cfg;
bad.crc ^= 0x55AA1234;   // flip bits
nvs_set_blob(s_nvs, "cfg", &bad, sizeof(bad));
nvs_commit(s_nvs);

        return true;

}

/* --- load --- */
bool storage_load(device_config_t *cfg)
{
    size_t size = sizeof(device_config_t);

    if (nvs_get_blob(s_nvs, "cfg", cfg, &size) != ESP_OK) 
    {
        ESP_LOGW(TAG, "No config found");
        storage_factory_reset(cfg);
        return false;
    }

    // version check
    if (cfg->version != CONFIG_VERSION) {
        ESP_LOGW(TAG, "Config version mismatch");
        storage_factory_reset(cfg);
        return false;           
    }

    // CRC check
    uint32_t calc = simple_crc32((uint8_t*)cfg, sizeof(device_config_t)-4);
    if (calc != cfg->crc) {
        ESP_LOGW(TAG, "CRC Failed! Data corrupeted");
        storage_factory_reset(cfg);
        return false;
    }

    ESP_LOGI(TAG,"Config Ok");
    return true;
}