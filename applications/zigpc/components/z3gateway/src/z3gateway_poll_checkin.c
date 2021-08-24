#include <string.h>
#include "z3gateway.h"

#include PLATFORM_HEADER
#include EMBER_AF_API_AF_HEADER
#include "gen/client-command-macro.h"

EmberStatus z3gatewaySendPollingCheckInResponse(void)
{
  bool fastPolling = true;

  //In units of quarters of a second
  //by default, has a value of 32 (based on poll-control-client plugin)
  uint16_t fastPollingTimeout
    = EMBER_AF_PLUGIN_POLL_CONTROL_CLIENT_DEFAULT_FAST_POLL_TIMEOUT;

  emberAfFillCommandPollControlClusterCheckInResponse(fastPolling,
                                                      fastPollingTimeout);

  emberAfSendResponse();

  return EMBER_SUCCESS;
}
