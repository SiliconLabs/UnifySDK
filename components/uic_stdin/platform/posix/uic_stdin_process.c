/*
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

// // Own header file
#include "uic_stdin_process.h"

// Generic includes
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>

// Contiki includes
#include "process.h"

// Third party
#include "editline/readline.h"

// Includes from other components
#include "sl_log.h"
#include "uic_main_externals.h"

// Includes from this component
#include "uic_stdin_command_handler.h"

// Log
#define LOG_TAG "uic_stdin_process"

// Globals
PROCESS(uic_stdin_process, "Unify Stdin");

static pthread_t stdin_thread;
pthread_cond_t cond   = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//File descriptors used send data from the readline thread to the contiki
// process.
static int fildes[2];
static bool thread_running;

static char **completer(const char *text, int start, int end)
{
  char **matches               = NULL;
  rl_attempted_completion_over = 1;
  /* If this word is at the start of the line, then it is a command
       to complete. */
  if (start == 0) {
    matches = rl_completion_matches(text, uic_stdin_command_generator);
  }
  return matches;
}

//Handle CTRL-C
static void handle_signals(int signo)
{
  if (signo == SIGINT) {
    sl_log_debug(LOG_TAG, "You pressed Ctrl+C\n");
    write(fildes[1], "exit", 5);
  }
}

//Thread loop for handling the console input.
static void *readline_loop(void *d)
{
  char *buf;
  rl_instream                      = (FILE *)d;
  rl_attempted_completion_function = completer;
  rl_initialize();
  rl_cleanup_after_signal();
  pthread_mutex_trylock(&mutex);
  signal(SIGINT, handle_signals);

  while (thread_running) {
    buf = readline(get_prompt_string());

    if (buf) {
      int len = strlen(buf);
      if (len > 0) {
        // Closing thread when exit command is executed
        if (strcmp(buf, "exit") == 0) {
          thread_running = false;
        }
        add_history(buf);
        //Send the buffer, +1 to include the 0 termination.
        write(fildes[1], buf, len + 1);
        //Wait for the contiki process to process the line
        pthread_cond_wait(&cond, &mutex);
      }
      free(buf);
    } else {
      //CTRL-D was pressed
      write(fildes[1], "exit", 5);
    }
  }

  pthread_mutex_unlock(&mutex);
  sl_log_debug(LOG_TAG, "stdin thread exited\n");
  return NULL;
}

static void uic_stdin_poll()
{
  char my_line[512];
  if (read(fildes[0], my_line, sizeof(my_line)) > 0) {
    uic_stdin_handle_command(my_line);
  }
  pthread_cond_broadcast(&cond);
}

PROCESS_THREAD(uic_stdin_process, ev, data)
{
  PROCESS_POLLHANDLER(uic_stdin_poll());
  PROCESS_BEGIN();
  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      uic_stdin_init();
      if (isatty(STDIN_FILENO)) {
        //We use a pipe to send line string from readline thread to the contiki
        //process.
        pipe(fildes);
        uic_main_ext_register_rfd(fildes[0], 0, &uic_stdin_process);

        sl_log_debug(LOG_TAG, "Registering stdin fileno %d\n", fildes[0]);
        // As readlines support for non-blocking io is very broken, we run the
        // console parset in a separate thread.
        thread_running = true;
        pthread_create(&stdin_thread, 0, readline_loop, stdin);
      } else {
        sl_log_warning(LOG_TAG,
                       "stdin is not a tty, input will not be handled.\n");
      }
    } else if (ev == PROCESS_EVENT_EXIT) {
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}

// Process setup fixt
sl_status_t uic_stdin_setup(void)
{
  process_start(&uic_stdin_process, NULL);
  return SL_STATUS_OK;
}

// Teardown uic CLI again
int uic_stdin_teardown()
{
  thread_running = false;
  rl_reset_terminal("");
  return (int)SL_STATUS_OK;
}
