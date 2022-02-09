/* # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

/* Generic includes */
#include <stdbool.h>

/* contiki includes */
#include "process.h"
#include "list.h"
#include "memb.h"

/* Includes from other components */
#include "sl_log.h"
#include "sl_status.h"

/* Includes from this component */
#include "uic_main_externals.h"
#include "uic_main_externals_internal.h"

#define LOG_TAG "uic_main_externals"

/** Memory for readable file descriptors.
 *
 * @ingroup uic_main_ext
 */
MEMB(fd_registration_slots,
     uic_main_ext_fd_registration_t,
     UIC_MAIN_EXT_MAX_FDS);

/** Currently registered readable file descriptors.
 * @ingroup uic_main_ext
 */
LIST(fd_registrations);

/* Exposed functions */
sl_status_t uic_main_ext_register_rfd(int fdr,
                                      uic_ext_event_handler_t ev_hndl_func,
                                      struct process *ev_hndl_process)
{
  uic_main_ext_fd_registration_t *reg;

  for (reg = list_head(fd_registrations); reg != NULL;
       reg = list_item_next(reg)) {
    if (reg->fd == fdr) {
      sl_log_critical(LOG_TAG, "Attempt to double-register fdr %d", fdr);
      return SL_STATUS_FAIL;
    }
  }

  reg = memb_alloc(&fd_registration_slots);
  if (reg == NULL) {
    return SL_STATUS_FAIL;
  }

  reg->fd             = fdr;
  reg->ev_hndl_func   = ev_hndl_func;
  reg->owning_process = ev_hndl_process;
  list_add(fd_registrations, reg);
  return SL_STATUS_OK;
}

sl_status_t uic_main_ext_unregister_rfd(int fdr)
{
  uic_main_ext_fd_registration_t *reg;

  for (reg = list_head(fd_registrations); reg != NULL;
       reg = list_item_next(reg)) {
    if (reg->fd == fdr) {
      list_remove(fd_registrations, reg);
      memb_free(&fd_registration_slots, reg);
      return SL_STATUS_OK;
    }
  }
  sl_log_warning(LOG_TAG,
                 "Failed to remove fdr registration for %d: not found",
                 fdr);
  return SL_STATUS_FAIL;
}

void uic_main_ext_registration_iter(fd_handler_t iter_function)
{
  uic_main_ext_fd_registration_t *reg;

  for (reg = list_head(fd_registrations); reg != NULL;
       reg = list_item_next(reg)) {
    iter_function(reg);
  }
}

/* Component-local functions */

void uic_main_ext_call_handler(const uic_main_ext_fd_registration_t *reg)
{
  bool poll_requested = true;

  /*sl_log_debug(LOG_TAG,
         "Distributing event for fd %d to %s\n",
         reg->fd,
         (reg->owning_process)
         ? PROCESS_NAME_STRING(reg->owning_process)
         : "function");*/
  if (reg->ev_hndl_func) {
    poll_requested = (reg->ev_hndl_func)(reg->fd);
  }
  if (reg->owning_process) {
    if (poll_requested) {
      process_poll(reg->owning_process);
    }
  }
}
