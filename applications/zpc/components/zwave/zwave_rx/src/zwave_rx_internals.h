/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @defgroup zwave_rx_internals Z-Wave RX Internals
 * @ingroup zwave_rx
 * @brief Private definitions for the Z-Wave RX Component.
 *
 * These functions are used by the Z-Wave RX Fixture for init and teardown and
 * by the Z-Wave RX Process for listening to incoming messages from the Z-Wave
 * module.
 *
 * @{
 */

#ifndef ZWAVE_RX_INTERNALS_H
#define ZWAVE_RX_INTERNALS_H

// Includes from other components
#include "zwave_controller_connection_info.h"
#include "sl_status.h"

// Helper macro to return if the index exceeds the array length
#define ZWAVE_RX_VERIFY_OVERFLOW(current_index, array_length) \
  if (current_index >= array_length) {                        \
    return;                                                   \
  }

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Z-Wave Rx component
 *
 * This will setup the Z-Wave rx component and initialize the underlying
 * zwave api, by connecting to the Z-Wave controller serial dongle. A
 * contiki process will be spawnded, which will register itself with the
 * main loop select function.
 *
 * The dongle will be initalized with RF region and maximun transmission power.
 *
 * @param serial_port
 * Name of the serial port which hosts the Z-Wave controller module
 *
 * @param serial_port_fd
 * Pointer for the function to return the file descriptor used by the
 * Z-Wave API module.
 *
 * @param normal_tx_power_dbm
 * The power level used when transmitting frames at normal power. The power
 * level is in deci dBm, for example 1 dBm output power will be 10 in
 * normal_tx_power_dbm and -2 dBm will be -20 in normal_tx_power_dbm.
 *
 * @param measured_0dBm_power
 * The output power measured from the antenna when normal_tx_power_dbm is set
 * to 0 dBm. The power level is in deci dBm, for example 1dBm output power
 * will be 10 in measured_0dBm_power and -2 dBm will be -20 in
 * measured_0dBm_power.
 * @note On a protocols prior to 7.xx, measured_0dBm_power is ignored.
 *
 * @param max_lr_tx_power_dbm
 * The Z-Wave Max Long Range Transmit power level. The power
 * level is in deci dBm, for example 1 dBm output power will be 10 in
 * max_lr_tx_power_dbm and -2 dBm will be -20 in max_lr_tx_power_dbm.
 *
 * @param region
 * The region/frequency for the radio.
 * @note On a protocols prior to 7.xx the rf region is ignored.
 *
 * @returns SL_STATUS_SUCCESS if the initializations was successful.
 */
sl_status_t zwave_rx_init(const char *serial_port,
                          int *serial_port_fd,
                          int8_t normal_tx_power_dbm,
                          int8_t measured_0dBm_power,
                          int16_t max_lr_tx_power_dbm,
                          zwave_controller_region_t region);

/**
 * @brief Shutdown the module.
 *
 * This will shutdown the zwave_rx module, by closing the serial port
 * and stop the contiki process.
 */
void zwave_rx_shutdown();

/**
 * @brief Inform other components that Z-Wave RX just receievd a Z-Wave API started
 * notification
 */
void zwave_rx_on_zwave_api_started();

#ifdef __cplusplus
}
#endif
/** @} end zwave_rx */

#endif  //ZWAVE_RX_INTERNALS_H
