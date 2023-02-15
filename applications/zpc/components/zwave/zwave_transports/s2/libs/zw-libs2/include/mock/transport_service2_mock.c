/**
 * @file transport_service2_mock.c
 * @copyright 2022 Silicon Laboratories Inc.
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
