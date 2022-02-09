/**
 * @file sl_status.h
 * @brief SL Status Codes.
 *
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 */

#ifndef SL_STATUS_H
#define SL_STATUS_H

#include <stdint.h>


/**
 * @defgroup unify_definitions Unify definitions
 * @ingroup unify_components
 * @brief Type definitions and macros for Unify components and applications.
 */


/**
 * @defgroup unify_sl_status_codes Status Codes
 * @ingroup unify_definitions
 * @brief Status codes used to indicate the outcome of a function.
 * @{
 */

/*
 ***************************   STATUS DEFINES   ********************************
 */

/*
 ***************************   GENERIC ERRORS   ********************************
 */
/*******************************************************************************
 ***************************   GENERIC ERRORS   ********************************
 ******************************************************************************/

#define SL_STATUS_OK    ((sl_status_t)0x0000)  ///< No error.
#define SL_STATUS_FAIL  ((sl_status_t)0x0001)  ///< Generic error.

// State Errors
#define SL_STATUS_INVALID_STATE         ((sl_status_t)0x0002)  ///< Generic invalid state error.
#define SL_STATUS_NOT_READY             ((sl_status_t)0x0003)  ///< Module is not ready for requested operation.
#define SL_STATUS_BUSY                  ((sl_status_t)0x0004)  ///< Module is busy and cannot carry out requested operation.
#define SL_STATUS_IN_PROGRESS           ((sl_status_t)0x0005)  ///< Operation is in progress and not yet complete (pass or fail).
#define SL_STATUS_ABORT                 ((sl_status_t)0x0006)  ///< Operation aborted.
#define SL_STATUS_TIMEOUT               ((sl_status_t)0x0007)  ///< Operation timed out.
#define SL_STATUS_PERMISSION            ((sl_status_t)0x0008)  ///< Operation not allowed per permissions.
#define SL_STATUS_WOULD_BLOCK           ((sl_status_t)0x0009)  ///< Non-blocking operation would block.
#define SL_STATUS_IDLE                  ((sl_status_t)0x000A)  ///< Operation/module is Idle, cannot carry requested operation.
#define SL_STATUS_IS_WAITING            ((sl_status_t)0x000B)  ///< Operation cannot be done while construct is waiting.
#define SL_STATUS_NONE_WAITING          ((sl_status_t)0x000C)  ///< No task/construct waiting/pending for that action/event.
#define SL_STATUS_SUSPENDED             ((sl_status_t)0x000D)  ///< Operation cannot be done while construct is suspended.
#define SL_STATUS_NOT_AVAILABLE         ((sl_status_t)0x000E)  ///< Feature not available due to software configuration.
#define SL_STATUS_NOT_SUPPORTED         ((sl_status_t)0x000F)  ///< Feature not supported.
#define SL_STATUS_INITIALIZATION        ((sl_status_t)0x0010)  ///< Initialization failed.
#define SL_STATUS_NOT_INITIALIZED       ((sl_status_t)0x0011)  ///< Module has not been initialized.
#define SL_STATUS_ALREADY_INITIALIZED   ((sl_status_t)0x0012)  ///< Module has already been initialized.
#define SL_STATUS_DELETED               ((sl_status_t)0x0013)  ///< Object/construct has been deleted.
#define SL_STATUS_ISR                   ((sl_status_t)0x0014)  ///< Illegal call from ISR.
#define SL_STATUS_NETWORK_UP            ((sl_status_t)0x0015)  ///< Illegal call because network is up.
#define SL_STATUS_NETWORK_DOWN          ((sl_status_t)0x0016)  ///< Illegal call because network is down.
#define SL_STATUS_NOT_JOINED            ((sl_status_t)0x0017)  ///< Failure due to not being joined in a network.
#define SL_STATUS_NO_BEACONS            ((sl_status_t)0x0018)  ///< Invalid operation as there are no beacons.

// Allocation/ownership Errors
#define SL_STATUS_ALLOCATION_FAILED   ((sl_status_t)0x0019)  ///< Generic allocation error.
#define SL_STATUS_NO_MORE_RESOURCE    ((sl_status_t)0x001A)  ///< No more resource available to perform the operation.
#define SL_STATUS_EMPTY               ((sl_status_t)0x001B)  ///< Item/list/queue is empty.
#define SL_STATUS_FULL                ((sl_status_t)0x001C)  ///< Item/list/queue is full.
#define SL_STATUS_WOULD_OVERFLOW      ((sl_status_t)0x001D)  ///< Item would overflow.
#define SL_STATUS_HAS_OVERFLOWED      ((sl_status_t)0x001E)  ///< Item/list/queue has been overflowed.
#define SL_STATUS_OWNERSHIP           ((sl_status_t)0x001F)  ///< Generic ownership error.
#define SL_STATUS_IS_OWNER            ((sl_status_t)0x0020)  ///< Already/still owning resource.

