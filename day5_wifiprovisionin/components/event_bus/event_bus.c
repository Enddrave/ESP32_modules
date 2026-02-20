#include "event_bus.h"
#include "esp_log.h"

static const char *TAG = "EVENT_BUS";

/* define the event base */
ESP_EVENT_DEFINE_BASE(APP_EVENTS);


/* initialize default loop */
void event_bus_init(void)
{
    static bool initialized = false;

    if (initialized)
        return;

    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG, "Event loop create failed: %s", esp_err_to_name(err));
        return;
    }

    initialized = true;
    ESP_LOGI(TAG, "Event bus ready");
}


/* publish event */
esp_err_t event_bus_post(app_event_id_t id, void *data, size_t len)
{
    return esp_event_post(APP_EVENTS, id, data, len, portMAX_DELAY);
}


/* subscribe to ALL events */
esp_err_t event_bus_subscribe(esp_event_handler_t handler, void *arg)
{
    return esp_event_handler_register(APP_EVENTS, ESP_EVENT_ANY_ID, handler, arg);
}