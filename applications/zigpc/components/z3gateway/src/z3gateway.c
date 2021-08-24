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
  int argc = 1;
  char *argv[Z3GATEWAY_EMBERAF_ARGV_LEN];

  if (opts == NULL) {
    emberAfCorePrintln("Gatway options not found");
    return EMBER_BAD_ARGUMENT;
  }

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

  halInit();
  emberAfMainInit();
  return emberAfMain(MAIN_FUNCTION_ARGUMENTS);
}

void z3gatewayTick(void)
{
  emberAfMainTick();
}

void z3gatewayShutdown(void)
{
  emberSerialCleanup();
}
