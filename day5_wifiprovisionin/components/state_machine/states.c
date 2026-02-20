#include "states.h"
#include "wifi_manager.h"
#include "esp_log.h"

static const char *TAG = "STATE";

/* Device just manufactured or reset */
void state_factory_enter(void)
{
    ESP_LOGI(TAG, "FACTORY: starting provisioning");
    wifi_manager_start_softap();
}

/* Phone connected to AP */
void state_provisioning_enter(void)
{
    ESP_LOGI(TAG, "PROVISIONING: waiting credentials");
}

/* Got SSID/password */
void state_connecting_enter(void)
{
    ESP_LOGI(TAG, "CONNECTING: joining router");
    wifi_manager_connect_sta();
}

/* Connected to router */
void state_online_enter(void)
{
    ESP_LOGI(TAG, "ONLINE: device operational");
}

/* Router lost */
void state_offline_enter(void)
{
    ESP_LOGW(TAG, "OFFLINE: reconnecting");
}

/* Something broken */
void state_recovery_enter(void)
{
    ESP_LOGE(TAG, "RECOVERY MODE");
    wifi_manager_start_softap();
}