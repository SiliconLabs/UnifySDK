/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/**
 * @file attribute_management_process.h
 * @defgroup attribute_management_process  ZigPC Attribute Management Process Handler
 * @ingroup attribute_management
 * @brief API and functions for managing attributes, responsible for storing
 * and reporting on attribute state
 *
 * @{
 *
**/

#ifndef ATTRIBUTE_MANAGEMENT_PROCESS_H
#define ATTRIBUTE_MANAGEMENT_PROCESS_H

#include "sl_status.h"

/**
 * @brief attribute_management_setup - sets up the parts of the software used 
 * by the attribute manager
 *
 * @return the status if the method succeeded and the component was set up
 * correctly
 */
sl_status_t attribute_management_setup(void);

/**
 * @brief attribute_management_shutdown - graceful shuts down all parts of the
 * attribute_management component, expected to be called before the program exits
 *
 * @return an integer representing the success of the operation. 0 for success
 * any other number represents a failure
 *
 */
int attribute_management_shutdown(void);

#endif  //ATTRIBUTE_MANAGEMENT_PROCESS_H

/** @} end attribute_management_process */
