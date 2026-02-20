#pragma once
#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Central event base */
ESP_EVENT_DECLARE_BASE(APP_EVENTS);

/* All system level events */
typedef enum
{
    APP_EVENT_BOOT = 0,

    /* provisioning */
    APP_EVENT_PROV_STARTED,
    APP_EVENT_PROV_CREDENTIALS_RECEIVED,
    APP_EVENT_PROV_SUCCESS,
    APP_EVENT_PROV_TIMEOUT,

    /* wifi */
    APP_EVENT_WIFI_CONNECTED,
    APP_EVENT_WIFI_DISCONNECTED,
    APP_EVENT_IP_ACQUIRED,

    /* health */
    APP_EVENT_INTERNET_OK,
    APP_EVENT_INTERNET_LOST,

    /* failures */
    APP_EVENT_ERROR

} app_event_id_t;


/* initialize system event loop */
void event_bus_init(void);

/* post event (any module can call) */
esp_err_t event_bus_post(app_event_id_t id, void *data, size_t len);

/* subscribe to events */
esp_err_t event_bus_subscribe(esp_event_handler_t handler, void *arg);

#ifdef __cplusplus
}
#endif