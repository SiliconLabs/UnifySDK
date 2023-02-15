#include "request_poller.h"
#include <stdio.h>

#include "contiki.h"
PROCESS_NAME(request_poller_process);
static void(*process_contiki_poll_cb)() = NULL;

void register_poll_handler(void(*handler)()) {
  process_contiki_poll_cb = handler;
}

void request_poller_poll_request() {
  process_poll(&request_poller_process);
}

PROCESS(request_poller_process, "request_poller");

PROCESS_THREAD(request_poller_process, ev, data)
{
  (void)(data);
  PROCESS_BEGIN();

  while (1) {
    if (ev == PROCESS_EVENT_INIT || 
          ev == PROCESS_EVENT_POLL) {
      if (process_contiki_poll_cb) {
        process_contiki_poll_cb();
      }
    }
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}


sl_status_t initialize_request_poller_process() {
  process_start(&request_poller_process, NULL);
  return SL_STATUS_OK;
}
