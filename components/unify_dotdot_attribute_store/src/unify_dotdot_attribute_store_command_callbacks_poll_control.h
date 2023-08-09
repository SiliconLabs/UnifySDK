/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @defgroup unify_dotdot_attribute_store_command_callbacks_poll_control PollControl Cluster Mapper
 * @ingroup dotdot_mapper
 * @brief Maps Level PollControl incoming Commands to attribute modifications.
 *
 * This module is used to register callbacks for incoming PollControl Cluster Commands
 * and update the corresponding ZCL attributes in the @ref attribute_store .
 * @{
 */

#ifndef UNIFY_DOTDOT_ATTRIBUTE_STORE_COMMAND_CALLBACKS_POLL_CONTROL_H
#define UNIFY_DOTDOT_ATTRIBUTE_STORE_COMMAND_CALLBACKS_POLL_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intitialise the PollControl command mapping
 */
void poll_control_cluster_mapper_init();

#ifdef __cplusplus
}
#endif

#endif  //UNIFY_DOTDOT_ATTRIBUTE_STORE_COMMAND_CALLBACKS_POLL_CONTROL_H
/** @} end unify_dotdot_attribute_store_command_callbacks_poll_control */
