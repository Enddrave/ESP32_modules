#include "esp_camera.h"
#include <WiFi.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

const char* ssid = "Airtel_BK_5";
const char* password = "bk@12345";

#define FLASH_LED_PIN 4

WiFiServer server(80);

void startCamera() {

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

  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;   // 640x480 (stable)
    config.jpeg_quality = 12;            // stable quality
    config.fb_count = 1;                 // important for stability
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }

  esp_camera_init(&config);
}

void setup() {
  Serial.begin(115200);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  startCamera();

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Open: http://");
  Serial.println(WiFi.localIP());

  server.begin();
}

void handleStream(WiFiClient client) {

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println();

  while (client.connected()) {

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) continue;

    client.println("--frame");
    client.println("Content-Type: image/jpeg");
    client.print("Content-Length: ");
    client.println(fb->len);
    client.println();

    client.write(fb->buf, fb->len);
    client.println();

    esp_camera_fb_return(fb);

    if (!client.connected()) break;

    delay(60);   // lower FPS for stability (~15fps)
  }

  client.stop();
}

void loop() {

  WiFiClient client = server.available();
  if (!client) return;

  while (!client.available()) delay(1);

  String req = client.readStringUntil('\r');
  client.flush();

  if (req.indexOf("GET /stream") >= 0) {
    handleStream(client);
  }
  else if (req.indexOf("GET /flashon") >= 0) {
    digitalWrite(FLASH_LED_PIN, HIGH);
    client.println("HTTP/1.1 200 OK\r\n\r\n");
    client.stop();
  }
  else if (req.indexOf("GET /flashoff") >= 0) {
    digitalWrite(FLASH_LED_PIN, LOW);
    client.println("HTTP/1.1 200 OK\r\n\r\n");
    client.stop();
  }
  else {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<html><body style='text-align:center;background:black;color:white;'>");
    client.println("<h2>ESP32-CAM Live Stream</h2>");
    client.println("<img src='/stream'><br><br>");
    client.println("<a href='/flashon'><button>FLASH ON</button></a>");
    client.println("<a href='/flashoff'><button>FLASH OFF</button></a>");
    client.println("</body></html>");
    client.stop();
  }
}