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
#include "zwave_s2_process.h"

#include "zwave_controller_transport.h"
#include "zwave_controller_transport_internal.h"
#include "sl_log.h"
#include "sys/etimer.h"
#include "sys/process.h"
#include "zwave_controller.h"
#include "zwave_s2_internal.h"
#include "zwave_s2_keystore_int.h"
#include "zwave_s2_network.h"
#include "zwave_s2_transport.h"

#define LOG_TAG "zwave_s2_process"

static struct etimer s2_timer;
static struct etimer s2_inclusion_timer;

PROCESS(zwave_s2_process, "zwave_s2_process");
enum { S2_TIMER, S2_TIMER_STOP, S2_INCLUSION_TIMER, S2_INCLUSION_TIMER_STOP };

uint8_t s2_inclusion_set_timeout(struct S2 *ctxt, uint32_t interval)
{
  (void)ctxt;
  sl_log_debug(LOG_TAG,
               "Setting S2 Inclusion timeout to: %i ms\n",
               interval * 10);
  process_post(&zwave_s2_process,
               S2_INCLUSION_TIMER,
               (void *)(intptr_t)(interval * 10));
  return 0;
}

void s2_inclusion_stop_timeout(void)
{
  process_post(&zwave_s2_process, S2_INCLUSION_TIMER_STOP, 0);
}

void S2_stop_timeout(struct S2 *ctxt)
{
  (void)ctxt;
  process_post(&zwave_s2_process, S2_TIMER_STOP, 0);
}

void S2_set_timeout(struct S2 *ctxt, uint32_t interval)
{
  (void)ctxt;
  sl_log_debug(LOG_TAG, "Setting S2 Send Data timeout to: %i ms\n", interval);
  process_post(&zwave_s2_process, S2_TIMER, (void *)(intptr_t)interval);
}

static void zwave_s2_on_new_network_entered(zwave_home_id_t home_id,
                                            zwave_node_id_t node_id,
                                            zwave_keyset_t granted_keys,
                                            zwave_kex_fail_type_t kex_fail_type)
{
  (void)home_id;
  (void)node_id;
  (void)granted_keys;
  (void)kex_fail_type;
  zwave_s2_network_init();
}

static void zwave_s2_init()
{
  static zwave_controller_callbacks_t callbacks
    = {.on_new_network_entered     = zwave_s2_on_new_network_entered,
       .on_multicast_group_deleted = zwave_s2_on_on_multicast_group_deleted};

  zwave_controller_register_callbacks(&callbacks);

  static zwave_controller_transport_t transport = {
    .priority          = 3,
    .command_class     = COMMAND_CLASS_SECURITY_2,
    .version           = COMMAND_CLASS_SECURITY_2_VERSION,
    .send_data         = zwave_s2_send_data,
    .abort_send_data   = zwave_s2_abort_send_data,
    .on_frame_received = zwave_s2_on_frame_received,
  };
  zwave_controller_transport_register(&transport);

  zwave_s2_keystore_init();
  zwave_s2_network_init();
  zwave_s2_transport_init();
}

PROCESS_THREAD(zwave_s2_process, ev, ev_data)
{
  PROCESS_BEGIN();
  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      etimer_stop(&s2_timer);
      etimer_stop(&s2_inclusion_timer);
      zwave_s2_init();
    } else if (ev == PROCESS_EVENT_TIMER) {
      if (ev_data == &s2_inclusion_timer) {
        sl_log_debug(LOG_TAG, "S2 inclusion timer has now expired\n");
        s2_inclusion_notify_timeout(s2_ctx);
      } else if (ev_data == &s2_timer) {
        sl_log_debug(LOG_TAG, "S2 send data timer has now expired\n");
        S2_timeout_notify(s2_ctx);
      }
    } else if (ev == S2_TIMER) {
      etimer_set(&s2_timer, (intptr_t)ev_data);
    } else if (ev == S2_INCLUSION_TIMER) {
      etimer_set(&s2_inclusion_timer, (intptr_t)ev_data);
    } else if (ev == S2_INCLUSION_TIMER_STOP) {
      etimer_stop(&s2_inclusion_timer);
    } else if (ev == S2_TIMER_STOP) {
      etimer_stop(&s2_timer);
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
