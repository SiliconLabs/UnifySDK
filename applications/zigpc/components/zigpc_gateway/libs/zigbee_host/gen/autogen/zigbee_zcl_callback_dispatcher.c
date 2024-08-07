/***************************************************************************//**
 * @file zigbee_zcl_callback_dispatcher.c
 * @brief ZCL callback dispatcher definitions.
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
#include "af.h"
#include "zigbee_zcl_callback_dispatcher.h"

// Reset Attributes
void sli_zigbee_af_reset_attributes(uint8_t endpointId)
{
  (void)endpointId;

}

// Pre command received
bool sli_zigbee_af_pre_command_received(sl_zigbee_af_cluster_command_t* cmd)
{
  return (
sl_zigbee_af_pre_command_received_cb(cmd)
);
}

// Pre ZDO message received
bool sli_zigbee_af_pre_zdo_message_received(sl_802154_short_addr_t nodeId, sl_zigbee_aps_frame_t* apsFrame, uint8_t* message, uint16_t length)
{
  return (
sl_zigbee_af_pre_zdo_message_received_cb(nodeId, apsFrame, message, length)
);
}

// ZDO message received
void sli_zigbee_af_zdo_message_received(sl_802154_short_addr_t sender, sl_zigbee_aps_frame_t* apsFrame, uint8_t* message, uint16_t length)
{
  (void)sender;
  (void)apsFrame;
  (void)message;
  (void)length;
  emAfZDOHandleBindingResponseCallback(sender, apsFrame, message, length);
}

// Retrieve attribute and craft response
bool sli_zigbee_af_retrieve_attribute_and_craft_response(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId, sl_zigbee_af_attribute_id_t attrId, uint8_t mask, uint16_t maunfacturerCode, uint16_t readLength)
{
  (void)endpoint;
  (void)clusterId;
  (void)attrId;
  (void)mask;
  (void)maunfacturerCode;
  (void)readLength;

return (false

);
}

// Read attributes response
bool sli_zigbee_af_read_attributes_response(sl_zigbee_af_cluster_id_t clusterId, uint8_t* buffer, uint16_t bufLen)
{
  return (
sl_zigbee_af_read_attributes_response_cb(clusterId, buffer, bufLen)
|| emberAfReadAttributesResponseCallback(clusterId, buffer, bufLen)
);
}

// Report attributes
bool sli_zigbee_af_report_attributes(sl_zigbee_af_cluster_id_t clusterId, uint8_t * buffer, uint16_t bufLen)
{
  return (
sl_zigbee_af_report_attributes_cb(clusterId, buffer, bufLen)
|| emberAfReportAttributesCallback(clusterId, buffer, bufLen)
);
}