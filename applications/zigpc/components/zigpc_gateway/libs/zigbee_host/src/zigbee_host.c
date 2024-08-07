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

#if defined(SL_CATALOG_KERNEL_PRESENT)
#error "project cannot be compiled with RTOS runtimes"
#endif

// project includes
#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "sl_system_process_action.h"

// emberznet includes
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/ezsp-host/ezsp-host-ui.h"
#include "app/util/serial/linux-serial.h"

// public includes
#include "zigbee_host.h"
#include "zigbee_host_common.h"

#define ZIGBEE_HOST_EMBERAF_ARGV_LEN 5
#define ZIGBEE_HOST_EMBERAF_ARG0     "zigbeeHost"

struct zigbeeHostState z3gwState;

int zigbeeHostInit(struct zigbeeHostOpts *opts)
{
  int argc = 1;
  char *argv[ZIGBEE_HOST_EMBERAF_ARGV_LEN];

  if (opts == NULL) {
    appDebugPrint("Error: Gateway options not found\n");
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (opts->callbacks == NULL) {
    appDebugPrint("Error: Gateway callbacks not found\n");
    return SL_STATUS_INVALID_PARAMETER;
  }
  z3gwState.callbacks = opts->callbacks;

  /* build stdin arguments passed to Ember application framework */
  argv[0] = ZIGBEE_HOST_EMBERAF_ARG0;
  if (opts->serialPort != NULL) {
    argv[1] = "-p";
    argv[2] = opts->serialPort;
    argc += 2;
  }

  if (opts->otaPath != NULL) {
    argv[3] = "-d";
    argv[4] = opts->otaPath;
    argc += 2;
  }

  // This will process EZSP command-line options
  if (!sl_zigbee_ezsp_process_command_options(argc, argv)) {
    return SL_STATUS_FAIL;
  }

  // NOTE: Gateway backchannel is not used/enabled

  sl_status_t status =
    zigbeeHostRegisterClusters(
        opts->supportedClusterList,
        opts->supportedClusterListSize);

  if(SL_STATUS_OK != status)
  {
    return SL_STATUS_FAIL;
  }
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // Note that if the kernel is present, processing task(s) will be created by
  // this call.
  sl_system_init();

  return SL_STATUS_OK;
}

void zigbeeHostTick(void)
{
  // Do not remove this call: Silicon Labs components process action routine
  // must be called from the super loop.
  sl_system_process_action();
}

void zigbeeHostShutdown(void)
{
  sli_legacy_serial_cleanup();
}

sl_status_t zigbeeHostGetEmberKey(sl_zigbee_key_type_t type, sl_zigbee_key_struct_t *const key)
{
  sl_status_t status = SL_STATUS_OK;
  if (key == NULL) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    sl_zigbee_sec_man_context_t ctxt = {};
    sl_zigbee_sec_man_init_context(&ctxt);

    // Currently this API is called only for current network key type.
    UNUSED_VAR(type);
    ctxt.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_NETWORK;

    status = sl_zigbee_sec_man_export_key(&ctxt, (sl_zigbee_sec_man_key_t *)&key->key);
  }
  return status;
}

sl_zigbee_ezsp_status_t zigbeeHostSetEzspPolicy(sl_zigbee_ezsp_policy_id_t policyId,
                                   sl_zigbee_ezsp_decision_id_t decisionId,
                                   const char *policyName,
                                   const char *decisionName)
{
  return sl_zigbee_af_set_ezsp_policy(policyId, decisionId, policyName, decisionName);
}

sl_status_t zigbeeHostRegisterClusters(
              const uint16_t *cluster_list,
              unsigned int cluster_list_size )
{

  sl_status_t status = SL_STATUS_OK;

  if(cluster_list != NULL)
  {
    for(unsigned int i =0; i < cluster_list_size; i++)
    {
      uint16_t cluster_id = cluster_list[i];
      (void)sl_zigbee_subscribe_to_zcl_commands(
              cluster_id,
              0xFFFF,
              ZCL_DIRECTION_CLIENT_TO_SERVER,
              &emberAfClusterServiceCallback);

      (void)sl_zigbee_subscribe_to_zcl_commands(
              cluster_id,
              0xFFFF,
              ZCL_DIRECTION_SERVER_TO_CLIENT,
              &emberAfClusterServiceCallback);
    }
  }
  else
  {
    status = SL_STATUS_INVALID_PARAMETER;
  }

  return status;
}

