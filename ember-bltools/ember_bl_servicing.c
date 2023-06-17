/*
 * Servicing for ember_bl.
 *
 * At the moment, this just helps reboot the node when an update starts running.
 *
 * 1. Add ember_bl_servicing_rf to your ember_task_list and have the correct CAN definitions.
 * 2. Provide the callbacks listed in ember_bl_servicing.h.
 * 3. Node should reboot when update comes in and it's safe to do so.
*/

#include "ember_bl_servicing.h"

#include <esp_system.h>
#include <freertos/FreeRTOS.h>

#include <ember_taskglue.h>
#include <opencan_rx.h>
#include <watchdog.h>

static void bl_servicing_10Hz(void);

ember_rate_funcs_S ember_bl_servicing_rf = {
    .call_10Hz = bl_servicing_10Hz,
};

static void bl_servicing_10Hz(void) {
    /* Critical section */
    portDISABLE_INTERRUPTS();
    {
        if (CANRX_is_node_UPD_ok() && CANRX_getRaw_UPD_currentIsoTpChunk() == 0U) {
            if (ember_bl_servicing_cb_are_we_ready_to_reboot()) {
                /* It's update time. No going back; we will reboot. */

                /**
                 * Set the RTC watchdog timeout to 1 second to give us some time
                 * since the task_wdt_servicer() is not running anymore.
                 */
                set_up_rtc_watchdog_1sec();

                /* Reboot */
                esp_restart();
            }
        }
    }
    portENABLE_INTERRUPTS();
}
