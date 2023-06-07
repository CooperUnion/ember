#include "watchdog.h"

#include <esp_attr.h>
#include <freertos/FreeRTOS.h>
#include <hal/wdt_hal.h>
#include <hal/rwdt_ll.h>
#include <soc/rtc.h>

#include "ember_common.h"

/*
 * watchdog - EXPERIMENTAL IMPLEMENTATION
 */

// ######        DEFINES        ###### //

// note on init timeout: init currently takes <1ms
// note: anything <25ms or so will screw with JTAG
// and you'll have a bad time
#define INIT_TIMEOUT_MS 25
#define FINAL_TIMEOUT_MS 25
#define FW_UPDATE_TIMEOUT_MS 1000

// ######      PROTOTYPES       ###### //

// ######     PRIVATE DATA      ###### //

static volatile bool task_1Hz_checkin;
static volatile bool task_10Hz_checkin;
static volatile bool task_100Hz_checkin;
static volatile bool task_1kHz_checkin;

static wdt_hal_context_t hal = {
    .inst = WDT_RWDT,
    .rwdt_dev = &RTCCNTL
};

// ######   PRIVATE FUNCTIONS   ###### //

/*
 * Kick hardware watchdog.
 */
static IRAM_ATTR void kick_rtc_watchdog(void)
{
#ifndef EMBER_TASKING_DISABLE_WATCHDOG
    wdt_hal_write_protect_disable(&hal);
    wdt_hal_feed(&hal);
    wdt_hal_write_protect_enable(&hal);
#endif
}

// ######   PUBLIC FUNCTIONS    ###### //

void task_1Hz_wdt_kick()
{
    task_1Hz_checkin = true;
}

void task_10Hz_wdt_kick()
{
    task_10Hz_checkin = true;
}

void task_100Hz_wdt_kick()
{
    task_100Hz_checkin = true;
}

void task_1kHz_wdt_kick()
{
    task_1kHz_checkin = true;
}

/*
 * Checks task checkins and ensures timing deadlines were met. Incorrect timing
 * will latch (static bool violated) and cause the watchdog to reset the system.
 *
 * Run this function in the 1kHz interrupt.
 */
void IRAM_ATTR task_wdt_servicer()
{
    // maintain count of ticks since last checkin/kick
    static uint tick_count_1kHz = 0;
    static uint tick_count_100Hz = 0;
    static uint tick_count_10Hz = 0;
    static uint tick_count_1Hz = 0;

    tick_count_1kHz++;
    tick_count_100Hz++;
    tick_count_10Hz++;
    tick_count_1Hz++;

    if (task_1kHz_checkin) {
        tick_count_1kHz = 0;
        task_1kHz_checkin = false;
    }

    if (task_100Hz_checkin) {
        tick_count_100Hz = 0;
        task_100Hz_checkin = false;
    }

    if (task_10Hz_checkin) {
        tick_count_10Hz = 0;
        task_10Hz_checkin = false;
    }

    if (task_1Hz_checkin) {
        tick_count_1Hz = 0;
        task_1Hz_checkin = false;
    }

    /*
     * Below, we check if any of the tasks have exceeded their timing limits.
     * For example, there are at most 1000 ticks permitted between each checkin
     * of the 1Hz task. 1001 ticks passing would mean that the 1Hz task had more
     * than 1 second between runs.
     *
     * For the 1Hz task: (1kHz watchdog tick rate)/(1Hz task rate) = 1000
     */

    static bool violated; // static so we latch forever

    violated |= tick_count_1kHz > 1;
    violated |= tick_count_100Hz > 10;
    violated |= tick_count_10Hz > 100;
    violated |= tick_count_1Hz > 1000;

    if (!violated) {
        kick_rtc_watchdog();
    }
    // implicit else is that the watchdog resets the SoC shortly after
}

/*
 * Set up and activate the rtc watchdog with the given timeout. Note that there
 * can only be one watchdog active at a time - this function will override any
 * previous active watchdog.
 *
 * //todo: error handling
 */
void set_up_rtc_watchdog(uint32_t timeout_ms)
{
#ifndef EMBER_TASKING_DISABLE_WATCHDOG
    wdt_hal_deinit(&hal);
    wdt_hal_init(&hal, hal.inst, 0, false);
    wdt_hal_write_protect_disable(&hal);

    // ticks calculation derived from esp-idf (see esp_restart_noos())
    const uint32_t stage_timeout_ticks =
        (uint32_t)((uint64_t)timeout_ms * rtc_clk_slow_freq_get_hz() / 1000);

    wdt_hal_config_stage(&hal, WDT_STAGE0, stage_timeout_ticks, WDT_STAGE_ACTION_RESET_RTC);
    wdt_hal_enable(&hal);
    wdt_hal_write_protect_enable(&hal);
#else
    (void)timeout_ms;
#endif
}

/*
 * Configure the watchdog with a temporarily larger timeout so we can run the
 * taskinig init and call_init()'s and have some insurance against the system
 * hanging while we do it.
 */
void set_up_rtc_watchdog_for_init()
{
    set_up_rtc_watchdog(INIT_TIMEOUT_MS);
}

/*
 * Configure the watchdog with the final task-running timeout.
 *
 * You're on very strict time once this happens!
 */
void set_up_rtc_watchdog_final()
{
    set_up_rtc_watchdog(FINAL_TIMEOUT_MS);
}

void set_up_rtc_watchdog_1sec()
{
    set_up_rtc_watchdog(FW_UPDATE_TIMEOUT_MS);
}
