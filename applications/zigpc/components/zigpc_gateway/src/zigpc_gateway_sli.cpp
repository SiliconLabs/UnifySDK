#include "sl_log.h"

// Component includes
#include "zigpc_gateway.h"

#include "sl_status.h"

#include "zigpc_gateway_int.h"

// Internal stream
#ifdef __cplusplus
extern "C" {
#endif

void zigpc_gateway_send_emberaf_command(char* array){
 zigbeeHostSendSlCliCommand(array);
}

#ifdef __cplusplus
}
#endif