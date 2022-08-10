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

#ifndef UIC_MAIN_EXTERNALS_H
#define UIC_MAIN_EXTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "sys/process.h"
#include "sl_status.h"

/** @defgroup uic_main_ext Unify External Event Demultiplexer
 *
 * @ingroup uic_main
 *
 * This sub-component distributes incoming events from the external
 * interfaces to the interface-handler components.
 *
 * The sub-component is invoked when the central loop detects an
 * incoming external event (e.g., when a file descriptor has data
 * ready to read).
 *
 * \note All components that handle events must be implemented as
 * Contiki processes.
 *
 * There are two ways this sub-component can pass on an event to the
 * handler:
 *
 * - In the simplest scenario, one Contiki process
 * handles all events on the interface directly.  In this case, the
 * process should register its file descriptor of the interface with
 * the \ref uic_main_ext.  The process will then be polled when an
 * event happens on the file descriptor.
 *
 * - In other cases, e.g., the Contiki standard input handler,
 * incoming data is "pre-screened" by a reader function and dropped if
 * it is irrelevant. So, the process is only polled if the data is
 * relevant.
 * In this case, the component should only register an event handler
 * function with the \ref uic_main_ext.
 *
 * \note An event handler function must only execute the minimal
 * handling to import the data into the Unify Main process.
 * The only Contiki process feature it is allowed to use is *one* process_poll.
 *
 * \note If a component registers both a function and a process,
 * the process will be polled if the function returns true.
 */

/** Maximum number of file descriptors we can select on.
 *
 * @ingroup uic_main_ext
 *
 * \note This is not the same as \ref fd_max.
 */
#define UIC_MAIN_EXT_MAX_FDS 5

/** Type of an event handler function.
 * @ingroup uic_main_ext
 */
typedef bool (*uic_ext_event_handler_t)(int);

/** Register an event handler for file descriptor readable events.
 *
 * @ingroup uic_main_ext
 *
 * If \p ev_hndl_func is set, the function will be called when an event
 * is detected on the file descriptor \p fdr.  The function should
 * only read the event from the file descriptor to an internal
 * storage. If any processing is required, a process should be
 * polled.
 *
 * If \p ev_hndl_process is set, the process will be polled when an
 * event is detected on the file descriptor \p fdr.  If the process
 * should not always be polled, the ev_hndl_func can poll the process,
 * and the ev_hndl_process argument should be NULL.
 *
 * @param fdr A readable file descriptor.
 * @param ev_hndl_func The function to call if there is an event on \p fdr.
 * @param ev_hndl_process The process to poll if there is an event on \p fdr.
 * @return true if the registration succeeded, false otherwise.
 */
sl_status_t uic_main_ext_register_rfd(int fdr,
                                      uic_ext_event_handler_t ev_hndl_func,
                                      struct process *ev_hndl_process);

/** Remove a file descriptor from file descriptor readable events.
 *
 * @ingroup uic_main_ext
 *
 * \param fdr The file descriptor that should be unregistered.
 *
 * @return true if the removal succeeded, false otherwise.
 */
sl_status_t uic_main_ext_unregister_rfd(int fdr);

#ifdef __cplusplus
}
#endif

#endif
