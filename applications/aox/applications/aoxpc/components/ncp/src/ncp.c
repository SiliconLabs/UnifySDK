/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "uic_main_externals.h"
#include "process.h"
#include "sl_log.h"
#include "aoxpc_config.h"
#include "uart.h"
#include "tcp.h"
#include "sl_bt_ncp_host.h"
#include "sl_bt_api.h"
#include "ncp_fixt.h"
#include "ncp.h"

// Log tag used in this module
#define LOG_TAG "ncp"

typedef enum { BT_EVENT, BT_RESET } ncp_process_event_t;

// Serial port settings
#define DEFAULT_SERIAL_FLOW_CONTROL 1
#define DEFAULT_SERIAL_TIMEOUT      100

// TCP/IP settings
#define DEFAULT_TCP_PORT              "4901"

typedef struct ncp_device_s {
  int fd; // File descriptor
  int32_t (*tx)(void *handle, uint32_t len, uint8_t *data);
  int32_t (*rx)(void *handle, uint32_t len, uint8_t *data);
  int32_t (*peek)(void *handle);
  int32_t (*close)(void *handle);
  void (*flush)(void *handle);
} ncp_device_t;

// NCP device instance
static ncp_device_t ncp_device;

// NCP ready flag
static bool ncp_ready = false;

// Bluetooth on event callback function pointer
static void(*ncp_bt_on_event)(sl_bt_msg_t *evt);

// Static function declarations
static sl_status_t ncp_open_serial(void);
static sl_status_t ncp_open_tcp(void);
static void ncp_poll(void);
static void ncp_tx(uint32_t len, uint8_t *data);
static int32_t ncp_rx(uint32_t len, uint8_t *data);
static int32_t ncp_peek(void);
static void ncp_flush(void);

// NCP process definition
PROCESS(ncp_process, "NCP process");

