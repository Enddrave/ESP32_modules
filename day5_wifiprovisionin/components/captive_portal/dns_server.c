#include "dns_server.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include <string.h>

#define DNS_PORT 53

static const char *TAG = "DNS";

static void dns_task(void *pv)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(DNS_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    ESP_LOGI(TAG, "DNS server started");

    uint8_t rx[256];

    while (1)
    {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);

        int r = recvfrom(sock, rx, sizeof(rx), 0,
                        (struct sockaddr*)&client, &len);

        if (r <= 0) continue;

        /* convert query into response */
        rx[2] |= 0x80; // response flag
        rx[3] |= 0x80; // recursion available
        rx[7] = 1;     // answer count

        /* append answer */
        uint8_t answer[] = {
            0xC0,0x0C, // pointer
            0x00,0x01, // type A
            0x00,0x01, // class IN
            0x00,0x00,0x00,0x3C, // TTL
            0x00,0x04, // length
            192,168,4,1 // ESP IP
        };

        memcpy(rx + r, answer, sizeof(answer));
        r += sizeof(answer);

        sendto(sock, rx, r, 0,
               (struct sockaddr*)&client, len);
    }
}

void dns_server_start(void)
{
     
    xTaskCreate(dns_task, "dns", 4096, NULL, 5, NULL);
}