/*****************************************************************************/
/** EmberAf callbacks                                                       **/
/*****************************************************************************/

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup. Any
 * code that you would normally put into the top of the application's main()
 * routine should be put into this function. This is called before the clusters,
 * plugins, and the network are initialized so some functionality is not yet
 * available.
        Note: No callback in the Application Framework is
 * associated with resource cleanup. If you are implementing your application on
 * a Unix host where resource cleanup is a consideration, we expect that you
 * will use the standard Posix system calls, including the use of atexit() and
 * handlers for signals such as SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If
 * you use the signal() function to register your signal handler, please mind
 * the returned value which may be an Application Framework function. If the
 * return value is non-null, please make sure that you call the returned
 * function from your handler to avoid negating the resource cleanup of the
 * Application Framework itself.
 *
 */
void sl_zigbee_af_main_init_cb(void)
{
  bool callbackExists = ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                                    onEmberAfStackInitalized);
  if (callbackExists) {
    z3gwState.callbacks->onEmberAfStackInitalized();
  }
}

/** @brief Ncp Init
 *
 * This function is always called when the network coprocessor is being
 * initialized, either at startup or upon reset.  It provides applications
 * an opportunity to perform additional configuration of the NCP. For
 * example, tables on the NCP can be resized and populated. This callback
 * issue EZSP commands to configure memory layout on the ncp and is not
 * called on SoCs.
 * 
 */
void emberAfNcpInitCallback()
{
  bool callbackExists
    = ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks, onEmberAfNcpPostReset);
  if (callbackExists) {
    z3gwState.callbacks->onEmberAfNcpPostReset();
  }
}

/** @brief EZSP Error Handler
 *
 * This callback is fired when the host process receives an error from the EZSP
 * link when talking to the NCP. The return boolean gives the user application
 * the option to reboot the NCP. If this function returns true, the NCP will be
 * rebooted and the connection between the host and NCP will drop. If not, the
 * NCP will continue operating.
 *
 * @param status The sl_zigbee_ezsp_status_t error code received.
 *
 * @return bool True to reset NCP, false not to.
 *
 * @note This callback is only fired on the host application. It has no use for
 * SoC or NCP applications.
 */
bool sl_zigbee_af_zcl_framework_core_ezsp_error_cb(sl_zigbee_ezsp_status_t status)
{
#if defined EZSP_HOST
  if (status == SL_ZIGBEE_EZSP_ERROR_OVERFLOW) {
    appDebugPrint("WARNING: the NCP has run out of buffers, causing "
                  "general malfunction. Remediate network congestion, if "
                  "present.\n");
    sl_zigbee_af_core_flush();
  }

  // Do not reset if this is a decryption failure, as we ignored the packet
  // Do not reset for a callback overflow, as we don't want the device to reboot
  // under stress
  // For all other errors, we reset the NCP
  if ((status != SL_ZIGBEE_EZSP_ERROR_SECURITY_PARAMETERS_INVALID)
      && (status != SL_ZIGBEE_EZSP_ERROR_OVERFLOW)) {
    bool callbackExists
      = ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks, onEmberAfNcpPreReset);
    if (callbackExists) {
      z3gwState.callbacks->onEmberAfNcpPreReset(status);
    }
    return true;
  }
#endif  // EZSP_HOST

  return false;
}

//STANDALONE_BOOTLOADER_RESET_MODE is 0
//STANDALONE_BOOTLOADER_NORMAL_MODE is 1
sl_status_t zigbeeHostLaunchBootloader()
{
  sl_status_t status = sl_zigbee_ezsp_launch_standalone_bootloader(1);

  sl_zigbee_ezsp_close();
  //sl_zigbee_af_core_flush(); alternatively will work

  return status;
}
