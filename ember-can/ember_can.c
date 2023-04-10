#include "ember_can_callbacks.h"

#include <stdio.h>
#include <string.h>

#include <esp_ota_ops.h>
#include <esp_timer.h>
#include <driver/twai.h>

#include "ember_common.h"
#include "ember_taskglue.h"
#include "node_pins.h"
#include "opencan_callbacks.h"
#include "opencan_rx.h"
#include "opencan_tx.h"

// ######        DEFINES        ###### //

#define CAN_TX_GPIO NODE_BOARD_PIN_CANTX
#define CAN_RX_GPIO NODE_BOARD_PIN_CANRX

// ######      PROTOTYPES       ###### //

void freelunch_can_rx_callback(const twai_message_t *message);

// ######     PRIVATE DATA      ###### //

static bool can_ready;

// ######    RATE FUNCTIONS     ###### //

static void can_init();
static void can_1kHz();

ember_rate_funcs_S can_rf = {
    .call_init = can_init,
    .call_1kHz = can_1kHz,
};

static void can_init()
{
    // see TWAI_GENERAL_CONFIG_DEFAULT in driver/twai.h
    const twai_general_config_t g_config = {
        .mode = TWAI_MODE_NO_ACK,
        .tx_io = CAN_TX_GPIO,
        .rx_io = CAN_RX_GPIO,
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 8,
        .rx_queue_len = 8,
        .alerts_enabled = TWAI_ALERT_NONE,
        .clkout_divider = 0,
        .intr_flags = ESP_INTR_FLAG_LEVEL1
    };

    const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        can_ready = 1;
    } else {
        can_ready = 0;
    }

    if (twai_start() == ESP_OK) {
        can_ready &= 1;
    } else {
        can_ready &= 0;
    }
}

// RX and TX for CAN. RX needs to be split off into a separate task.
static void can_1kHz()
{
    /* Run TX scheduler */
    CANTX_scheduler_1kHz();

    /**
     * RX
     *
     * Loop trying twai_recieve() until we run out of messages to process.
     *
     * todo: this is synchronous at 1kHz, when it really should be asynchronous
     */
    for (;;) {
        twai_message_t message;
        const esp_err_t result = twai_receive(&message, 0);

        switch (result) {
            case ESP_ERR_TIMEOUT: // no messages in queue
                return;

            case ESP_OK:
                CANRX_handle_rx(message.identifier, message.data, message.data_length_code);
                freelunch_can_rx_callback(&message);
                break;

            default: // error
                return;
        }
    }
}

// ######   PRIVATE FUNCTIONS   ###### //

// ######   PUBLIC FUNCTIONS    ###### //

void CAN_callback_enqueue_tx_message(const uint8_t * const data, const uint8_t len, const uint32_t id) {
    /* prepare twai_message_t */
    twai_message_t message = {
        .identifier = id,
        .extd = true, // todo
        .data_length_code = len,
        .self = true, // for freelunch
    };

    /* copy the provided data into the twai_message_t */
    memcpy(message.data, data, len);

    /* send/enqueue message */
    const esp_err_t r = twai_transmit(&message, 0);
    if (r != ESP_OK) {
        ember_can_callback_notify_lost_can();
    }
}

uint64_t CAN_callback_get_system_time(void) {
    return esp_timer_get_time();
}
