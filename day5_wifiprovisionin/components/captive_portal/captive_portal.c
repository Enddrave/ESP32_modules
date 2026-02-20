#include "dns_server.h"

#include <string.h>
#include <stddef.h>

#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_wifi.h"

static const char *TAG = "CAPTIVE_PORTAL";

/* ========================================================= */
/* ROOT PAGE (shown to user)                                 */
/* ========================================================= */
static esp_err_t root_handler(httpd_req_t *req)
{
    const char *html =
        "<!DOCTYPE html><html><body>"
        "<h2>ESP32 WiFi Setup</h2>"
        "<form action=\"/wifi\" method=\"get\">"
        "SSID:<br><input name=\"s\"><br>"
        "Password:<br><input name=\"p\" type=\"password\"><br><br>"
        "<input type=\"submit\" value=\"Connect\">"
        "</form>"
        "</body></html>";

    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* ========================================================= */
/* CAPTIVE DETECTION HANDLER                                 */
/* Android/iOS/Windows hit special URLs → serve same page    */
/* ========================================================= */
static esp_err_t captive_detect_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Captive probe: %s", req->uri);
    return root_handler(req);  // serve page directly (no redirect)
}

/* ========================================================= */
/* WIFI SAVE HANDLER                                         */
/* ========================================================= */
static esp_err_t wifi_handler(httpd_req_t *req)
{
    char buf[200];
    size_t len = httpd_req_get_url_query_len(req) + 1;

    if (len > sizeof(buf))
        return ESP_FAIL;

    httpd_req_get_url_query_str(req, buf, len);

    char ssid[64] = {0};
    char pass[64] = {0};

    httpd_query_key_value(buf, "s", ssid, sizeof(ssid));
    httpd_query_key_value(buf, "p", pass, sizeof(pass));

    ESP_LOGI("WIFI", "Received SSID:%s PASS:%s", ssid, pass);

    /* Save credentials */
    nvs_handle_t nvs;
    nvs_open("wifi", NVS_READWRITE, &nvs);
    nvs_set_str(nvs, "ssid", ssid);
    nvs_set_str(nvs, "pass", pass);
    nvs_commit(nvs);
    nvs_close(nvs);

    const char *resp =
        "<html><body><h3>Connecting to WiFi...</h3>"
        "You can close this window</body></html>";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    vTaskDelay(pdMS_TO_TICKS(1200));
    esp_restart();
    return ESP_OK;
}

/* ========================================================= */
/* START CAPTIVE PORTAL                                      */
/* ========================================================= */
void captive_portal_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "HTTP server start failed");
        return;
    }

    /* ROOT */
    httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler
    };

    /* WIFI FORM SUBMIT */
    httpd_uri_t wifi = {
        .uri = "/wifi",
        .method = HTTP_GET,
        .handler = wifi_handler
    };

    /* ANDROID */
    httpd_uri_t android1 = { .uri="/generate_204", .method=HTTP_GET, .handler=captive_detect_handler };
    httpd_uri_t android2 = { .uri="/generate204", .method=HTTP_GET, .handler=captive_detect_handler };

    /* IOS */
    httpd_uri_t ios = { .uri="/hotspot-detect.html", .method=HTTP_GET, .handler=captive_detect_handler };

    /* WINDOWS */
    httpd_uri_t windows = { .uri="/connecttest.txt", .method=HTTP_GET, .handler=captive_detect_handler };

    /* XIAOMI */
    httpd_uri_t miui = { .uri="/ncsi.txt", .method=HTTP_GET, .handler=captive_detect_handler };

    /* REGISTER ALL */
    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &wifi);
    httpd_register_uri_handler(server, &android1);
    httpd_register_uri_handler(server, &android2);
    httpd_register_uri_handler(server, &ios);
    httpd_register_uri_handler(server, &windows);
    httpd_register_uri_handler(server, &miui);

    /* Start DNS hijack AFTER HTTP */
    dns_server_start();

    ESP_LOGI(TAG, "Captive portal ready");
}