/***************************************************************************//**
 * @file zigbee_common_callback_dispatcher.c
 * @brief ZigBee common dispatcher definitions.
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

#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "zigbee_app_framework_common.h"
#include "zigbee_common_callback_dispatcher.h"

void emAfEventInit(void)
{
  emAfZclFrameworkCoreInitEventsCallback(SL_ZIGBEE_INIT_LEVEL_EVENT);
  emAfServiceDiscoveryInitEventsCallback(SL_ZIGBEE_INIT_LEVEL_EVENT);
  emberAfPluginGatewayInitCallback(SL_ZIGBEE_INIT_LEVEL_EVENT);
  emAfPluginNetworkSteeringInitCallback(SL_ZIGBEE_INIT_LEVEL_EVENT);
  emberAfPluginReportingInitCallback(SL_ZIGBEE_INIT_LEVEL_EVENT);
  emAfPluginScanDispatchInitCallback(SL_ZIGBEE_INIT_LEVEL_EVENT);
  emberAfPluginTrustCenterNwkKeyUpdatePeriodicInitCallback(SL_ZIGBEE_INIT_LEVEL_EVENT);
  emAfPluginTrustCenterNwkKeyUpdateUnicastInitCallback(SL_ZIGBEE_INIT_LEVEL_EVENT);
  emAfPluginUpdateTcLinkKeyBeginTcLinkKeyUpdateInit(SL_ZIGBEE_INIT_LEVEL_EVENT);
}

void emAfLocalDataInit(void)
{
  emberAfPluginConcentratorInitCallback(SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA);
}

void emAfInitDone(void)
{
  emAfInitCallback(SL_ZIGBEE_INIT_LEVEL_DONE);
  emberAfInit(SL_ZIGBEE_INIT_LEVEL_DONE);
  emberAfPluginCountersInitCallback(SL_ZIGBEE_INIT_LEVEL_DONE);
  emberAfPluginGatewayInitCallback(SL_ZIGBEE_INIT_LEVEL_DONE);
  emberAfPluginNetworkCreatorSecurityInitCallback(SL_ZIGBEE_INIT_LEVEL_DONE);
  emAfPluginOtaStoragePosixFilesystemInitCallback(SL_ZIGBEE_INIT_LEVEL_DONE);
  emberAfPluginReportingInitCallback(SL_ZIGBEE_INIT_LEVEL_DONE);
  emberAfPluginTrustCenterNwkKeyUpdatePeriodicInitCallback(SL_ZIGBEE_INIT_LEVEL_DONE);
  emAfNetworkInit(SL_ZIGBEE_INIT_LEVEL_DONE);
}


void emAfTick(void)
{
  emAfHostFrameworkTick();
  emberAfOtaServerTick();
}
