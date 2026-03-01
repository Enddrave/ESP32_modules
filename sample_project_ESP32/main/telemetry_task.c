#include "abnormality.h"
#include "telemetry_task.h"
#include "producer_queue.h"
#include "data_model.h"
#include "azure_iot.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

/* ================= CONFIG ================= */

#define DHT_COUNT               5
#define DOOR_COUNT              2
#define TELEMETRY_PERIOD_MS     7000   // 7 seconds

static const char *TAG = "TELEMETRY";

/* ================= LOCAL CACHE ================= */

static sensor_data_t dht_cache[DHT_COUNT];
static sensor_data_t door_cache[DOOR_COUNT];

static bool dht_valid[DHT_COUNT]   = {0};
static bool door_valid[DOOR_COUNT] = {1, 1};  // doors always valid

/* ================= JSON BUILDER ================= */

static void build_batch_json(char *buf, size_t len)
{
    char *p = buf;
    size_t remaining = len;
    int n;

    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);

    /* ---------------- HEADER ---------------- */
    n = snprintf(p, remaining,
        "{"
        "\"deviceId\":\"ENV-NODE-01\","
        "\"location\":\"Factory Lab - Gurugram\","
        "\"firmwareVersion\":\"v1.0.3\","
        "\"status\":\"online\","
        "\"ts\":%lu,",
        (unsigned long)now
    );
    p += n; remaining -= n;

    /* ---------------- DHT22 ARRAY ---------------- */
    n = snprintf(p, remaining, "\"dht22\":[");
    p += n; remaining -= n;

    bool first = true;
    float temps[DHT_COUNT] = {0};
    float hums[DHT_COUNT]  = {0};

    for (int i = 0; i < DHT_COUNT; i++) {
        if (!dht_valid[i]) continue;

        temps[i] = dht_cache[i].temperature;
        hums[i]  = dht_cache[i].humidity;

        n = snprintf(p, remaining,
            "%s{\"id\":%d,\"temperature\":%.2f,\"humidity\":%.2f}",
            first ? "" : ",",
            dht_cache[i].sensor_id,
            dht_cache[i].temperature,
            dht_cache[i].humidity
        );
        p += n; remaining -= n;
        first = false;
    }

    n = snprintf(p, remaining, "],");
    p += n; remaining -= n;

    /* ---------------- DOOR ARRAY ---------------- */
    n = snprintf(p, remaining, "\"doors\":[");
    p += n; remaining -= n;

    first = true;
    bool door_closed = true;

    for (int i = 0; i < DOOR_COUNT; i++) {
        if (!door_valid[i]) continue;

        if (door_cache[i].logic_level == 1) {
            door_closed = false;
        }

        n = snprintf(p, remaining,
            "%s{\"id\":%d,\"state\":%d}",
            first ? "" : ",",
            door_cache[i].sensor_id,
            door_cache[i].logic_level
        );
        p += n; remaining -= n;
        first = false;
    }

    n = snprintf(p, remaining, "],");
    p += n; remaining -= n;

    /* ---------------- ABNORMALITY SCORE ---------------- */
    abnormality_result_t abn = abnormality_compute(
        temps,
        hums,
        dht_valid,
        DHT_COUNT,
        door_closed,
        (uint8_t)tm_info.tm_hour
    );

    n = snprintf(p, remaining,
        "\"abnormality\":{"
            "\"score\":%.2f,"
            "\"state\":\"%s\","
            "\"components\":{"
                "\"thermal\":%.2f,"
                "\"roc\":%.2f,"
                "\"consensus\":%.2f,"
                "\"door\":%.2f,"
                "\"baseline\":%.2f"
            "}"
        "}"
        "}",
        abn.score,
        abn.state,
        abn.components.thermal,
        abn.components.roc,
        abn.components.consensus,
        abn.components.door,
        abn.components.baseline
    );
}


/* ================= TELEMETRY TASK ================= */

void telemetry_task(void *arg)
{
    sensor_data_t incoming;
    char payload[1024];

    TickType_t last_wake_time = xTaskGetTickCount();

    ESP_LOGI(TAG, "Telemetry task started (2 sec batch)");

    /* ---- Initialize door cache so JSON is never empty ---- */
    for (int i = 0; i < DOOR_COUNT; i++) {
        door_cache[i].type        = SENSOR_GPIO;
        door_cache[i].sensor_id   = i;
        door_cache[i].logic_level = 0;  // default CLOSED
        door_cache[i].timestamp   = time(NULL);
        door_valid[i]             = true;
    }

    while (1) {

        /* -------- Drain queue -------- */
        while (producer_queue_receive(&incoming, 0)) {

            if (incoming.type == SENSOR_DHT22 &&
                incoming.sensor_id < DHT_COUNT) {

                dht_cache[incoming.sensor_id] = incoming;
                dht_valid[incoming.sensor_id] = true;
            }
            else if (incoming.type == SENSOR_GPIO &&
                     incoming.sensor_id < DOOR_COUNT) {

                door_cache[incoming.sensor_id] = incoming;
                door_valid[incoming.sensor_id] = true;
            }
        }

        vTaskDelayUntil(&last_wake_time,
                        pdMS_TO_TICKS(TELEMETRY_PERIOD_MS));

        memset(payload, 0, sizeof(payload));
        build_batch_json(payload, sizeof(payload));

        ESP_LOGI(TAG, "Sending batch telemetry: %s", payload);
        azure_iot_send(payload);
    }
}