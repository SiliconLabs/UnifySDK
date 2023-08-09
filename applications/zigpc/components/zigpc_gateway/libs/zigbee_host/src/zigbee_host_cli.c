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
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/include/af-types.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/util/util.h"
#include "app/framework/security/security-config.h"

/* external includes */
#include "zigbee_host.h"

/* internal includes */
#include "zigbee_host_common.h"

void zigbeeHostCommandPrintInfo(void)
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
  appDebugPrint("MFG String: %s\n", mfgString);

  appDebugPrint("AppBuilder MFG Code: 0x%2X\n", EMBER_AF_MANUFACTURER_CODE);
  zigbeeHostGetEui64(myEui64);
  emberAfGetNetworkParameters(&nodeTypeResult, &networkParams);
  appDebugPrint("node [");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(myEui64));
  emberAfAppFlush();
  appDebugPrint("] chan [%d] pwr [%d]\n",
                networkParams.radioChannel,
                networkParams.radioTxPower);

  numPhyInterfaces = emberGetPhyInterfaceCount();

  if ((numPhyInterfaces > 1) && (255 != numPhyInterfaces)) {
    EmberMultiPhyRadioParameters multiPhyRadioParams;
    uint8_t i;
    EmberStatus status;

    appDebugPrint("Additional interfaces\n");
    for (i = 1; i < numPhyInterfaces; ++i) {
      appDebugPrint("  %d: ", i);
      status = emberGetRadioParameters(i, &multiPhyRadioParams);
      switch (status) {
        case EMBER_SUCCESS:
          appDebugPrint("page [%d] chan [%d] pwr [%d]\n",
                        multiPhyRadioParams.radioPage,
                        multiPhyRadioParams.radioChannel,
                        multiPhyRadioParams.radioTxPower);
          break;
        case EMBER_NETWORK_DOWN:
        case EMBER_NOT_JOINED:
          appDebugPrint("not active\n");
          break;
        default:
          appDebugPrint("error 0x%x\n", status);
          break;
      }
    }
  }

  appDebugPrint("panID [0x%2x] nodeID [0x%2x] ",
                networkParams.panId,
                emberAfGetNodeId());
  emberAfAppFlush();
  appDebugPrint("xpan [0x");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(networkParams.extendedPanId));
  appDebugPrint("]\n");
  emberAfAppFlush();

#ifndef EZSP_HOST
  appDebugPrint("parentID [0x%2x] parentRssi [%d]\n",
                emberGetParentId(),
                emberGetAvgParentRssi());
  emberAfAppFlush();
#endif  // EZSP_HOST

  sli_zigbee_af_cli_version_command();
  emberAfAppFlush();

  appDebugPrint("nodeType [");
  if (nodeTypeResult != 0xFF) {
    appDebugPrint("0x%x", nodeTypeResult);
  } else {
    appDebugPrint("unknown");
  }
  appDebugPrint("]\n");
  emberAfAppFlush();

  appDebugPrint("%p level [%x]\n", "Security", emberAfGetSecurityLevel());

  // NOTE: Disabled in ZigPC
  // printSmartEnergySecurityInfo();
  // printSmartEnergySecurityInfo283k1();

  appDebugPrint("network state [%x] ", emberNetworkState());

  // NOTE: Replaced in ZigPC by content of function
  // printPacketBuffers();
  appDebugPrint("Buffs: %d / %d\n",
                sli_zigbee_af_get_packet_buffer_free_count(),
                sli_zigbee_af_get_packet_buffer_total_count());

  emberAfAppFlush();

  // EMZIGBEE-5125: apps with lots of endpoints will wdog while printing
  halResetWatchdog();
  // Print the endpoint information.
  {
    uint8_t i, j;
    appDebugPrint("Ep cnt: %d\n", emberAfEndpointCount());
    // Loop for each endpoint.
    for (i = 0; i < emberAfEndpointCount(); i++) {
      EmberAfEndpointType *et = sli_zigbee_af_endpoints[i].endpointType;
      appDebugPrint("ep %d [endpoint %p, device %p] ",
                    emberAfEndpointFromIndex(i),
                    (emberAfEndpointIndexIsEnabled(i) ? "enabled" : "disabled"),
                    emberAfIsDeviceEnabled(emberAfEndpointFromIndex(i))
                      ? "enabled"
                      : "disabled");
      appDebugPrint("nwk [%d] profile [0x%2x] devId [0x%2x] ver [0x%x]\n",
                    emberAfNetworkIndexFromEndpointIndex(i),
                    emberAfProfileIdFromIndex(i),
                    emberAfDeviceIdFromIndex(i),
                    emberAfDeviceVersionFromIndex(i));
      // Loop for the clusters within the endpoint.
      for (j = 0; j < et->clusterCount; j++) {
        EmberAfCluster *zc = &(et->cluster[j]);
        appDebugPrint(
          "    %p cluster: 0x%2x ",
          (emberAfClusterIsClient(zc) ? "out(client)" : "in (server)"),
          zc->clusterId);
        emberAfAppDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(
          zc->clusterId,
          sli_zigbee_af_get_manufacturer_code_for_cluster(zc)));
        appDebugPrint("\n");
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
    const char *names[] = {EMBER_AF_GENERATED_NETWORK_STRINGS};
    uint8_t i;
    appDebugPrint("Nwk cnt: %d\n", EMBER_SUPPORTED_NETWORKS);
    for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++) {
      (void)emberAfPushNetworkIndex(i);
      appDebugPrint("nwk %d [%p]\n", i, names[i]);
      if (sli_zigbee_af_pro_is_current_network()) {
        appDebugPrint("  nodeType [0x%x]\n",
                      sli_zigbee_af_current_zigbee_pro_network->nodeType);
        appDebugPrint("  securityProfile [0x%x]\n",
                      sli_zigbee_af_current_zigbee_pro_network->securityProfile);
      }
      (void)emberAfPopNetworkIndex();
    }
  }
}
