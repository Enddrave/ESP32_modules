#include "diag.h"
#include "esp_system.h"
#include "esp_log.h"

void diag_print_reset_reason(void)
{
    ESP_LOGI("DIAG", "Reset reason: %d", esp_reset_reason());
}