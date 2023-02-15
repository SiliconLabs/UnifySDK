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

#ifndef UIC_MAIN_INIT_H
#define UIC_MAIN_INIT_H

#include "stdbool.h"
/**
 * @addtogroup uic_main
 */
struct uic_fixt_setup_step;
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read in the configuration parameters. Initialize and start up contiki
 * OS and start up the Unify processes.
 *
 * @param fixt_app_setup    array of fixtures for the program.
 * @param argc              argc for the running program
 * @param argv              argv for the running program
 * @param version
 * @returns sl_status_t
 */
sl_status_t uic_init(const struct uic_fixt_setup_step *fixt_app_setup,
                     int argc,
                     char **argv,
                     const char *version);

#ifdef __cplusplus
}
#endif
#endif
