/**
 * @file transport_service2_external.h
 * @copyright 2022 Silicon Laboratories Inc.
 */
#ifndef TRANSPORT_SERVICE2_EXTERNAL_H_
#define TRANSPORT_SERVICE2_EXTERNAL_H_

#include "stdbool.h"
/* Transport Sevice 2 command class version */
#define TRANSPORT_SERVICE2_SUPPORTED_VERSION TRANSPORT_SERVICE_VERSION_V2

typedef struct ts_CommandHandler
{
  ts_param_t *pParam;
  ZW_APPLICATION_TX_BUFFER *pCmd;
  uint16_t wCmdLength;
} ts_CommandHandler_t;

/**
 * \defgroup TransportService Transport service module
 * \{
 *
 * This module handles the Z-Wave Transport Service command class version 2.
 * The module is able handle a single TX session to a node, and able to handle
 * a number of RX session for some nodes.
 */


/**
 * Send a large frame from srcNodeID to dstNodeID using TRANSPORT_SERVICE V2. Only one
 * transmit session is allowed at any time.
 *
 * \param p structure containing the parameters of the transmission, like source node and destination node.
 * \param pData pointer to the data being sent. The contents of this buffer must not change
 * while the transmission is in progress.
 * \param txOption the Z-Wave transmit options to use in this transmission.
 * \param completedFunc A callback which is called when the transmission has completed. The status of the
 * transmission is given in txStatus. See \ref ZW_SendData.
 * \return
 *      - true if the transmission is started, and callback will be called when the transmission is done.
 *      - false Transmission is not started, because another transmission is already on progress.
 *        The callback function will not be called.
 */
#if defined(ZIPGW) || defined(EFR32ZG) || defined(ZWAVE_ON_LINUX)
bool ZW_TransportService_SendData(ts_param_t* p, const uint8_t *pData, uint16_t dataLength,
    void (*completedFunc)(uint8_t txStatus, void *));
#else
bool ZW_TransportService_SendData(ts_param_t* p, uint8_t *pData, uint16_t dataLength,
    void (*completedFunc)(uint8_t txStatus, void *));
#endif

/**
 * Input function for the transport service module, this must be called when we receive a frame
 * of type COMMAND_CLASS_TRANSPORT_SERVICE.
 *
 * \param pCmdHandlerStruct structure contains the reveiced frame and the parameters of the transmission.
 *    p structure containing the parameters of the transmission, like source node and destination node.
 *    pCmd pointer to the received frame.
 *    mdLength Length of the received frame.
 */
#ifdef __C51__
void TransportService_ApplicationCommandHandler(ts_CommandHandler_t *pCmdHandlerStruct);
#else
void TransportService_ApplicationCommandHandler(ts_param_t* p,uint8_t *pCmd, uint8_t cmdLength);
#endif

/**
 * Abort the current transmission, and cause the callback of \ref ZW_TransportService_SendData to
 * be sent as soon as possible.
 * \note the transmission is not aborted immediately.
 */
void ZW_TransportService_SendDataAbort();

/**
 * Initialize the Transport service state machine.
 * \param commandHandler Application command handler to be called when a full datagram has been received.
 *
 */
void
ZW_TransportService_Init(void (*commandHandler)(ts_param_t* p, ZW_APPLICATION_TX_BUFFER *pCmd,
uint16_t cmdLength));

/**
 * Return true if at least one RX session is in progress.
 */
bool ZW_TransportService_Is_Receving();


/**
 * Return true if a TX session is in progress.
 */
bool ZW_TransportService_Is_Sending();


/**
 * Output function of the Transport Service module.
 * This function is called whenever a message has been reassembled and is ready for processing
 * at the client layer.
 */
void TransportService_msg_received_event(unsigned char *pCmd, unsigned char cmdLen,  unsigned char srcNode);

/**
 * \}
 */

#if (defined (EFR32ZG) || defined(ZWAVE_ON_LINUX)) && !defined(NEW_TEST_T2)
/**
 * Sending function called by the transport service module when a frame is ready to be sent by the lower layer.
 */
bool TS_SendRaw(uint16_t dst, uint8_t *buf, uint8_t buflen, uint8_t txopt, VOID_CALLBACKFUNC(cb)(unsigned char status, TX_STATUS_TYPE* txS));
#endif

#endif /* TRANSPORT_SERVICE2_EXTERNAL_H_ */
