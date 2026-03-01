#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "blink.h"
#include "stdlib.h"
#include "stdio.h"


void app_main(void)
{
    blink_init();
    xTaskCreate(blink_task, "blink_task", 2048, NULL, 5, NULL);
}
