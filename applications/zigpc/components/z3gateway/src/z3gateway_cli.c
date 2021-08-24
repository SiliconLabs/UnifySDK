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

#include PLATFORM_HEADER
#include EMBER_AF_API_AF_HEADER
#include EMBER_AF_API_AF_MAIN_HEADER
#include EMBER_AF_API_AF_TYPES_HEADER
#include EMBER_AF_API_AF_MAIN_HOST_HEADER
#include EMBER_AF_API_ATTRIBUTE_STORAGE_HEADER
#include EMBER_AF_API_UTIL_HEADER

/* external includes */
#include "z3gateway.h"

/* internal includes */
#include "z3gateway_common.h"

void z3gatewayCommandPrintInfo(void)
{
  /**
   * NOTE: The following logic was duplicated from emAfCliInfoCommand()
   * under app/framework/cli/core-cli.c.
   *
   */

  EmberNodeType nodeTypeResult = 0xFF;
  EmberEUI64 myEui64;
  EmberNetworkParameters networkParams = {0};
  uint8_t numPhyInterfaces;

  uint8_t mfgString[MFG_STRING_MAX_LENGTH + 1];

  // NOTE: Disabled in ZigPC
  // emberStringCommandArgument(-1, &commandLength);

  // NOTE: Replaced in ZigPC by content inside printMfgString
  // printMfgString();
  emberAfFormatMfgString(mfgString);
  // Note: We use '%s' here because this is a RAM string. Normally,
  // most strings are literals or constants in flash and use '%p'.
  emberAfAppPrintln("MFG String: %s", mfgString);

  emberAfAppPrintln("AppBuilder MFG Code: 0x%2X", EMBER_AF_MANUFACTURER_CODE);
  emberAfGetEui64(myEui64);
  emberAfGetNetworkParameters(&nodeTypeResult, &networkParams);
  emberAfAppPrint("node [");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(myEui64));
  emberAfAppFlush();
  emberAfAppPrintln("] chan [%d] pwr [%d]",
                    networkParams.radioChannel,
                    networkParams.radioTxPower);

  numPhyInterfaces = emberGetPhyInterfaceCount();

  if ((numPhyInterfaces > 1) && (255 != numPhyInterfaces)) {
    EmberMultiPhyRadioParameters multiPhyRadioParams;
    uint8_t i;
    EmberStatus status;

    emberAfAppPrintln("Additional interfaces");
    for (i = 1; i < numPhyInterfaces; ++i) {
      emberAfAppPrint("  %d: ", i);
      status = emberGetRadioParameters(i, &multiPhyRadioParams);
      switch (status) {
        case EMBER_SUCCESS:
          emberAfAppPrintln("page [%d] chan [%d] pwr [%d]",
                            multiPhyRadioParams.radioPage,
                            multiPhyRadioParams.radioChannel,
                            multiPhyRadioParams.radioTxPower);
          break;
        case EMBER_NETWORK_DOWN:
        case EMBER_NOT_JOINED:
          emberAfAppPrintln("not active");
          break;
        default:
          emberAfAppPrintln("error 0x%x", status);
          break;
      }
    }
  }

  emberAfAppPrint("panID [0x%2x] nodeID [0x%2x] ",
                  networkParams.panId,
                  emberAfGetNodeId());
  emberAfAppFlush();
  emberAfAppPrint("xpan [0x");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(networkParams.extendedPanId));
  emberAfAppPrintln("]");
  emberAfAppFlush();

#ifndef EZSP_HOST
  emberAfAppPrintln("parentID [0x%2x] parentRssi [%d]",
                    emberGetParentId(),
                    emberGetAvgParentRssi());
  emberAfAppFlush();
#endif  // EZSP_HOST

  emAfCliVersionCommand();
  emberAfAppFlush();

  emberAfAppPrint("nodeType [");
  if (nodeTypeResult != 0xFF) {
    emberAfAppPrint("0x%x", nodeTypeResult);
  } else {
    emberAfAppPrint("unknown");
  }
  emberAfAppPrintln("]");
  emberAfAppFlush();

  emberAfAppPrintln("%p level [%x]", "Security", emberAfGetSecurityLevel());

  // NOTE: Disabled in ZigPC
  // printSmartEnergySecurityInfo();
  // printSmartEnergySecurityInfo283k1();

  emberAfAppPrint("network state [%x] ", emberNetworkState());

  // NOTE: Replaced in ZigPC by content of function
  // printPacketBuffers();
  emberAfAppPrintln("Buffs: %d / %d",
                    emAfGetPacketBufferFreeCount(),
                    emAfGetPacketBufferTotalCount());

  emberAfAppFlush();

  // EMZIGBEE-5125: apps with lots of endpoints will wdog while printing
  halResetWatchdog();
  // Print the endpoint information.
  {
    uint8_t i, j;
    emberAfAppPrintln("Ep cnt: %d", emberAfEndpointCount());
    // Loop for each endpoint.
    for (i = 0; i < emberAfEndpointCount(); i++) {
      EmberAfEndpointType *et = emAfEndpoints[i].endpointType;
      emberAfAppPrint(
        "ep %d [endpoint %p, device %p] ",
        emberAfEndpointFromIndex(i),
        (emberAfEndpointIndexIsEnabled(i) ? "enabled" : "disabled"),
        emberAfIsDeviceEnabled(emberAfEndpointFromIndex(i)) ? "enabled"
                                                            : "disabled");
      emberAfAppPrintln("nwk [%d] profile [0x%2x] devId [0x%2x] ver [0x%x]",
                        emberAfNetworkIndexFromEndpointIndex(i),
                        emberAfProfileIdFromIndex(i),
                        emberAfDeviceIdFromIndex(i),
                        emberAfDeviceVersionFromIndex(i));
      // Loop for the clusters within the endpoint.
      for (j = 0; j < et->clusterCount; j++) {
        EmberAfCluster *zc = &(et->cluster[j]);
        emberAfAppPrint(
          "    %p cluster: 0x%2x ",
          (emberAfClusterIsClient(zc) ? "out(client)" : "in (server)"),
          zc->clusterId);
        emberAfAppDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(
          zc->clusterId,
          emAfGetManufacturerCodeForCluster(zc)));
        emberAfAppPrintln("");
        emberAfAppFlush();
      }
      emberAfAppFlush();
      // EMZIGBEE-5125
      halResetWatchdog();
    }
    // EMZIGBEE-5125
    halResetWatchdog();
  }

  {
    const char *names[] = {EM_AF_GENERATED_NETWORK_STRINGS};
    uint8_t i;
    emberAfAppPrintln("Nwk cnt: %d", EMBER_SUPPORTED_NETWORKS);
    for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++) {
      (void)emberAfPushNetworkIndex(i);
      emberAfAppPrintln("nwk %d [%p]", i, names[i]);
      if (emAfProIsCurrentNetwork()) {
        emberAfAppPrintln("  nodeType [0x%x]",
                          emAfCurrentZigbeeProNetwork->nodeType);
        emberAfAppPrintln("  securityProfile [0x%x]",
                          emAfCurrentZigbeeProNetwork->securityProfile);
      }
      (void)emberAfPopNetworkIndex();
    }
  }
}
