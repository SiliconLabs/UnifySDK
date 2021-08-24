/* # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

#ifndef UIC_COMPONENT_FIXTURES_INTERNAL_H
#define UIC_COMPONENT_FIXTURES_INTERNAL_H

#include <stdbool.h>

#include "uic_component_fixtures.h"
#include "uic_main.h"

/**
 * @ingroup uic_fixt
 * @{
 */
/** @defgroup uic_fixt_int Internal Fixture Interfaces.
 *
 * Internal part of \ref uic_fixt.
 */
/** @} */

extern uic_fixt_setup_step_t *uic_fixt_setup_steps;

extern uic_fixt_shutdown_step_t *uic_fixt_shutdown_steps;

/** Loop over the set-up functions.
 * \ingroup uic_fixt_int
 *
 * \return false if any of the set-up steps return false.
 */
bool uic_fixt_setup_loop(const uic_fixt_setup_step_t *fixt_app_setup);

/** Loop over the tear-down steps.
 *
 * \ingroup uic_fixt_int
 *
 * @return Number of issues detected during shutdown.
 */
int uic_fixt_shutdown_loop(const uic_fixt_shutdown_step_t *fixt_app_shutdown);
#endif
