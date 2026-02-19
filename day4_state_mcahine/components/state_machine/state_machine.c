#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "state_machine.h"
#include "storage.h"
#include "states.h"

static device_state_t state;

/* Load last saved state */
void sm_init(void)
{
    storage_init();
    state = storage_load_state();
}

/* change state (single place) */
static void change_state(device_state_t new_state)
{
    if (new_state == state)
        return;

    state = new_state;
    storage_save_state(state);
}

/* run one step of state machine */
void sm_run(void)
{
    switch (state)
    {
        case DEVICE_STATE_FACTORY:

            state_factory_enter();      // behaviour
            vTaskDelay(pdMS_TO_TICKS(3000));

            change_state(DEVICE_STATE_PROVISIONING);
            break;


        case DEVICE_STATE_PROVISIONING:

            state_provisioning_enter();
            vTaskDelay(pdMS_TO_TICKS(3000));

            change_state(DEVICE_STATE_NORMAL);
            break;


        case DEVICE_STATE_NORMAL:

            state_normal_enter();
            vTaskDelay(pdMS_TO_TICKS(3000));

            change_state(DEVICE_STATE_RECOVERY);
            break;


        case DEVICE_STATE_RECOVERY:

            state_recovery_enter();
            vTaskDelay(pdMS_TO_TICKS(3000));

            change_state(DEVICE_STATE_FACTORY);
            break;
    }
}