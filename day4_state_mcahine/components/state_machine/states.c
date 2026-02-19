#include "states.h"
#include "esp_log.h"


static const char *TAG = "StateMachine";

void state_factory_enter(void)
{
    ESP_LOGI(TAG, "Entering Factory State");
    // Add factory state specific initialization here
}

void state_provisioning_enter(void)
{
    ESP_LOGI(TAG, "Entering Provisioning State");
    // Add provisioning state specific initialization here
}

void state_normal_enter(void)
{
    ESP_LOGI(TAG, "Entering Normal State");
    // Add normal state specific initialization here
}

void state_recovery_enter(void)
{
    ESP_LOGI(TAG, "Entering Recovery State");
    // Add recovery state specific initialization here
}   