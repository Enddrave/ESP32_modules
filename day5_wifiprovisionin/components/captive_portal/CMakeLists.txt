idf_component_register(
    SRCS
        "captive_portal.c"
        "dns_server.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi esp_http_server esp_netif esp_event nvs_flash lwip
)