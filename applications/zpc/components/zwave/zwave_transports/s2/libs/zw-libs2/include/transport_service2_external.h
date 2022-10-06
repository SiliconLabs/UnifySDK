/* © 2017 Sigma Designs, Inc. This is an unpublished work protected by Sigma
 * Designs, Inc. as a trade secret, and is not to be used or disclosed except as
 * provided Z-Wave Controller Development Kit Limited License Agreement. All
 * rights reserved.
 *
 * Notice: All information contained herein is confidential and/or proprietary to
 * Sigma Designs and may be covered by U.S. and Foreign Patents, patents in
 * process, and are protected by trade secret or copyright law. Dissemination or
 * reproduction of the source code contained herein is expressly forbidden to
 * anyone except Licensees of Sigma Designs  who have executed a Sigma Designs’
 * Z-WAVE CONTROLLER DEVELOPMENT KIT LIMITED LICENSE AGREEMENT. The copyright
 * notice above is not evidence of any actual or intended publication of the
 * source code. THE RECEIPT OR POSSESSION OF  THIS SOURCE CODE AND/OR RELATED
 * INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS  TO REPRODUCE, DISCLOSE OR
 * DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE, USE, OR SELL A PRODUCT THAT IT  MAY
 * DESCRIBE.
 *
 * THE SIGMA PROGRAM AND ANY RELATED DOCUMENTATION OR TOOLS IS PROVIDED TO COMPANY
 * "AS IS" AND "WITH ALL FAULTS", WITHOUT WARRANTY OF ANY KIND FROM SIGMA. COMPANY
 * ASSUMES ALL RISKS THAT LICENSED MATERIALS ARE SUITABLE OR ACCURATE FOR
 * COMPANY’S NEEDS AND COMPANY’S USE OF THE SIGMA PROGRAM IS AT COMPANY’S
 * OWN DISCRETION AND RISK. SIGMA DOES NOT GUARANTEE THAT THE USE OF THE SIGMA
 * PROGRAM IN A THIRD PARTY SERVICE ENVIRONMENT OR CLOUD SERVICES ENVIRONMENT WILL
 * BE: (A) PERFORMED ERROR-FREE OR UNINTERRUPTED; (B) THAT SIGMA WILL CORRECT ANY
 * THIRD PARTY SERVICE ENVIRONMENT OR CLOUD SERVICE ENVIRONMENT ERRORS; (C) THE
 * THIRD PARTY SERVICE ENVIRONMENT OR CLOUD SERVICE ENVIRONMENT WILL OPERATE IN
 * COMBINATION WITH COMPANY’S CONTENT OR COMPANY APPLICATIONS THAT UTILIZE THE
 * SIGMA PROGRAM; (D) OR WITH ANY OTHER HARDWARE, SOFTWARE, SYSTEMS, SERVICES OR
 * DATA NOT PROVIDED BY SIGMA. COMPANY ACKNOWLEDGES THAT SIGMA DOES NOT CONTROL
 * THE TRANSFER OF DATA OVER COMMUNICATIONS FACILITIES, INCLUDING THE INTERNET,
 * AND THAT THE SERVICES MAY BE SUBJECT TO LIMITATIONS, DELAYS, AND OTHER PROBLEMS
 * INHERENT IN THE USE OF SUCH COMMUNICATIONS FACILITIES. SIGMA IS NOT RESPONSIBLE
 * FOR ANY DELAYS, DELIVERY FAILURES, OR OTHER DAMAGE RESULTING FROM SUCH ISSUES.
 * SIGMA IS NOT RESPONSIBLE FOR ANY ISSUES RELATED TO THE PERFORMANCE, OPERATION
 * OR SECURITY OF THE THIRD PARTY SERVICE ENVIRONMENT OR CLOUD SERVICES
 * ENVIRONMENT THAT ARISE FROM COMPANY CONTENT, COMPANY APPLICATIONS OR THIRD
 * PARTY CONTENT. SIGMA DOES NOT MAKE ANY REPRESENTATION OR WARRANTY REGARDING THE
 * RELIABILITY, ACCURACY, COMPLETENESS, CORRECTNESS, OR USEFULNESS OF THIRD PARTY
 * CONTENT OR SERVICE OR THE SIGMA PROGRAM, AND DISCLAIMS ALL LIABILITIES ARISING
 * FROM OR RELATED TO THE SIGMA PROGRAM OR THIRD PARTY CONTENT OR SERVICES. TO THE
 * EXTENT NOT PROHIBITED BY LAW, THESE WARRANTIES ARE EXCLUSIVE. SIGMA OFFERS NO
 * WARRANTY OF NON-INFRINGEMENT, TITLE, OR QUIET ENJOYMENT. NEITHER SIGMA NOR ITS
 * SUPPLIERS OR LICENSORS SHALL BE LIABLE FOR ANY INDIRECT, SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES OR LOSS (INCLUDING DAMAGES FOR LOSS OF BUSINESS, LOSS OF
 * PROFITS, OR THE LIKE), ARISING OUT OF THIS AGREEMENT WHETHER BASED ON BREACH OF
 * CONTRACT, INTELLECTUAL PROPERTY INFRINGEMENT, TORT (INCLUDING NEGLIGENCE),
 * STRICT LIABILITY, PRODUCT LIABILITY OR OTHERWISE, EVEN IF SIGMA OR ITS
 * REPRESENTATIVES HAVE BEEN ADVISED OF OR OTHERWISE SHOULD KNOW ABOUT THE
 * POSSIBILITY OF SUCH DAMAGES. THERE ARE NO OTHER EXPRESS OR IMPLIED WARRANTIES
 * OR CONDITIONS INCLUDING FOR SOFTWARE, HARDWARE, SYSTEMS, NETWORKS OR
 * ENVIRONMENTS OR FOR MERCHANTABILITY, NONINFRINGEMENT, SATISFACTORY QUALITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * The Sigma Program  is not fault-tolerant and is not designed, manufactured or
 * intended for use or resale as on-line control equipment in hazardous
 * environments requiring fail-safe performance, such as in the operation of
 * nuclear facilities, aircraft navigation or communication systems, air traffic
 * control, direct life support machines, or weapons systems, in which the failure
 * of the Sigma Program, or Company Applications created using the Sigma Program,
 * could lead directly to death, personal injury, or severe physical or
 * environmental damage ("High Risk Activities").  Sigma and its suppliers
 * specifically disclaim any express or implied warranty of fitness for High Risk
 * Activities.Without limiting Sigma’s obligation of confidentiality as further
 * described in the Z-Wave Controller Development Kit Limited License Agreement,
 * Sigma has no obligation to establish and maintain a data privacy and
 * information security program with regard to Company’s use of any Third Party
 * Service Environment or Cloud Service Environment. For the avoidance of doubt,
 * Sigma shall not be responsible for physical, technical, security,
 * administrative, and/or organizational safeguards that are designed to ensure
 * the security and confidentiality of the Company Content or Company Application
 * in any Third Party Service Environment or Cloud Service Environment that
 * Company chooses to utilize.
 */
/*
 * transport_service2.h
 *
 *  Created on: Nov 12, 2015
 *      Author: jbu
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
