#include "zigpc_config_validate.h"

static const int MIN_VAL = 1;
static const int MAX_PAN_ID = 65535;
static const int MAX_RADIO_PW = 127;
static const int MAX_CHANNEL = 255;

config_status_t
    zigpc_config_network_args_validate(
            bool use_network_args,
            int network_pan_id,
            int network_radio_power,
            int network_channel)
{
    config_status_t status = CONFIG_STATUS_ERROR;

    if(use_network_args)
    {
        bool valid_pan = 
            (MIN_VAL <= network_pan_id) && 
            (network_pan_id <= MAX_PAN_ID );

        bool valid_radio_pw = 
            (MIN_VAL <= network_radio_power) && 
            (network_radio_power <= MAX_RADIO_PW );

        bool valid_channel = 
            (MIN_VAL <= network_radio_power) && 
            (network_radio_power <= MAX_CHANNEL );

        status = (valid_pan && valid_radio_pw && valid_channel)? 
            CONFIG_STATUS_OK: CONFIG_STATUS_ERROR;
    }
    else
    {
        status = CONFIG_STATUS_OK;
    }
    
    return status;
}

