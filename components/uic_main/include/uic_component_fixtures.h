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

#ifndef UIC_COMPONENT_FIXTURES_H
#define UIC_COMPONENT_FIXTURES_H

#include <stdbool.h>
#include "sl_status.h"

/** \defgroup uic_fixt Unify Component Fixture Set Up and Tear Down
 *
 * \ingroup uic_main
 *
 * A fixture is the environment a component runs in.  So it must be
 * set up before the component can work and maybe also cleaned up
 * afterwards.
 *
 * In the Unify SDK, most of what the components need are provided by other
 * components or by the Contiki OS.
 * However, there are key features that are needed by a few of the
 * components and that are not provided by Unify itself.  This may
 * relate to external interfaces or dynamic memory.
 *
 * If a component needs to set up and/or tear down such a resource, it
 * can install an initializer (the set-up function).
 * If the component is in the Unify library, this shall be done in \ref
 * uic_fixt_setup_steps_list array and a tear-down function in the \ref
 * uic_fixt_shutdown_steps_list array.
 * If the component is outside the Unify library an array of
 * \ref uic_fixt_setup_step_t and \ref uic_fixt_shutdown_step_t
 * are added to \ref uic_main function call.
 *
 * A tear-down function may also be useful for releasing allocated resources.
 *
 * <b> Order of Execution </b>
 *
 * The set-up functions are executed as the first thing after the
 * configuration settings have been read and the logging system
 * has been initialized.
 *
 * Tear-down functions are run after the \ref uic_main_loop() has
 * terminated and after the Contiki OS is terminated, i.e., there are
 * no processes and no Contiki timers.
 *
 * \note If any of the set-up functions fail, the whole system will
 * fail to start and the tear-down functions will not be called.
 */

/** Type of a component fixture set-up function.
 *
 *  \ingroup uic_fixt
 *
 * The function should returns SL_STATUS_FAIL if the Unify GW cannot start,
 * SL_STATUS_OK otherwise.
 */
typedef sl_status_t (*uic_fixt_setup_t)(void);

/** Type of a component tear-down function.
 *
 *  \ingroup uic_fixt
 *
 */
typedef int (*uic_fixt_shutdown_t)(void);

#endif
