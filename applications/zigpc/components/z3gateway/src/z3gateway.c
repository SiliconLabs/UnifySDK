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

/* stack includes */
#include PLATFORM_HEADER
#include EMBER_AF_API_AF_HEADER
#include EMBER_AF_API_AF_TYPES_HEADER
#include EMBER_AF_API_AF_MAIN_HEADER
#include EMBER_AF_API_AF_MAIN_HOST_HEADER
#include "znet-bookkeeping.h"
#include "linux-serial.h"

/* external includes */
#include "z3gateway.h"

/* internal includes */
#include "z3gateway_common.h"

#define Z3GATEWAY_EMBERAF_ARGV_LEN 3
#define Z3GATEWAY_EMBERAF_ARG0     "z3gateway"

struct z3gatewayState z3gwState;

int z3gatewayInit(struct z3gatewayOpts *opts)
{
  EmberStatus status = EMBER_SUCCESS;
  int argc           = 1;
  char *argv[Z3GATEWAY_EMBERAF_ARGV_LEN];

  if (opts == NULL) {
    emberAfCorePrintln("Gateway options not found");
    status = EMBER_BAD_ARGUMENT;
  } else {
    if (opts->callbacks != NULL) {
      z3gwState.callbacks = opts->callbacks;
    }

    /* build stdin arguments passed to Ember application framework */
    argv[0] = Z3GATEWAY_EMBERAF_ARG0;
    if (opts->serialPort != NULL) {
      argv[1] = "-p";
      argv[2] = (char *)opts->serialPort;
      argc += 2;
    }

    // call EmberAf init routines
    halInit();
    emberAfMainInit();
    status = emberAfMain(MAIN_FUNCTION_ARGUMENTS);
  }

  return status;
}

void z3gatewayTick(void)
{
  emberAfMainTick();
}

void z3gatewayShutdown(void)
{
  emberSerialCleanup();
}

EmberStatus z3gatewayGetEmberKey(EmberKeyType type, EmberKeyStruct *const key)
{
  EmberStatus status = EMBER_SUCCESS;
  if (key == NULL) {
    status = EMBER_BAD_ARGUMENT;
  } else {
    status = emberGetKey(type, key);
  }
  return status;
}

EzspStatus z3gatewaySetEzspPolicy(EzspPolicyId policyId,
                                  EzspDecisionId decisionId,
                                  const char *policyName,
                                  const char *decisionName)
{
  return emberAfSetEzspPolicy(policyId, decisionId, policyName, decisionName);
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
void emberAfMainInitCallback(void)
{
  bool callbackExists
    = Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks, onEmberAfStackInitalized);
  if (callbackExists) {
    z3gwState.callbacks->onEmberAfStackInitalized();
  }
}

/** @brief Ncp Init
 *
 * This function is called when the network coprocessor is being initialized,
 * either at startup or upon reset.  It provides applications on opportunity to
 * perform additional configuration of the NCP.  The function is always called
 * twice when the NCP is initialized.  In the first invocation, memoryAllocation
 * will be true and the application should only issue EZSP commands that affect
 * memory allocation on the NCP.  For example, tables on the NCP can be resized
 * in the first call.  In the second invocation, memoryAllocation will be false
 * and the application should only issue EZSP commands that do not affect memory
 * allocation.  For example, tables on the NCP can be populated in the second
 * call.  This callback is not called on SoCs.
 *
 * @param memoryAllocation   Ver.: always
 */
void emberAfNcpInitCallback(bool memoryAllocation)
{
  bool callbackExists
    = Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks, onEmberAfNcpPostReset);
  if (callbackExists) {
    z3gwState.callbacks->onEmberAfNcpPostReset(memoryAllocation);
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
 * @param status The EzspStatus error code received.
 *
 * @return bool True to reset NCP, false not to.
 *
 * @note This callback is only fired on the host application. It has no use for
 * SoC or NCP applications.
 */
bool emberAfPluginZclFrameworkCoreEzspErrorCallback(EzspStatus status)
{
#if defined EZSP_HOST
  if (status == EZSP_ERROR_OVERFLOW) {
    emberAfCorePrintln("WARNING: the NCP has run out of buffers, causing "
                       "general malfunction. Remediate network congestion, if "
                       "present.");
    emberAfCoreFlush();
  }

  // Do not reset if this is a decryption failure, as we ignored the packet
  // Do not reset for a callback overflow, as we don't want the device to reboot
  // under stress
  // For all other errors, we reset the NCP
  if ((status != EZSP_ERROR_SECURITY_PARAMETERS_INVALID)
      && (status != EZSP_ERROR_OVERFLOW)) {
    bool callbackExists
      = Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks, onEmberAfNcpPreReset);
    if (callbackExists) {
      z3gwState.callbacks->onEmberAfNcpPreReset(status);
    }
    return true;
  }
#endif  // EZSP_HOST

  return false;
}