// NCP process implementation
PROCESS_THREAD(ncp_process, ev, data)
{
  PROCESS_POLLHANDLER(ncp_poll());

  PROCESS_BEGIN()
  while (1) {
    switch (ev) {
      case PROCESS_EVENT_INIT:
        sl_log_info(LOG_TAG, "Reset NCP");
        sl_bt_system_reset(sl_bt_system_boot_mode_normal);
        ncp_flush();
        break;
      case BT_EVENT:
        if (NULL != ncp_bt_on_event) {
          ncp_bt_on_event((sl_bt_msg_t *)data);
        }
        free(data);
        break;
      case BT_RESET:
         sl_bt_system_reset(sl_bt_system_boot_mode_normal);
         break;
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}

// Setup fixture, called on start-up
sl_status_t ncp_fixt_setup(void)
{
  sl_status_t status;

  status = ncp_open_tcp();
  if (status == SL_STATUS_INVALID_PARAMETER) {
    sl_log_debug(LOG_TAG,
                 "No TCP address specified for NCP. "
                 "Falling back on serial port.");
    status = ncp_open_serial();
    if (status != SL_STATUS_OK) {
      return status;
    }
  } else if (status != SL_STATUS_OK) {
    return status;
  }

  status = uic_main_ext_register_rfd(ncp_device.fd, NULL, &ncp_process);
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = sl_bt_api_initialize_nonblock(ncp_tx, ncp_rx, ncp_peek);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Start the NCP process
  process_start(&ncp_process, NULL);

  return SL_STATUS_OK;
}

int ncp_fixt_shutdown(void)
{
  int retval = 0;
  if (NULL != ncp_device.close) {
    retval = ncp_device.close(&ncp_device.fd);
  }
  return retval;
}

void ncp_set_cb(void(*bt_on_event)(sl_bt_msg_t *evt))
{
  ncp_bt_on_event = bt_on_event;
}

void ncp_reset(void) {
  if (ncp_ready) {
    ncp_ready = false;
    process_post(&ncp_process, BT_RESET, NULL);
  }
}

static sl_status_t ncp_open_serial(void)
{
  const char *serial_port;
  int baud_rate;
  int32_t status;

  if (CONFIG_STATUS_OK
      != config_get_as_string(CONFIG_KEY_AOXPC_NCP_SERIAL, &serial_port)) {
    return SL_STATUS_FAIL;
  }

  if (CONFIG_STATUS_OK
      != config_get_as_int(CONFIG_KEY_AOXPC_NCP_BAUD_RATE, &baud_rate)) {
    return SL_STATUS_FAIL;
  }

  if (baud_rate < 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = uartOpen(&ncp_device.fd,
                    (int8_t *)serial_port,
                    baud_rate,
                    DEFAULT_SERIAL_FLOW_CONTROL,
                    DEFAULT_SERIAL_TIMEOUT);
  if (status < 0) {
    sl_log_error(LOG_TAG, "Failed to open serial port.");
    return SL_STATUS_FAIL;
  }

  ncp_device.tx = uartTx;
  ncp_device.rx = uartRx;
  ncp_device.peek = uartRxPeek;
  ncp_device.close = uartClose;
  ncp_device.flush = uartFlush;

  return SL_STATUS_OK;
}

static sl_status_t ncp_open_tcp(void)
{
  const char *address;
  int32_t status;

  if (CONFIG_STATUS_OK
      != config_get_as_string(CONFIG_KEY_AOXPC_NCP_TPC, &address)) {
    return SL_STATUS_FAIL;
  }

  // Check if the address string is empty
  if (*address == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = tcp_open(&ncp_device.fd,
                    (char *)address,
                    DEFAULT_TCP_PORT);
  if (status != 0) {
    sl_log_error(LOG_TAG, "Failed to open TCP port.");
    return SL_STATUS_FAIL;
  }

  ncp_device.tx = tcp_tx;
  ncp_device.rx = tcp_rx;
  ncp_device.peek = tcp_rx_peek;
  ncp_device.close = tcp_close;
  ncp_device.flush = NULL;

  return SL_STATUS_OK;
}

static void ncp_poll(void)
{
  sl_bt_msg_t evt;
  sl_bt_msg_t *evt_ptr;

  // Pop (non-blocking) a Bluetooth stack event from event queue.
  sl_status_t status = sl_bt_pop_event(&evt);
  if (status != SL_STATUS_OK) {
    return;
  }

  // Ignore all events until the boot event.
  if (sl_bt_evt_system_boot_id == SL_BT_MSG_ID(evt.header)) {
    ncp_ready = true;
  }

  if (false == ncp_ready) {
    return;
  }

  evt_ptr = malloc(sizeof(sl_bt_msg_t));
  if (evt_ptr != NULL) {
    memcpy(evt_ptr, &evt, sizeof(sl_bt_msg_t));
    process_post(&ncp_process, BT_EVENT, evt_ptr);
  } else {
    sl_log_error(LOG_TAG, "Memory allocation failed, BT event lost.");
  }
}

static void ncp_tx(uint32_t len, uint8_t *data)
{
  if (ncp_device.tx(&ncp_device.fd, len, data) < 0) {
    sl_log_error(LOG_TAG, "tx failed");
  }
}

static int32_t ncp_rx(uint32_t len, uint8_t *data)
{
  return ncp_device.rx(&ncp_device.fd, len, data);
}

static int32_t ncp_peek(void)
{
  int32_t retval;

  retval = ncp_device.peek(&ncp_device.fd);
  if (retval < 0) {
    sl_log_critical(LOG_TAG,
                    "NCP Peek failed. Return value: %i. Error: %s. "
                    "Exiting Application.",
                    retval,
                    strerror(errno));
    exit(EXIT_FAILURE);
  }
  return retval;
}

static void ncp_flush(void)
{
  if (NULL != ncp_device.flush) {
    ncp_device.flush(&ncp_device.fd);
  }
}