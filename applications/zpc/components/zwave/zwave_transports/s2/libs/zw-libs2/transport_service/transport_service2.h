/* © 2017 Silicon Laboratories Inc.
 */
/*
 * transport_service2.h
 *
 *  Created on: Oct 10, 2014
 *      Author: aes
 */

#ifndef TRANSPORT_SERVICE2_H_
#define TRANSPORT_SERVICE2_H_

//#include "ZW_typedefs.h"
#if defined(ZIPGW)
#include <TYPES.H>
#include "ZW_SendDataAppl.h" /* for ts_param_t */
#include <stdio.h>
#include <ZW_transport_api.h>
#else
#include "ts_types.h"
#endif

#include <ZW_classcmd.h> /* for ZW_APPLICATION_TX_BUFFER */
#include "transport2_fsm.h"
//#include <ZIP_Router.h>


extern TRANSPORT2_ST_T current_state;


#define FRAGMENT_FC_TIMEOUT          1000 /*ms*/
#define FRAGMENT_RX_TIMEOUT          800 /*ms*/

//#define DATAGRAM_SIZE_MAX       (UIP_BUFSIZE - UIP_LLH_LEN) /*1280*/
#define DATAGRAM_SIZE_MAX       (200) /*1280*/

#ifdef __C51__
#ifndef slash
#define slash /
#endif
#endif

#ifdef __C51__
#define T2_DBG slash/
#else
//#define DBG 1
#ifdef DBG
#define T2_DBG(...) \
        printf("T2: %s sid: %d rid: %d, %s():%d: ",T2_STATES_STRING[current_state],scb.cmn.session_id, rcb.cmn.session_id,__func__, __LINE__);\
        printf(__VA_ARGS__); \
        printf("\n");
#else
#define T2_DBG(...)
#endif
#endif

#ifdef __C51__
#define T2_ERR slash/
#else
#if defined(ZIPGW) || defined(DBG)
#define T2_ERR(...) \
        printf("T2: %s sid: %d rid: %d, %s():%d: ", T2_STATES_STRING[current_state],scb.cmn.session_id, rcb.cmn.session_id,__func__, __LINE__);\
        printf(__VA_ARGS__); \
        printf("\n");
#else
#define T2_ERR(...)
#endif
#endif

#define TIMER
//#define DEBUG_TIMER

#if !defined(ZIPGW)
#include <transport_service2_external.h>
#else
/**
 * Input function for the transport service module, this must be called when we receive a frame
 * of type COMMAND_CLASS_TRANSPORT_SERVICE.
 *
 * \param p structure containing the parameters of the transmission, like source node and destination node.
 * \param pCmd pointer to the received frame.
 * \param cmdLength Length of the received frame.
 */
void TransportService_ApplicationCommandHandler(ts_param_t* p, uint8_t *pCmd, uint8_t cmdLength);

/**
 * Initialize the Transport service state machine.
 * \param commandHandler Application command handler to be called when a full datagram has been received.
 *
 */
void
ZW_TransportService_Init(void
(*commandHandler)(ts_param_t* p, ZW_APPLICATION_TX_BUFFER *pCmd,
uint16_t cmdLength));

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
bool ZW_TransportService_SendData(ts_param_t* p, const uint8_t *pData, uint16_t dataLength,
    void (*completedFunc)(uint8_t txStatus, TX_STATUS_TYPE *));


#endif

#endif /* TRANSPORT_SERVICE2_H_ */