// Invalid Parameters Errors
#define SL_STATUS_INVALID_PARAMETER       ((sl_status_t)0x0021)  ///< Generic invalid argument or consequence of invalid argument.
#define SL_STATUS_NULL_POINTER            ((sl_status_t)0x0022)  ///< Invalid null pointer received as argument.
#define SL_STATUS_INVALID_CONFIGURATION   ((sl_status_t)0x0023)  ///< Invalid configuration provided.
#define SL_STATUS_INVALID_MODE            ((sl_status_t)0x0024)  ///< Invalid mode.
#define SL_STATUS_INVALID_HANDLE          ((sl_status_t)0x0025)  ///< Invalid handle.
#define SL_STATUS_INVALID_TYPE            ((sl_status_t)0x0026)  ///< Invalid type for operation.
#define SL_STATUS_INVALID_INDEX           ((sl_status_t)0x0027)  ///< Invalid index.
#define SL_STATUS_INVALID_RANGE           ((sl_status_t)0x0028)  ///< Invalid range.
#define SL_STATUS_INVALID_KEY             ((sl_status_t)0x0029)  ///< Invalid key.
#define SL_STATUS_INVALID_CREDENTIALS     ((sl_status_t)0x002A)  ///< Invalid credentials.
#define SL_STATUS_INVALID_COUNT           ((sl_status_t)0x002B)  ///< Invalid count.
#define SL_STATUS_INVALID_SIGNATURE       ((sl_status_t)0x002C)  ///< Invalid signature / verification failed.
#define SL_STATUS_NOT_FOUND               ((sl_status_t)0x002D)  ///< Item could not be found.
#define SL_STATUS_ALREADY_EXISTS          ((sl_status_t)0x002E)  ///< Item already exists.

// IO/Communication Errors
#define SL_STATUS_IO                    ((sl_status_t)0x002F)  ///< Generic I/O failure.
#define SL_STATUS_IO_TIMEOUT            ((sl_status_t)0x0030)  ///< I/O failure due to timeout.
#define SL_STATUS_TRANSMIT              ((sl_status_t)0x0031)  ///< Generic transmission error.
#define SL_STATUS_TRANSMIT_UNDERFLOW    ((sl_status_t)0x0032)  ///< Transmit underflowed.
#define SL_STATUS_TRANSMIT_INCOMPLETE   ((sl_status_t)0x0033)  ///< Transmit is incomplete.
#define SL_STATUS_TRANSMIT_BUSY         ((sl_status_t)0x0034)  ///< Transmit is busy.
#define SL_STATUS_RECEIVE               ((sl_status_t)0x0035)  ///< Generic reception error.
#define SL_STATUS_OBJECT_READ           ((sl_status_t)0x0036)  ///< Failed to read on/via given object.
#define SL_STATUS_OBJECT_WRITE          ((sl_status_t)0x0037)  ///< Failed to write on/via given object.
#define SL_STATUS_MESSAGE_TOO_LONG      ((sl_status_t)0x0038)  ///< Message is too long.

// ANSI C/POSIX codes
#define SL_STATUS_ERRNO                   ((sl_status_t)0x0101) ///< System error: errno is set and strerror can be used to fetch the error-message.

// Net-MQTT status codes
#define SL_STATUS_NET_MQTT_NO_CONN              ((sl_status_t)0x0841) ///< Not connected to a broker.
#define SL_STATUS_NET_MQTT_LOST_CONN            ((sl_status_t)0x0842) ///< Connection to broker lost.
#define SL_STATUS_NET_MQTT_PROTOCOL             ((sl_status_t)0x0843) ///< Protocol error.
#define SL_STATUS_NET_MQTT_TLS_HANDSHAKE        ((sl_status_t)0x0844) ///< TLS negotiation failed.
#define SL_STATUS_NET_MQTT_PAYLOAD_SIZE         ((sl_status_t)0x0845) ///< Payload size is too large.
#define SL_STATUS_NET_MQTT_NOT_SUPPORTED        ((sl_status_t)0x0846) ///< MQTTv5 properties are set but client is not using MQTTv5.
#define SL_STATUS_NET_MQTT_AUTH                 ((sl_status_t)0x0847) ///< Authentication failed.
#define SL_STATUS_NET_MQTT_ACL_DENIED           ((sl_status_t)0x0848) ///< Access control list deny.
#define SL_STATUS_NET_MQTT_MALFORMED_UTF8       ((sl_status_t)0x0849) ///< Malformed UTF-8 string in the specified MQTT-topic.
#define SL_STATUS_NET_MQTT_DUPLICATE_PROPERTY   ((sl_status_t)0x084A) ///< An MQTTv5 property is duplicated where it is forbidden.
#define SL_STATUS_NET_MQTT_QOS_NOT_SUPPORTED    ((sl_status_t)0x084B) ///< The requested QoS level is not supported by the broker.
#define SL_STATUS_NET_MQTT_OVERSIZE_PACKET      ((sl_status_t)0x084C) ///< Resulting packet will become larger than the broker supports.

/*
 ********************************   DATA TYPES   *******************************
 */

typedef uint32_t sl_status_t;

/*
 ********************************   FUNCTIONS   ********************************
 */

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder for future string functions.

#ifdef __cplusplus
}
#endif

/** @} (end unify_sl_status_codes) */

#endif /* SL_STATUS_H */
