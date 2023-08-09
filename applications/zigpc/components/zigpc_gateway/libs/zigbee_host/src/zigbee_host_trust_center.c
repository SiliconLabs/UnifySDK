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

#define EMBER_INSTALL_CODE_MAX_SIZE (16 + EMBER_INSTALL_CODE_CRC_SIZE)

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
void emberAfZigbeeKeyEstablishmentCallback(EmberEUI64 partner,
                                           EmberKeyStatus status)
{
  if (status == EMBER_TC_REQUESTER_VERIFY_KEY_SUCCESS) {
    bool callbackExists
      = (z3gwState.callbacks != NULL)
        && (z3gwState.callbacks->onTrustCenterDeviceJoinComplete != NULL);
    if (callbackExists == true) {
      z3gwState.callbacks->onTrustCenterDeviceJoinComplete(partner);
    }
  }
}

EmberKeyData zigbeeHostGetTrustCenterWellKownKey(void)
{
  EmberKeyData key = ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY;

  return key;
}

EmberStatus zigbeeHostTrustCenterAddLinkKey(const EmberEUI64 eui64,
                                            const EmberKeyData *key,
                                            bool inTransientTable)
{
  EmberStatus status = EMBER_SUCCESS;

  // Copy to non-const variables
  EmberEUI64 eui64_cp;
  EmberKeyData key_cp;
  memcpy(eui64_cp, eui64, sizeof(EmberEUI64));
  memcpy(&key_cp, key, sizeof(EmberKeyData));

  if (inTransientTable) {
    status = emberAddTransientLinkKey(eui64_cp, &key_cp);
  } else {
    status = emberAddOrUpdateKeyTableEntry(eui64_cp, true, &key_cp);
  }
  return status;
}

bool zigbeeHostTrustCenterInstallCodeValid(const uint8_t *installCode,
                                           uint8_t installCodeLength)
{
  if (installCodeLength > EMBER_INSTALL_CODE_MAX_SIZE) {
    return false;
  }

  EmberKeyData linkKey;
  uint8_t installCodeCp[EMBER_INSTALL_CODE_MAX_SIZE];
  memcpy(installCodeCp, installCode, installCodeLength);

  EmberStatus status
    = sli_zigbee_af_install_code_to_key(installCodeCp, installCodeLength, &linkKey);
  return (status == EMBER_SUCCESS);
}

EmberStatus zigbeeHostTrustCenterAddDeviceInstallCode(const EmberEUI64 eui64,
                                                      uint8_t *installCode,
                                                      uint8_t installCodeLength)
{
  EmberStatus status = EMBER_SUCCESS;
  EmberKeyData linkKey;

  if ((NULL == installCode) || (NULL == eui64) || (installCodeLength == 0)) {
    status = EMBER_BAD_ARGUMENT;
  } else {
    status = sli_zigbee_af_install_code_to_key(installCode, installCodeLength, &linkKey);
    if (status != EMBER_SUCCESS) {
      appDebugPrint("Error: Failed to convert Install Code to Link Key: 0x%X\n",
                    status);
    }
  }

  if (status == EMBER_SUCCESS) {
    status = zigbeeHostTrustCenterAddLinkKey(eui64, &linkKey, true);
    if (status != EMBER_SUCCESS) {
      appDebugPrint("Error: Failed to add Link Key Pair: 0x%X\n", status);
    }
  }

  return status;
}

EmberStatus zigbeeHostTrustCenterJoinOpen(bool openForever)
{
  EmberStatus status = EMBER_SUCCESS;

  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
    status = EMBER_NETWORK_DOWN;
    appDebugPrint("Error: Network not up: 0x%X\n", status);
  } else {
    // 0xFF: forever, 0xFE: 254 seconds
    uint8_t permitDuration = (openForever == true) ? 0xFF : 0xFE;
    zaTrustCenterSetJoinPolicy(EMBER_USE_PRECONFIGURED_KEY);
    status = emberAfPermitJoin(permitDuration, true);
  }

  return status;
}

EmberStatus zigbeeHostTrustCenterJoinClose(void)
{
  EmberStatus status = EMBER_SUCCESS;

  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
    status = EMBER_NETWORK_DOWN;
    appDebugPrint("Error: Network not up: 0x%X\n", status);
  } else {
    zaTrustCenterSetJoinPolicy(EMBER_ALLOW_REJOINS_ONLY);
    status = emberAfPermitJoin(0, true);
  }

  return status;
}

