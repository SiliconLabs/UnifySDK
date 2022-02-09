/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#ifndef RUST_CONTIKI_FIXT_H
#define RUST_CONTIKI_FIXT_H
#include "sl_status.h"

/**
 * @brief shutdown sequence for applications that are built on top of contiki.
 * in particular Rust's task executor.
 * 
 * @return always return SL_STATUS_OK
 * 
 */
int rust_contiki_teardown();

#endif