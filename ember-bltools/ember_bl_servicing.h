#pragma once

#include <stdbool.h>

/*
 * Code should provide this callback to tell ember_bl_servicing
 * whether the node is ready to reboot.
 *
 * This is called in a critical section with interrupts disabled.
*/
bool ember_bl_servicing_cb_are_we_ready_to_reboot(void);
