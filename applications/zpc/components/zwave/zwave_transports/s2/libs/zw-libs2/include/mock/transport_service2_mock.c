/*
 * transport_service2_mock.c
 *
 *  Created on: 11. nov. 2019
 *      Author: cholsen
 */
#include <ts_types.h>

void TransportService_ApplicationCommandHandler(ts_param_t* p,uint8_t *pCmd, uint8_t cmdLength)
{

}

bool ZW_TransportService_Is_Sending()
{
  return true;
}

bool ZW_TransportService_SendData(ts_param_t* p, const uint8_t *pData, uint16_t dataLength,
    void (*completedFunc)(uint8_t txStatus, void *))
{
  return true;
}
