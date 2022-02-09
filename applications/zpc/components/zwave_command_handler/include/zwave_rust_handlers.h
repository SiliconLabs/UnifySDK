#ifndef ZWAVE_RUST_HANDLERS_H
#define ZWAVE_RUST_HANDLERS_H
#include "sl_status.h"

/**
 * @brief Setup fixture for @ref rust command_classes.
 *
 * This setup function will call the setup/initialization functions for
 * all command classes defined in rust. see rust project rust_command_classes
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_init_rust_handlers();
#endif