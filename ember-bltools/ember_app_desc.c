#include "ember_app_desc.h"

#ifndef EMBER_NODE_IDENTITY
    #error "EMBER_NODE_IDENTITY must be defined"
#endif

// stringification macros
#define XSTR(s) STR(s)
#define STR(s) #s

const IN_DESC_SECTION ember_app_desc_v1_t ember_app_description = {
    .ember_magic = EMBER_MAGIC,
    .app_desc_version = EMBER_APP_DESC_VERSION,

    .node_identity = XSTR(EMBER_NODE_IDENTITY),
};
