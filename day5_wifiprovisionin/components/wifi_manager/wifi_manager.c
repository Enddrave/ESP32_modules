#include "wifi_manager.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include <string.h>

#include "captive_portal.h"
#include "dns_server.h"

static const char *TAG = "WIFI";
static esp_netif_t *ap_netif = NULL;


/* ================= INIT ================= */

void wifi_manager_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ap_netif = esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}


/* ================= FORCE PHONE DNS TO ESP ================= */

static void force_dns_to_esp(void)
{
    esp_netif_ip_info_t ip;
    esp_netif_dns_info_t dns;

    esp_netif_get_ip_info(ap_netif, &ip);

    dns.ip.u_addr.ip4.addr = ip.ip.addr;
    dns.ip.type = ESP_IPADDR_TYPE_V4;

    esp_netif_set_dns_info(ap_netif, ESP_NETIF_DNS_MAIN, &dns);

    ESP_LOGW(TAG, "Phone DNS forced to ESP32: " IPSTR, IP2STR(&ip.ip));
}


/* ================= START PROVISION AP ================= */

void wifi_manager_start_softap(void)
{
    ESP_LOGW(TAG, "Starting Provisioning SoftAP...");

    wifi_config_t ap_config = {
        .ap = {
            .ssid = "ESP-PROVISION",
            .ssid_len = 0,
            .password = "12345678",
            .channel = 1,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK
        },
    };

    if (strlen("12345678") == 0)
        ap_config.ap.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    /* Wait DHCP to assign 192.168.4.1 */
    vTaskDelay(pdMS_TO_TICKS(1200));

    /* Force mobile DNS to ESP */
    force_dns_to_esp();

    /* Start DNS hijack server */
    dns_server_start();

    /* Start HTTP captive portal */
    captive_portal_start();

    ESP_LOGI(TAG, "Captive Portal Ready → Connect WiFi & popup should appear");
}


/* ================= CONNECT TO SAVED WIFI ================= */

void wifi_manager_connect_sta(void)
{
    ESP_LOGI(TAG, "Connecting to saved WiFi...");

    wifi_config_t sta_config = {0};

    nvs_handle_t nvs;
    if (nvs_open("wifi", NVS_READONLY, &nvs) != ESP_OK) {
        ESP_LOGW(TAG, "No saved WiFi credentials");
        return;
    }

    size_t ssid_len = sizeof(sta_config.sta.ssid);
    size_t pass_len = sizeof(sta_config.sta.password);

    if (nvs_get_str(nvs, "ssid", (char *)sta_config.sta.ssid, &ssid_len) != ESP_OK ||
        nvs_get_str(nvs, "pass", (char *)sta_config.sta.password, &pass_len) != ESP_OK) {
        ESP_LOGW(TAG, "Credentials not found in NVS");
        nvs_close(nvs);
        return;
    }

    nvs_close(nvs);

    ESP_LOGI(TAG, "Connecting to SSID: %s", sta_config.sta.ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}