#include "esp_camera.h"
#include <WiFi.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

const char* ssid = "Airtel_BK_2.4";
const char* password = "bk@12345";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

if (psramFound()) {
  config.frame_size   = FRAMESIZE_HVGA;   // 800 x600
  config.jpeg_quality = 25;                // High quality
  config.fb_count     = 1;
}else {
  config.frame_size   = FRAMESIZE_HVGA;   // 800 x 600
  config.jpeg_quality = 25;                // High quality
  config.fb_count     = 1;
  }


  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Camera IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {

  WiFiClient client = server.available();
  if (!client) return;

  while (!client.available()) delay(1);

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    client.println("HTTP/1.1 500 Internal Server Error");
    client.println("Connection: close");
    client.println();
    client.stop();
    return;
  }

  // ===== Proper HTTP Header =====
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/jpeg");
  client.println("Content-Length: " + String(fb->len));
  client.println("Connection: close");
  client.println();

  // ===== Send in chunks (VERY IMPORTANT) =====
  size_t toSend = fb->len;
  uint8_t *ptr = fb->buf;

  while (toSend > 0) {
    size_t chunk = toSend > 1024 ? 1024 : toSend;
    client.write(ptr, chunk);
    ptr += chunk;
    toSend -= chunk;
    delay(1);  // prevent packet drop
  }

  client.flush();
  esp_camera_fb_return(fb);
  client.stop();
}