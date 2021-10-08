#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "platform_exec.h"
#include "sl_log.h"

#define LOG_TAG "platform_exec"

void platform_exec(const char *file,
                   char *const *args,
                   char *const *environment)
{
  pid_t pid = 0;
  /*
   * Unless the signal for child events are ignored the parent process
   * must call wait() to wait for the child to exit to avoid zombie/defunct
   * child processes. We do not want to delay the parent by waiting for the
   * child script to exit.
   *
   * NB: The child will generally inherit the ignored signals. POSIX does not
   * clearly state if SIGCHLD is inherited, so we'll reset the handler
   * to default in the child to be safe.
   *
   * NB: The ignored signal is a process global setting - if we ever need to
   * handle child signals in the zipgateway this function must be changed.
   */

  if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
    sl_log_error(LOG_TAG,
                 "Failed to ignore signals from %s process. Call to errno=%d\n",
                 file,
                 errno);
  }

  pid = fork();

  switch (pid) {
    case -1: /* Fork failed */

      sl_log_error(LOG_TAG,
                   "Cannot spawn process for %s. Call to fork() "
                   "failed. errno=%d\n",
                   file,
                   errno);
      break;

    case 0: /* Child process */

      /* This is now the child process running here. */

      /* Restore the child signal handler for the child.
       * (potential errors are ignored - there's currently nowhere to
       * log the error from the child)
       */
      signal(SIGCHLD, SIG_DFL);

      /*
       * Invoke the external script.
       * On success execve() will never return here (the new process image takes over).
       */

      if (execve(file, args, environment) == -1) {
        sl_log_error(LOG_TAG,
                     "Error executing %s: %s (errno %d)\n",
                     file,
                     strerror(errno),
                     errno);
      }
      /* We should never get here. If we do execev failed. */

      _Exit(EXIT_FAILURE);
      break;

    default: /* Parent process */

      /* Nothing more to do here. Back to the usual business */
      break;
  }
}
