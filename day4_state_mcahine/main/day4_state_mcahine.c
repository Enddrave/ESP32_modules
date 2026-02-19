#include <stdio.h>
#include "state_machine.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "storage.h"
#include "device_types.h"
void app_main(void)
{
    sm_init();

        // ---- TEST: force save NORMAL state ----
    storage_save_state(DEVICE_STATE_NORMAL);

    
    while(1) {
        sm_run();
        vTaskDelay(pdMS_TO_TICKS(3000)); // Run state machine every 3 seconds   
    }
}
