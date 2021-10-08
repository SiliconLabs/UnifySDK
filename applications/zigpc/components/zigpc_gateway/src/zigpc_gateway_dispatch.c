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

#include <stdlib.h>

/* Contiki includes */
#include "sl_log.h"
#include "sl_status.h"

#include "zigpc_gateway.h"
#include "zigpc_gateway_int.h"

#define DISPATCH_CHAR_BUFFER_LENGTH 50
#define RETURN_STR_CASE_DISPATCH_TYPE(x) \
  case ZIGPC_GW_DISPATCH_##x:            \
    return #x;

sl_status_t zigpc_gateway_dispatch(struct zigpc_gateway_dispatch_event *event)
{
  sl_status_t result           = SL_STATUS_OK;
  EmberStatus operation_status = EMBER_ERR_FATAL;
  if (event == NULL) {
    sl_log_warning(LOG_TAG, "empty dispatch event received");
    result = SL_STATUS_NULL_POINTER;
  } else {
    switch (event->type) {
      case ZIGPC_GW_DISPATCH_NETWORK_INIT:
        operation_status = z3gatewayTrustCenterInit();
        break;
      case ZIGPC_GW_DISPATCH_ADD_NODE_INSTALL_CODE:
        operation_status = z3gatewayTrustCenterAddDeviceInstallCode(
          event->add_node.eui64,
          event->add_node.install_code,
          event->add_node.install_code_length);
        break;
      case ZIGPC_GW_DISPATCH_PERMIT_JOINS:
        if (event->permit_joins.enable) {
          const zigpc_config_t *conf = zigpc_get_config();
          if (conf->tc_use_well_known_key == true) {
            EmberEUI64 wildcardEui64    = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
            EmberKeyData centralizedKey = z3gatewayGetTrustCenterWellKownKey();
            operation_status = z3gatewayTrustCenterAddLinkKey(wildcardEui64,
                                                              &centralizedKey,
                                                              true);
          }
          operation_status = z3gatewayTrustCenterJoinOpen(true);

          zigpc_gateway_command_print_nwk_key();
        } else {
          operation_status = z3gatewayTrustCenterJoinClose();
        }
        sl_log_info(LOG_TAG,
                    "%s Joins status: 0x%X",
                    event->permit_joins.enable ? "Permit" : "Deny",
                    operation_status);
        break;
      case ZIGPC_GW_DISPATCH_REMOVE_NODE:
        operation_status
          = z3gatewayTrustCenterRemoveDevice(event->remove_node.eui64);
        break;
      case ZIGPC_GW_DISPATCH_INTERVIEW_NODE:
        operation_status
          = z3gatewayTrustCenterStartDiscovery(event->interview_node.eui64);
        break;
      default:
        sl_log_warning(LOG_TAG,
                       "invalid dispatch event(0x%X) received",
                       event->type);
        result = SL_STATUS_INVALID_TYPE;
    }

    /* Convert EmberStatus if a command was called */
    if (result == SL_STATUS_OK) {
      result
        = (operation_status == EMBER_SUCCESS) ? SL_STATUS_OK : SL_STATUS_FAIL;
    }
  }

  return result;
}

const char *
  zigpc_gateway_dispatch_name(const enum zigpc_gateway_dispatch_type type)
{
  static char str[DISPATCH_CHAR_BUFFER_LENGTH];
  switch (type) {
    RETURN_STR_CASE_DISPATCH_TYPE(NETWORK_INIT)
    RETURN_STR_CASE_DISPATCH_TYPE(ADD_NODE_INSTALL_CODE)
    RETURN_STR_CASE_DISPATCH_TYPE(INTERVIEW_NODE)
    RETURN_STR_CASE_DISPATCH_TYPE(REMOVE_NODE)
    RETURN_STR_CASE_DISPATCH_TYPE(PERMIT_JOINS)
    default:
      snprintf(str, DISPATCH_CHAR_BUFFER_LENGTH, "UNKNOWN TYPE: %d", type);
      return str;
  }
}

sl_status_t zigpc_gateway_dispatch_zcl_frame(
  const zigpc_gateway_dispatch_zcl_frame_t *zcl_data)
{
  sl_status_t status = SL_STATUS_OK;
  EmberStatus z3_status
    = z3gatewayFillZclFrame(zcl_data->frame.buffer,
                            zcl_data->frame.size,
                            zcl_data->frame.offset_sequence_id);

  if (z3_status == EMBER_SUCCESS) {
    if (zcl_data->send_as_multicast == true) {
      z3_status = z3gatewaySendZclFrameMulticast(zcl_data->multicast_id,
                                                 zcl_data->cluster_id);
    } else {
      z3_status = z3gatewaySendZclFrameUnicast(zcl_data->eui64,
                                               zcl_data->endpoint_id,
                                               zcl_data->cluster_id);
    }
  }

  status = (z3_status == EMBER_SUCCESS) ? SL_STATUS_OK : SL_STATUS_FAIL;

  return status;
}

sl_status_t zigpc_gateway_dispatch_init_reports(
  zigpc_gateway_dispatch_init_report_t *report_data)
{
  sl_status_t status    = SL_STATUS_OK;
  EmberStatus z3_status = z3gatewayInitReporting(report_data->eui64,
                                                 report_data->endpoint_id,
                                                 report_data->cluster_id,
                                                 report_data->report_record,
                                                 report_data->record_size);

  status = (z3_status == EMBER_SUCCESS) ? SL_STATUS_OK : SL_STATUS_FAIL;

  return status;
}

sl_status_t zigpc_gateway_dispatch_config_binding(
  zigpc_gateway_dispatch_bind_request_t *binding_data)
{
  sl_status_t status = SL_STATUS_OK;
  EmberStatus z3_status
    = z3gatewayInitBinding(binding_data->eui64,
                           binding_data->endpoint_id,
                           binding_data->cluster_id,
                           0);  //only unicast bindings for now

  status = (z3_status == EMBER_SUCCESS) ? SL_STATUS_OK : SL_STATUS_FAIL;

  return status;
}

sl_status_t zigpc_gateway_dispatch_ota_image(
  const zigpc_gateway_dispatch_add_ota_image_t *ota_image_data)
{
  EmberStatus emStatus = z3gatewayAddOtaImage(ota_image_data->filename);

  return (emStatus == EMBER_SUCCESS) ? SL_STATUS_OK : SL_STATUS_FAIL;
}
