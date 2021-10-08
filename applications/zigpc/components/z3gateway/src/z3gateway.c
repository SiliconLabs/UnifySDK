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
