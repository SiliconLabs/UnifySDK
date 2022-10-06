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

sl_status_t uic_init(const struct uic_fixt_setup_step *fixt_app_setup,
                     int argc,
                     char **argv,
                     const char *version);

#ifdef __cplusplus
}
#endif
#endif
