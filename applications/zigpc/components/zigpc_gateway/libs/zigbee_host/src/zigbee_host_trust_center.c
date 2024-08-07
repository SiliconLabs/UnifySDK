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

#include PLATFORM_HEADER
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/security/af-security.h"
#include "app/framework/security/security-config.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"
#include "app/framework/plugin/network-creator/network-creator.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"

#define SL_ZIGBEE_INSTALL_CODE_MAX_SIZE (16 + SL_ZIGBEE_INSTALL_CODE_CRC_SIZE)

extern struct zigbeeHostState z3gwState;

/** @brief ZigbeeHost handler for Zigbee Key Establishment
 *
 * A callback to the application to notify it of the status of the request for
 * a Link Key. The status will cover stages from responding to a link key
 * request, potential failure paths, to final link key verification. The link
 * key verification stage is used by ZigbeeHost to indicate the end of
 * successful device addition to the network any notify the
 * onTrustCenterDeviceJoinComplete callback for that particular device.
 *
 * @param partner partner The IEEE address of the partner device.  Or all zeros
 * if the Key establishment failed.  Ver.: always
 * @param status The status of the key establishment.  Ver.: always
 */
void sl_zigbee_af_zigbee_key_establishment_cb(sl_802154_long_addr_t partner,
                                           sl_zigbee_key_status_t status)
{
  if (status == SL_ZIGBEE_TC_REQUESTER_VERIFY_KEY_SUCCESS) {
    bool callbackExists
      = (z3gwState.callbacks != NULL)
        && (z3gwState.callbacks->onTrustCenterDeviceJoinComplete != NULL);
    if (callbackExists == true) {
      z3gwState.callbacks->onTrustCenterDeviceJoinComplete(partner);
    }
  }
}

sl_zigbee_key_data_t zigbeeHostGetTrustCenterWellKownKey(void)
{
  sl_zigbee_key_data_t key = ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY;

  return key;
}

sl_status_t zigbeeHostTrustCenterAddLinkKey(const sl_802154_long_addr_t eui64,
                                            const sl_zigbee_key_data_t *key,
                                            bool inTransientTable)
{
  sl_status_t status = SL_STATUS_OK;

  // Copy to non-const variables
  sl_802154_long_addr_t eui64_cp;
  sl_zigbee_key_data_t key_cp;
  memcpy(eui64_cp, eui64, sizeof(sl_802154_long_addr_t));
  memcpy(&key_cp, key, sizeof(sl_zigbee_key_data_t));

  if (inTransientTable) {
    status = sl_zigbee_sec_man_import_transient_key(eui64_cp, (sl_zigbee_sec_man_key_t *)&key_cp);
  } else {
    status = sl_zigbee_sec_man_import_link_key(0xFF, eui64_cp, (sl_zigbee_sec_man_key_t *)&key_cp);
  }
  return status;
}

bool zigbeeHostTrustCenterInstallCodeValid(const uint8_t *installCode,
                                           uint8_t installCodeLength)
{
  if (installCodeLength > SL_ZIGBEE_INSTALL_CODE_MAX_SIZE) {
    return false;
  }

  sl_zigbee_key_data_t linkKey;
  uint8_t installCodeCp[SL_ZIGBEE_INSTALL_CODE_MAX_SIZE];
  memcpy(installCodeCp, installCode, installCodeLength);

  sl_status_t status
    = sli_zigbee_af_install_code_to_key(installCodeCp, installCodeLength, &linkKey);
  return (status == SL_STATUS_OK);
}

