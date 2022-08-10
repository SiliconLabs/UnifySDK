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

#include "sl_log.h"
#include "aoxpc_config.h"
#include "uart.h"
#include "tcp.h"
#include "sl_bt_ncp_host.h"
#include "aoxpc_ncp_core.h"
#include "aoxpc_ncp_core_fixt.h"

// Log tag used in this module
#define LOG_TAG "aoxpc_ncp_core"

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

// Static function declarations
static sl_status_t ncp_open_serial(void);
static sl_status_t ncp_open_tcp(void);

sl_status_t aoxpc_ncp_core_fixt_setup(void)
{
  sl_status_t status;

  status = ncp_open_tcp();
  if (status == SL_STATUS_INVALID_PARAMETER) {
    sl_log_debug(LOG_TAG,
                 "No TCP address specified for NCP. "
                 "Falling back on serial port.");
    status = ncp_open_serial();
  }
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = sl_bt_api_initialize_nonblock(ncp_tx, ncp_rx, ncp_peek);
  
  return status;
}

int ncp_get_fd(void)
{
  return ncp_device.fd;
}

void ncp_tx(uint32_t len, uint8_t *data)
{
  if (ncp_device.tx(&ncp_device.fd, len, data) < 0) {
    sl_log_error(LOG_TAG, "tx failed");
  }
}

int32_t ncp_rx(uint32_t len, uint8_t *data)
{
  return ncp_device.rx(&ncp_device.fd, len, data);
}

int32_t ncp_peek(void)
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

void ncp_flush(void)
{
  if (NULL != ncp_device.flush) {
    ncp_device.flush(&ncp_device.fd);
  }
}

int ncp_deinit(void)
{
  int retval = 0;
  if (NULL != ncp_device.close) {
    retval = ncp_device.close(&ncp_device.fd);
  }
  return retval;
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
