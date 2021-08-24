/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef SILABS_PLUGIN_DEVICE_TABLE_INTERNAL_H
#define SILABS_PLUGIN_DEVICE_TABLE_INTERNAL_H

/**
 * This file contains the prototypes and declartions needed to run z3gateway unit
 * tests. For more details, refer to:
 * <GSDK>/protocol/zigbee/app/framework/plugin/device-table/device-table-internal.h
 */

uint16_t emAfDeviceTableFindIndexNodeIdEndpoint(uint16_t nodeId,
                                                uint8_t endpoint);

#endif /* SILABS_PLUGIN_DEVICE_TABLE_INTERNAL_H */
