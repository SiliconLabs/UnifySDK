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
 * @file zigpc_ncp_updater.h
 * @defgroup zigpc_ncp_updater Capabilities for updating the NCP
 * @ingroup zigpc_components
 * @brief This header contains the external interface for the
 * component responsible for updating the Protocol Controller's
 * NCP radio.
 *
 * @{
 */
#ifndef ZIGPC_NCP_UPDATER_H
#define ZIGPC_NCP_UPDATER_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * @brief zigpc_ncp_updater_setup 
 * Set up the NCP Updater component. 
 *
 * @return SL_STATUS_OK if able to successfully initialize the component
 */
sl_status_t zigpc_ncp_updater_setup(void);

/* 
 * @brief zigpc_ncp_updater_run
 * The function that performs an NCP update on a given USB port,
 * with a given firmware image.
 *
 * @param serial_path   - the path to the serial port of the NCP
 * @param firmware_path - the path to the NCP firmware image
 *
 * @return SL_STATUS_OK if able to successfully update the device firmware
 */
sl_status_t zigpc_ncp_updater_run(
        const char* serial_path,
        const char* firmware_path);

/*
 * @brief zigpc_ncp_update_progress
 * The function that notifies the user of the firmware
 * upgrade progress. Currently, prints the percent done
 * in debug mode.
 *
 * @param offset    - the current index of the upgrade being sent
 * @param size      - the total size of the upgrade file
 *
 */ 
void zigpc_ncp_update_progress(int offset, int size);

/*
 * @brief zigpc_ncp_updater_shutdown
 * The function that shuts down the ncp_updater component
 *
 * @return 0 if able to successfully shut down
 */
int zigpc_ncp_updater_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif //ZIGPC_NCP_UPDATER
/** @} end zigpc_ncp_updater */
