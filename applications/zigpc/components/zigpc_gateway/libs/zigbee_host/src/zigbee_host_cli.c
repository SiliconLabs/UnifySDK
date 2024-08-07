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
   * NOTE: The following logic was duplicated from sli_zigbee_af_cli_info_command()
   * under app/framework/cli/core-cli.c.
   *
   */

  sl_zigbee_node_type_t nodeTypeResult = 0xFF;
  sl_802154_long_addr_t myEui64;
  sl_zigbee_network_parameters_t networkParams = {0};
  uint8_t numPhyInterfaces;

  uint8_t mfgString[MFG_STRING_MAX_LENGTH + 1];

  // NOTE: Disabled in ZigPC
  // emberStringCommandArgument(-1, &commandLength);

  // NOTE: Replaced in ZigPC by content inside printMfgString
  // printMfgString();
  sl_zigbee_af_format_mfg_string(mfgString);
  // Note: We use '%s' here because this is a RAM string. Normally,
  // most strings are literals or constants in flash and use '%p'.
  appDebugPrint("MFG String: %s\n", mfgString);

  appDebugPrint("AppBuilder MFG Code: 0x%2X\n", SL_ZIGBEE_ZCL_MANUFACTURER_CODE);
  zigbeeHostGetEui64(myEui64);
  sl_zigbee_af_get_network_parameters(&nodeTypeResult, &networkParams);
  appDebugPrint("node [");
  sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_big_endian_eui64(myEui64));
  sl_zigbee_af_app_flush();
  appDebugPrint("] chan [%d] pwr [%d]\n",
                networkParams.radioChannel,
                networkParams.radioTxPower);

  numPhyInterfaces = sl_zigbee_get_phy_interface_count();

  if ((numPhyInterfaces > 1) && (255 != numPhyInterfaces)) {
    sl_zigbee_multi_phy_radio_parameters_t multiPhyRadioParams;
    uint8_t i;
    sl_status_t status;

    appDebugPrint("Additional interfaces\n");
    for (i = 1; i < numPhyInterfaces; ++i) {
      appDebugPrint("  %d: ", i);
      status = sl_zigbee_get_radio_parameters(i, &multiPhyRadioParams);
      switch (status) {
        case SL_STATUS_OK:
          appDebugPrint("page [%d] chan [%d] pwr [%d]\n",
                        multiPhyRadioParams.radioPage,
                        multiPhyRadioParams.radioChannel,
                        multiPhyRadioParams.radioTxPower);
          break;
        case SL_STATUS_NETWORK_DOWN:
        case SL_STATUS_NOT_JOINED:
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
                sl_zigbee_af_get_node_id());
  sl_zigbee_af_app_flush();
  appDebugPrint("xpan [0x");
  sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_big_endian_eui64(networkParams.extendedPanId));
  appDebugPrint("]\n");
  sl_zigbee_af_app_flush();

#ifndef EZSP_HOST
  appDebugPrint("parentID [0x%2x] parentRssi [%d]\n",
                sl_zigbee_get_parent_id(),
                sl_zigbee_get_avg_parent_rssi());
  sl_zigbee_af_app_flush();
#endif  // EZSP_HOST

  sli_zigbee_af_cli_version_command();
  sl_zigbee_af_app_flush();

  appDebugPrint("nodeType [");
  if (nodeTypeResult != 0xFF) {
    appDebugPrint("0x%x", nodeTypeResult);
  } else {
    appDebugPrint("unknown");
  }
  appDebugPrint("]\n");
  sl_zigbee_af_app_flush();

  appDebugPrint("%p level [%x]\n", "Security", sl_zigbee_af_get_security_level());

  // NOTE: Disabled in ZigPC
  // printSmartEnergySecurityInfo();
  // printSmartEnergySecurityInfo283k1();

  appDebugPrint("network state [%x] ", sl_zigbee_network_state());

  // NOTE: Replaced in ZigPC by content of function
  // printPacketBuffers();
  appDebugPrint("Buffs: %d / %d\n",
                sli_zigbee_af_get_packet_buffer_free_space(),
                sli_zigbee_af_get_packet_buffer_total_space());

  sl_zigbee_af_app_flush();

  // EMZIGBEE-5125: apps with lots of endpoints will wdog while printing
  halResetWatchdog();
  // Print the endpoint information.
  {
    uint8_t i, j;
    appDebugPrint("Ep cnt: %d\n", sl_zigbee_af_endpoint_count());
    // Loop for each endpoint.
    for (i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
      sl_zigbee_af_endpoint_type_t *et = sli_zigbee_af_endpoints[i].endpointType;
      appDebugPrint("ep %d [endpoint %p, device %p] ",
                    sl_zigbee_af_endpoint_from_index(i),
                    (sl_zigbee_af_endpoint_index_is_enabled(i) ? "enabled" : "disabled"),
                    sl_zigbee_af_is_device_enabled(sl_zigbee_af_endpoint_from_index(i))
                      ? "enabled"
                      : "disabled");
      appDebugPrint("nwk [%d] profile [0x%2x] devId [0x%2x] ver [0x%x]\n",
                    sl_zigbee_af_network_index_from_endpoint_index(i),
                    sl_zigbee_af_profile_id_from_index(i),
                    sl_zigbee_af_device_id_from_index(i),
                    sl_zigbee_af_device_version_from_index(i));
      // Loop for the clusters within the endpoint.
      for (j = 0; j < et->clusterCount; j++) {
        sl_zigbee_af_cluster_t *zc = &(et->cluster[j]);
        appDebugPrint(
          "    %p cluster: 0x%2x ",
          (sl_zigbee_af_cluster_is_client(zc) ? "out(client)" : "in (server)"),
          zc->clusterId);
        sl_zigbee_af_app_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(
          zc->clusterId,
          sli_zigbee_af_get_manufacturer_code_for_cluster(zc)));
        appDebugPrint("\n");
        sl_zigbee_af_app_flush();
      }
      sl_zigbee_af_app_flush();
      // EMZIGBEE-5125
      halResetWatchdog();
    }
    // EMZIGBEE-5125
    halResetWatchdog();
  }

  {
    const char *names[] = {SL_ZIGBEE_AF_GENERATED_NETWORK_STRINGS };
    uint8_t i;
    appDebugPrint("Nwk cnt: %d\n", SL_ZIGBEE_SUPPORTED_NETWORKS );
    for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS ; i++) {
      (void)sl_zigbee_af_push_network_index(i);
      appDebugPrint("nwk %d [%p]\n", i, names[i]);
      if (sli_zigbee_af_pro_is_current_network()) {
        appDebugPrint("  nodeType [0x%x]\n",
                      sli_zigbee_af_current_zigbee_pro_network->nodeType);
        appDebugPrint("  securityProfile [0x%x]\n",
                      sli_zigbee_af_current_zigbee_pro_network->securityProfile);
      }
      (void)sl_zigbee_af_pop_network_index();
    }
  }
}
