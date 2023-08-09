/***************************************************************************//**
 * @file zigbee_common_callback_dispatcher.h
 * @brief ZigBee common dispatcher declarations.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SL_ZIGBEE_COMMON_CALLBACK_DISPATCHER_H
#define SL_ZIGBEE_COMMON_CALLBACK_DISPATCHER_H

#include PLATFORM_HEADER
#include "stack/include/ember.h"

// Event Init
void sli_zigbee_af_event_init(void);
void sli_zigbee_af_zcl_framework_core_init_events_callback(uint8_t init_level);
void sli_zigbee_af_service_discovery_init_events_callback(uint8_t init_level);
void sli_zigbee_af_network_steering_init_callback(uint8_t init_level);
void emberAfPluginReportingInitCallback(uint8_t init_level);
void sli_zigbee_af_scan_dispatch_init_callback(uint8_t init_level);
void emberAfPluginTrustCenterNwkKeyUpdatePeriodicInitCallback(uint8_t init_level);
void sli_zigbee_af_trust_center_nwk_key_update_unicast_init_callback(uint8_t init_level);
void sli_zigbee_af_update_tc_link_key_begin_tc_link_key_update_init(uint8_t init_level);
// Local data Init
void sli_zigbee_af_local_data_init(void);
void emberAfPluginConcentratorInitCallback(uint8_t init_level);
// Init done
void sli_zigbee_af_initDone(void);
void sli_zigbee_af_initCallback(uint8_t init_level);
void sli_zigbee_zcl_cli_init(uint8_t init_level);
void emberAfInit(uint8_t init_level);
void emberAfPluginCountersInitCallback(uint8_t init_level);
void emberAfPluginGatewayInitCallback(uint8_t init_level);
void emberAfPluginNetworkCreatorSecurityInitCallback(uint8_t init_level);
void sli_zigbee_af_ota_storage_posix_filesystsli_zigbee_init_callback(uint8_t init_level);
void emberAfPluginReportingInitCallback(uint8_t init_level);
void emberAfPluginTrustCenterNwkKeyUpdatePeriodicInitCallback(uint8_t init_level);
void sli_zigbee_af_network_init(uint8_t init_level);

// Tick
void sli_zigbee_af_tick(void);
void emberAfPluginGatewayTickCallback(void);
void sli_zigbee_af_host_framework_tick(void);
void emberAfOtaServerTick(void);

#endif // SL_ZIGBEE_COMMON_CALLBACK_DISPATCHER_H