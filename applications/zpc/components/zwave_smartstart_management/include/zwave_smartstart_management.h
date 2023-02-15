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
 * @defgroup zwave_smartstart_management Z-Wave SmartStart Management
 * @ingroup zpc_components
 * @brief Z-Wave SmartStart management component
 *
 * This component makes ZPC specific usage of the @ref unify_smartstart_management
 * component.
 *
 * This component is responsible for reacting to SmartStart inclusion requests,
 * which contains a partial DSK, and check with the local cache that keeps
 * updated with ucl/SmartStart/List MQTT topic. Furthermore, this component may
 * ignore the request if an invalid inclusion request is found.
 *
 * @{
 */

#ifndef ZWAVE_SMARTSTART_MANAGEMENT_H
#define ZWAVE_SMARTSTART_MANAGEMENT_H

#include "zwave_controller_types.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Verify if the device DSK is in smart list by comparing
 * DSK sent in the param with full DSK in smart start list 
 *
 * @param dsk DSK which will be searched in the smartstart list.
 * @param obfuscated_bytes no of bytes obfuscated in the dsk. These many bytes
 *                          are skipped in comparing the DSKs
 * @returns true if dsk is in SmartStart. And fill the indicated number of 
 *              obfuscated bytes in the dsk from the smart list and return true.
 *          false otherwise.
 */

bool find_dsk_obfuscated_bytes_from_smart_start_list(zwave_dsk_t dsk,
                                                     uint8_t obfuscated_bytes);
#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_SMARTSTART_MANAGEMENT_H
        /** @} end zwave_smartstart_management */
