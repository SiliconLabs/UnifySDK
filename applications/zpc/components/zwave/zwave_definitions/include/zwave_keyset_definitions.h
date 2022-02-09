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
 * @defgroup zwave_keyset_definitions Z-Wave Keyset definitions
 * @ingroup zwave_definitions
 * @brief Z-Wave Security Keys definitions and constants
 *
 * @{
 */

#ifndef ZWAVE_KEYSET_DEFINITIONS_H
#define ZWAVE_KEYSET_DEFINITIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#define ZWAVE_CONTROLLER_S0_KEY                 0x80
#define ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY 0x01
#define ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY   0x02
#define ZWAVE_CONTROLLER_S2_ACCESS_KEY          0x04

typedef uint8_t zwave_keyset_t;

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_KEYSET_DEFINITIONS_H
/** @} end zwave_keyset_definitions */
