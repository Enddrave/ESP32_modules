#include <WiFi.h>
#include <TJpg_Decoder.h>

#include "rgb_lcd_port.h"
#include "gui_paint.h"
#include "gt911.h"
#include "fonts.h"
#include "i2c.h"

#define ROTATE ROTATE_0

// ================= WIFI =================
const char* ssid = "Airtel_BK_2.4";
const char* password = "bk@12345";

#define CAM_IP "192.168.1.7"
#define CAM_PORT 80

// ================= LCD Layout =================
UBYTE *BlackImage;

#define PREVIEW_X 60
#define PREVIEW_Y 60
#define PREVIEW_W 750
#define PREVIEW_H 500

unsigned long lastFrameTime = 0;
#define STREAM_INTERVAL 120   // ~8 FPS

// ================= TOUCH HELPER =================
bool isCirclePressed(int tx, int ty, int cx, int cy)
{
    int dx = tx - cx;
    int dy = ty - cy;
    return (dx * dx + dy * dy <= 50 * 50);
}

// ================= JPEG CALLBACK =================
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    if (y >= PREVIEW_H) return 0;

    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            Paint_SetPixel(PREVIEW_X + x + i,
                           PREVIEW_Y + y + j,
                           bitmap[j * w + i]);
    return 1;
}

// ================= HEAVY DEBUG FETCH =================
bool fetchJPEG(uint8_t **buffer, int &length)
{
    WiFiClient client;

    Serial.println("\n==========================");
    Serial.println("NEW FRAME REQUEST");
    Serial.println("==========================");

    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());

    Serial.print("Free PSRAM: ");
    Serial.println(ESP.getFreePsram());

    Serial.print("Connecting to ");
    Serial.println(CAM_IP);

    if (!client.connect(CAM_IP, CAM_PORT))
    {
        Serial.println("❌ TCP Connection FAILED");
        return false;
    }

    Serial.println("✅ TCP Connected");

    // Send HTTP request manually
    client.println("GET /capture HTTP/1.1");
    client.print("Host: ");
    client.println(CAM_IP);
    client.println("Connection: close");
    client.println();

    // Wait for response
    unsigned long timeout = millis();
    while (!client.available())
    {
        if (millis() - timeout > 3000)
        {
            Serial.println("❌ Timeout waiting for response");
            client.stop();
            return false;
        }
    }

    Serial.println("Response received");

    // Skip HTTP headers
    Serial.println("Skipping HTTP headers...");
    while (client.available())
    {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
            Serial.println("Header End Found");
            break;
        }
        Serial.print("HEADER: ");
        Serial.println(line);
    }

    int maxSize = 150000; // Safe for VGA
    *buffer = (uint8_t*)ps_malloc(maxSize);

    if (!*buffer)
    {
        Serial.println("❌ PSRAM Allocation FAILED");
        client.stop();
        return false;
    }

    Serial.println("✅ PSRAM Allocated");

    length = 0;
    bool jpegStarted = false;

    while (client.connected())
    {
        while (client.available())
        {
            uint8_t c = client.read();

            // Detect JPEG Start
            if (!jpegStarted)
            {
                if (c == 0xFF && client.peek() == 0xD8)
                {
                    jpegStarted = true;
                    Serial.println("📸 JPEG Start Detected");
                    (*buffer)[length++] = c;
                }
            }
            else
            {
                (*buffer)[length++] = c;

                // Detect JPEG End
                if (length > 1 &&
                    (*buffer)[length - 2] == 0xFF &&
                    (*buffer)[length - 1] == 0xD9)
                {
                    Serial.println("📸 JPEG End Detected");
                    Serial.print("Total JPEG Bytes: ");
                    Serial.println(length);

                    client.stop();
                    return true;
                }

                if (length >= maxSize)
                {
                    Serial.println("❌ Buffer Overflow Protection Triggered");
                    client.stop();
                    return false;
                }
            }
        }
    }

    Serial.println("❌ JPEG Not Completed");
    client.stop();
    return false;
}

// ================= DISPLAY =================
void showJPEG(uint8_t *buf, int len)
{
    Serial.println("Decoding JPEG...");

    Paint_DrawRectangle(PREVIEW_X+1, PREVIEW_Y+1,
                        PREVIEW_X+PREVIEW_W-1,
                        PREVIEW_Y+PREVIEW_H-1,
                        BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    TJpgDec.drawJpg(0, 0, buf, len);

    wavesahre_rgb_lcd_display(BlackImage);

    Serial.println("✅ Frame Displayed");
}

// ================= GUI =================
void drawGUI()
{
    Paint_Clear(BLACK);

    Paint_DrawString_EN(250, 10,
    "LIVE STREAM DEBUG MODE",
    &Font16, CYAN, BLACK);

    Paint_DrawRectangle(PREVIEW_X, PREVIEW_Y,
                        PREVIEW_X + PREVIEW_W,
                        PREVIEW_Y + PREVIEW_H,
                        WHITE, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);

    wavesahre_rgb_lcd_display(BlackImage);
}

// ================= SETUP =================
void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n================================");
    Serial.println("ESP32-S3 LCD CAMERA DEBUG MODE");
    Serial.println("================================");

    touch_gt911_init();
    waveshare_esp32_s3_rgb_lcd_init();
    wavesahre_rgb_lcd_bl_on();

    UDOUBLE size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2;
    BlackImage = (UBYTE*)heap_caps_malloc(size, MALLOC_CAP_SPIRAM);

    if (!BlackImage)
        Serial.println("❌ LCD Frame Buffer FAILED");
    else
        Serial.println("✅ LCD Frame Buffer OK");

    Paint_NewImage(BlackImage, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES, 0, BLACK);
    Paint_SetScale(65);
    Paint_SetRotate(ROTATE);

    TJpgDec.setCallback(tft_output);

    drawGUI();

    Serial.println("Connecting WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\n✅ WiFi Connected");
    Serial.print("LCD IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal Strength: ");
    Serial.println(WiFi.RSSI());
}

// ================= LOOP =================
void loop()
{
    if (millis() - lastFrameTime > STREAM_INTERVAL)
    {
        lastFrameTime = millis();

        uint8_t *buf;
        int len;

        if (fetchJPEG(&buf, len))
        {
            showJPEG(buf, len);
            free(buf);
        }
        else
        {
            Serial.println("❌ Frame Fetch Failed");
        }
    }
}