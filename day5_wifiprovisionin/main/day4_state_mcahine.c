#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "state_machine.h"

void app_main(void)
{
    sm_init();

    while (1)
    {
        sm_process();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}