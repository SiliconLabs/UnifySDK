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
#include "uic_signal_handler.h"
#include <signal.h>
PROCESS(uic_signal_handler_process, "UIC Signal Handler");

#define SIGNAL_INIT 1

static int interrupted = 0;
static void interrupt_handler()
{
  interrupted = 1;
  //Here we are not in any context we can only do polls from here.
  process_poll(&uic_signal_handler_process);

}

static void check_interrupt()
{
  if (interrupted) {
    process_post(PROCESS_BROADCAST, PROCESS_EVENT_EXIT, 0);
  }
}

sl_status_t uic_signal_handler_setup(void)
{
  process_start(&uic_signal_handler_process, NULL);
  return SL_STATUS_OK;
}

PROCESS_THREAD(uic_signal_handler_process, ev, data)
{
  PROCESS_POLLHANDLER(check_interrupt());
  PROCESS_BEGIN()
  while (1) {
    if(ev == PROCESS_EVENT_INIT) {
      process_post(&uic_signal_handler_process,SIGNAL_INIT,0);
    } else if(ev == SIGNAL_INIT) {
      signal(SIGINT, &interrupt_handler);
      signal(SIGTERM, &interrupt_handler);
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