EmberStatus zigbeeHostNetworkDeviceLeaveRequest(const EmberEUI64 eui64)
{
  if (eui64 == NULL) {
    return EMBER_BAD_ARGUMENT;
  }

  EmberNodeId nodeId;

  EmberStatus status = zigbeeHostGetAddressTableEntry(eui64, &nodeId);
  if (status != EMBER_SUCCESS) {
    appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                  "EUI64 to NodeId",
                  "Device Leave");
    return status;
  }

  EmberEUI64 eui64Dup;
  memcpy(eui64Dup, eui64, sizeof(EmberEUI64));

  uint8_t leaveRequestFlags = 0x00;
  leaveRequestFlags &= ~(LEAVE_REQUEST_REMOVE_CHILDREN_FLAG);
  leaveRequestFlags &= ~(LEAVE_REQUEST_REJOIN_FLAG);

  EmberApsOption apsOptions
    = (EMBER_APS_OPTION_RETRY | EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY
       | EMBER_APS_OPTION_ENABLE_ADDRESS_DISCOVERY);

  // Use the ZDO command to remove the device
  return emberLeaveRequest(nodeId, eui64Dup, leaveRequestFlags, apsOptions);
}

EmberStatus zigbeeHostTrustCenterInit(void)
{
  EmberStatus status = EMBER_SUCCESS;

  EmberNetworkStatus networkState = emberAfNetworkState();
  if (networkState == EMBER_NO_NETWORK) {
    status = emberAfPluginNetworkCreatorStart(true);
    if (status != EMBER_SUCCESS) {
      appDebugPrint("Error: Failed to create trust center network: 0x%X\n",
                    status);
    }
  }

  return status;
}

EmberStatus zigbeeHostTrustCenterInitWithArgs(
        uint16_t pan_id,
        int8_t radio_tx_power,
        uint8_t channel)
{
  EmberStatus status = EMBER_SUCCESS;

  EmberNetworkStatus networkState = emberAfNetworkState();
  if (networkState == EMBER_NO_NETWORK) {

    status = emberAfPluginNetworkCreatorNetworkForm(
                true,
                pan_id,
                radio_tx_power,
                channel);

    if (status != EMBER_SUCCESS) {
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
void emberAfStackStatusCallback(EmberStatus status)
{
  bool callbackExists
    = ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks, onNetworkInitialized);
  if (callbackExists == false) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK, "Trust Center Initialize");
    return;
  }

  if (status != EMBER_NETWORK_UP) {
    return;
  }

  EmberNodeType nodeType;
  EmberNetworkParameters network;
  EmberStatus paramStatus = emberAfGetNetworkParameters(&nodeType, &network);
  if (paramStatus != EMBER_SUCCESS) {
    appDebugPrint(
      "Error: Failed to get trust center network parameters: 0x%X\n",
      status);
    return;
  }

  z3gwState.callbacks->onNetworkInitialized(&network);
}

void emAfPluginGatewayInterfaceTrustCenterJoinHandler(
  const EmberEUI64 eui64, EmberNodeId nodeId, EmberJoinDecision decision)
{
  if (decision == EMBER_DENY_JOIN) {
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

void emAfPluginGatewayInterfaceTrustCenterLeaveHandler(const EmberEUI64 eui64)
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
 * implementation of emberTrustCenterJoinHandler or
 * ezspTrustCenterJoinHandler. This callback provides the same arguments
 * passed to the TrustCenterJoinHandler. For more information about the
 * TrustCenterJoinHandler please see documentation included in
 * stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
  switch (status) {
    case EMBER_DEVICE_LEFT:
      emAfPluginGatewayInterfaceTrustCenterLeaveHandler(newNodeEui64);
      break;
    case EMBER_STANDARD_SECURITY_SECURED_REJOIN:
      // fall-through
    case EMBER_STANDARD_SECURITY_UNSECURED_REJOIN:
      // fall-through
    case EMBER_STANDARD_SECURITY_UNSECURED_JOIN:
      emAfPluginGatewayInterfaceTrustCenterJoinHandler(newNodeEui64,
                                                       newNodeId,
                                                       decision);
      break;
    default:
      appDebugPrint("Unknown Trust Center event status: 0x%02X\n", status);
      break;
  }
}