sl_status_t zigbeeHostTrustCenterAddDeviceInstallCode(const sl_802154_long_addr_t eui64,
                                                      uint8_t *installCode,
                                                      uint8_t installCodeLength)
{
  sl_status_t emb_status = SL_STATUS_OK;
  sl_status_t status = SL_STATUS_FAIL;
  sl_zigbee_key_data_t linkKey;

  if ((NULL == installCode) || (NULL == eui64) || (installCodeLength == 0)) {
    emb_status = SL_STATUS_INVALID_PARAMETER;
    status = SL_STATUS_INVALID_PARAMETER;
    appDebugPrint("Error: Failed to add Link Key Pair: Invalid/Bad Arguments\n");
  } else {
    emb_status = sli_zigbee_af_install_code_to_key(installCode, installCodeLength, &linkKey);
    if (emb_status != SL_STATUS_OK) {
      appDebugPrint("Error: Failed to convert Install Code to Link Key: 0x%X\n",
                    emb_status);
    }
  }

  if (emb_status == SL_STATUS_OK) {
    status = zigbeeHostTrustCenterAddLinkKey(eui64, &linkKey, true);
    if (status != SL_STATUS_OK) {
      appDebugPrint("Error: Failed to add Link Key Pair: 0x%X\n", status);
    }
  }

  return status;
}

sl_status_t zigbeeHostTrustCenterJoinOpen(bool openForever)
{
  sl_status_t status = SL_STATUS_OK;

  if (sl_zigbee_af_network_state() != SL_ZIGBEE_JOINED_NETWORK) {
    status = SL_STATUS_NETWORK_DOWN;
    appDebugPrint("Error: Network not up: 0x%X\n", status);
  } else {
    // 0xFF: forever, 0xFE: 254 seconds
    uint8_t permitDuration = (openForever == true) ? 0xFF : 0xFE;
    zaTrustCenterSetJoinPolicy(SL_ZIGBEE_USE_PRECONFIGURED_KEY);
    status = sl_zigbee_af_permit_join(permitDuration, true);
  }

  return status;
}

sl_status_t zigbeeHostTrustCenterJoinClose(void)
{
  sl_status_t status = SL_STATUS_OK;

  if (sl_zigbee_af_network_state() != SL_ZIGBEE_JOINED_NETWORK) {
    status = SL_STATUS_NETWORK_DOWN;
    appDebugPrint("Error: Network not up: 0x%X\n", status);
  } else {
    zaTrustCenterSetJoinPolicy(SL_ZIGBEE_ALLOW_REJOINS_ONLY);
    status = sl_zigbee_af_permit_join(0, true);
  }

  return status;
}

sl_status_t zigbeeHostNetworkDeviceLeaveRequest(const sl_802154_long_addr_t eui64)
{
  if (eui64 == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_802154_short_addr_t nodeId;

  sl_status_t status = zigbeeHostGetAddressTableEntry(eui64, &nodeId);
  if (status != SL_STATUS_OK) {
    appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                  "EUI64 to NodeId",
                  "Device Leave");
    return status;
  }

  sl_802154_long_addr_t eui64Dup;
  memcpy(eui64Dup, eui64, sizeof(sl_802154_long_addr_t));

  uint8_t leaveRequestFlags = 0x00;
  leaveRequestFlags &= ~(LEAVE_REQUEST_REMOVE_CHILDREN_FLAG);
  leaveRequestFlags &= ~(LEAVE_REQUEST_REJOIN_FLAG);

  sl_zigbee_aps_option_t apsOptions
    = (SL_ZIGBEE_APS_OPTION_RETRY | SL_ZIGBEE_APS_OPTION_ENABLE_ROUTE_DISCOVERY
       | SL_ZIGBEE_APS_OPTION_ENABLE_ADDRESS_DISCOVERY);

  // Use the ZDO command to remove the device
  return sl_zigbee_leave_request(nodeId, eui64Dup, leaveRequestFlags, apsOptions);
}

sl_status_t zigbeeHostTrustCenterInit(void)
{
  sl_status_t status = SL_STATUS_OK;

  sl_zigbee_network_status_t networkState = sl_zigbee_af_network_state();
  if (networkState == SL_ZIGBEE_NO_NETWORK) {
    status = sl_zigbee_af_network_creator_start(true);
    if (status != SL_STATUS_OK) {
      appDebugPrint("Error: Failed to create trust center network: 0x%X\n",
                    status);
    }
  }

  return status;
}

