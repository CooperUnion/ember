#ifndef EMBER_APP_DESC_H
#define EMBER_APP_DESC_H

#include <stdint.h>

/**
 * See https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/system/app_image_format.html#adding-a-custom-structure-to-an-application
 */

#define IN_DESC_SECTION __attribute__((section(".rodata_custom_desc")))

// Just make sure char is 1 byte wide
_Static_assert(sizeof(char) == 1, "Expected char to be 1 byte wide");

#define EMBER_MAGIC "COOPERU"
#define EMBER_APP_DESC_VERSION 1

// Ember app description struct.
typedef struct __attribute__((packed)) {
    const char ember_magic[sizeof(EMBER_MAGIC)];
    const uint16_t app_desc_version;

    const char node_identity[16];
} ember_app_desc_v1_t;

// Double-check size of struct
_Static_assert(sizeof(ember_app_desc_v1_t) == 26, "Expected app desc to be 26 bytes wide");

// Global declaration of the description struct in .rodata_custom_desc
// Be aware that the bootloader and app will have their own ember_app_description.
extern const IN_DESC_SECTION ember_app_desc_v1_t ember_app_description;

#endif
