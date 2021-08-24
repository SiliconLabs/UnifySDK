/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// Includes from this component
#include "zwave_rx.h"
#include "zwave_rx_process.h"
#include "zwave_rx_internals.h"

// Includes from other components
#include "zpc_config.h"
#include "zwapi_init.h"

// Contiki includes
#include "process.h"
/**
 * @brief Polls for incoming messages from the Z-Wave Module.
 * 
 * This function dequeues and acknowledges the next frame received on the 
 * serial port by the Z-Wave module. 
 */
void zwave_rx_poll()
{
  // Dequeue one received frame and ask for more polling if there is
  // something left
  if (zwapi_poll()) {
    process_poll(&zwave_rx_process);
  }
}


////////////////////////////////////////////////////////////////////////////////
// Contiki Process
////////////////////////////////////////////////////////////////////////////////

PROCESS(zwave_rx_process, "Z-Wave RX process");

PROCESS_THREAD(zwave_rx_process, ev, data)
{
  // Continously poll for incoming messages for the Z-Wave API,
  // that's all the process does, other incoming events are ignored
  PROCESS_POLLHANDLER(zwave_rx_poll());

  PROCESS_BEGIN();

  while (1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
