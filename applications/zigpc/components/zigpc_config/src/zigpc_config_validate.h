
#ifndef ZIGPC_CONFIG_VALIDATE
#define ZIGPC_CONFIG_VALIDATE

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

config_status_t 
    zigpc_config_network_args_validate(
            bool use_network_args,
            int network_pan_id,
            int network_radio_power,
            int network_channel);

#endif

#ifdef __cplusplus
}
#endif
