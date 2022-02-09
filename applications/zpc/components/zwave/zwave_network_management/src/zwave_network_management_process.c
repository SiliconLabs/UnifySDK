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
//#include "zwave_nm_process.h"

#include "process.h"
#include "zwave_network_management.h"
#include "nm_state_machine.h"
#include "sl_log.h"

#define LOG_TAG "zwave_network_managment_process"

PROCESS(zwave_network_management_process, "zwave_network_management_process");

PROCESS_THREAD(zwave_network_management_process, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      // The fixture should have initialized the network management.

    } else if (ev == PROCESS_EVENT_EXIT) {
      //
    } else if (ev == PROCESS_EVENT_TIMER) {
      nm_fsm_post_event(NM_EV_TIMEOUT, data);
    } else if (ev
               < NM_EV_MAX) {  //TODO what if PROCESS_EVENT* and NM_EV_* collide
      if (NM_EV_MAX > PROCESS_EVENT_NONE) {
        sl_log_error(LOG_TAG,
                     "Eror: NM_EV_MAX is bigger than PROCESS_EVENT_NONE\n");
      }
      nm_fsm_post_event(ev, data);
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
