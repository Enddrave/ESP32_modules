#include "storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"


static const char *TAG = "storage";

static nvs_handle_t s_nvs;

#define NAMESPACE "devcfg"
#define KEY_STATE "state"

void storage_init(void){
    esp_err_t err =nvs_flash_init();

    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
        nvs_flash_erase();
        nvs_flash_init();
    }

    ESP_ERROR_CHECK(nvs_open(NAMESPACE, NVS_READWRITE, &s_nvs));
    ESP_LOGI(TAG, "Storage initialized");
}

void storage_load(device_state_t *state){
    size_t required_size = sizeof(device_state_t);
    esp_err_t err = nvs_get_blob(s_nvs, KEY_STATE, state, &required_size);

    if(err != ESP_OK){
        ESP_LOGW(TAG, "No saved state found, printing defaults");
        state->mode = 0; 
        state->last_runtime = 0;
        state->brightness = 100;
        state->boot_count = 0;
    } 
}

void storage_save(device_state_t *state){
nvs_set_blob(s_nvs, KEY_STATE, state, sizeof(device_state_t));
nvs_commit(s_nvs);
}   

