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
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "contiki-conf.h"

/* Includes from other components */
#include "sl_log.h"

/* Includes from this component */
#include "uic_main_externals.h"
#include "uic_main_externals_internal.h"

#define LOG_TAG "uic_main_externals_platform"

/** @defgroup uic_main_ext_posix Posix-Specific Implementation of the Event Demultiplexer's Internal Interface
 *
 * @ingroup uic_main_ext_int
 */

/** The set of readable file descriptors.
 * @ingroup uic_main_ext_platform
 */
static fd_set fdrs;

/** The highest numbered file descriptor.
 * @ingroup uic_main_ext_platform
 */
static int fd_max = -1;

/* Prototypes for local functions */

/** Set the registered file desriptors before the select call.
 *
 * @ingroup uic_main_ext_platform
 */
static void uic_main_ext_setup(void);

/** Set one registered file desriptor on the \p fdrs set.
 *
 * @ingroup uic_main_ext_platform
 *
 * @param reg The external event registration whose descriptor should be set.
 */
static void
  set_fdr_from_registration(const uic_main_ext_fd_registration_t *reg);

/** Call the registered handlers in \p reg if the fd is set.
 *
 * This function is called from \ref uic_main_ext_registration_iter().
 *
 * @ingroup uic_main_ext_platform
 * @param reg The external event registration whose fd should be checked.
 */
static void
  uic_main_check_registration(const uic_main_ext_fd_registration_t *reg);

/* Local functions. */

static void set_fdr_from_registration(const uic_main_ext_fd_registration_t *reg)
{
  FD_SET(reg->fd, &fdrs);
  if (reg->fd > fd_max) {
    fd_max = reg->fd;
  }
}

static void uic_main_ext_setup(void)
{
  FD_ZERO(&fdrs);

  uic_main_ext_registration_iter(set_fdr_from_registration);

  return;
}

/* Select and handle */

static void
  uic_main_check_registration(const uic_main_ext_fd_registration_t *reg)
{
  if (FD_ISSET(reg->fd, &fdrs)) {
    uic_main_ext_call_handler(reg);
  }
}

/* Exported functions */
void uic_main_ext_select(clock_time_t delay)
{
  struct timeval tv;

  tv.tv_sec  = delay / 1000;
  tv.tv_usec = (delay % 1000) * 1000;

  /* set the fds */
  uic_main_ext_setup();

  int readable_fds = select(fd_max + 1, &fdrs, NULL, NULL, &tv);

  if (readable_fds < 0) {
    /* We cannot really do anything about an error in a generic
     * sense.
     *
     * FIXME-maybe: do we also want to register an error handler for
     * each file descriptor? */
    sl_log_error(LOG_TAG,
                 "Error reading file descriptors: %s (errno %d)\n",
                 strerror(errno),
                 errno);
    for (int ii = 0; ii <= fd_max; ii++) {
      if (FD_ISSET(ii, &fdrs)) {
        sl_log_error(LOG_TAG, "fd: %d\n", ii);
      }
    }
  } else if (readable_fds > 0) {
    uic_main_ext_registration_iter(uic_main_check_registration);
  }
  /* If no file descriptors are ready, just move on. */
}