sl_status_t zigbeeHostTrustCenterInitWithArgs(
        uint16_t pan_id,
        int8_t radio_tx_power,
        uint8_t channel)
{
  sl_status_t status = SL_STATUS_OK;

  sl_zigbee_network_status_t networkState = sl_zigbee_af_network_state();
  if (networkState == SL_ZIGBEE_NO_NETWORK) {

    status = sl_zigbee_af_network_creator_network_form(
                true,
                pan_id,
                radio_tx_power,
                channel);

    if (status != SL_STATUS_OK) {
      appDebugPrint("Error: Failed to create trust center network: 0x%X\n",
                    status);
    }
  }

  return status;
}

/*****************************************************************************/
/** Trust Center callbacks                                                  **/
/*****************************************************************************/

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
void sl_zigbee_af_stack_status_cb(sl_status_t status)
{
  bool callbackExists
    = ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks, onNetworkInitialized);
  if (callbackExists == false) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK, "Trust Center Initialize");
    return;
  }

  if (status != SL_STATUS_NETWORK_UP) {
    return;
  }

  sl_zigbee_node_type_t nodeType;
  sl_zigbee_network_parameters_t network;
  sl_status_t paramStatus = sl_zigbee_af_get_network_parameters(&nodeType, &network);
  if (paramStatus != SL_STATUS_OK) {
    appDebugPrint(
      "Error: Failed to get trust center network parameters: 0x%X\n",
      status);
    return;
  }

  z3gwState.callbacks->onNetworkInitialized(&network);
}

void emAfPluginGatewayInterfaceTrustCenterJoinHandler(
  const sl_802154_long_addr_t eui64, sl_802154_short_addr_t nodeId, sl_zigbee_join_decision_t decision)
{
  if (decision == SL_ZIGBEE_DENY_JOIN) {
    appDebugPrint("Ignoring network deny join event\n");
    return;
  }

  bool callbackExists
    = ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks, onNetworkDeviceJoin);
  if (callbackExists == false) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK, "Network device announce");
    return;
  }

  /*
   * Start tracking EUI64 in address-table.
   */
  zigbeeHostAddAddressTableEntry(eui64, nodeId);

  z3gwState.callbacks->onNetworkDeviceJoin(eui64);
}

void emAfPluginGatewayInterfaceTrustCenterLeaveHandler(const sl_802154_long_addr_t eui64)
{
  bool callbackExists
    = ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                  onNetworkDeviceLeaveResponse);
  if (callbackExists == false) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK, "Network device leave");
    return;
  }

  z3gwState.callbacks->onNetworkDeviceLeaveResponse(eui64);
}

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of sl_zigbee_internal_trust_center_join_handler or
 * sl_zigbee_ezsp_trust_center_post_join_handler. This callback provides 
 * the same arguments passed to the TrustCenterJoinHandler. For more 
 * information about the TrustCenterJoinHandler please see documentation 
 * included in stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void sl_zigbee_af_trust_center_join_cb(sl_802154_short_addr_t newNodeId,
                                    sl_802154_long_addr_t newNodeEui64,
                                    sl_802154_short_addr_t parentOfNewNode,
                                    sl_zigbee_device_update_t status,
                                    sl_zigbee_join_decision_t decision)
{
  switch (status) {
    case SL_ZIGBEE_DEVICE_LEFT:
      emAfPluginGatewayInterfaceTrustCenterLeaveHandler(newNodeEui64);
      break;
    case SL_ZIGBEE_STANDARD_SECURITY_SECURED_REJOIN:
      // fall-through
    case SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_REJOIN:
      // fall-through
    case SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_JOIN:
      emAfPluginGatewayInterfaceTrustCenterJoinHandler(newNodeEui64,
                                                       newNodeId,
                                                       decision);
      break;
    default:
      appDebugPrint("Unknown Trust Center event status: 0x%02X\n", status);
      break;
  }
}
