/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file zpc_datastore_fixt.h
 * @defgroup zpc_datastore_fixture ZPC Config Fixture
 * @ingroup zpc_datastore
 * @brief ZPC Configuration fixture.
 *
 * Fixture to initialize the zpc_datastore from uic_main,
 * to be used in \ref zpc_datastore
 *
 * @{
 */

#ifndef ZPC_DATASTORE_FIXT_H
#define ZPC_DATASTORE_FIXT_H

#include "sl_status.h"
/**
 * @brief Fixture for setting up the zpc_datastore component.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t zpc_datastore_fixt_setup();

/** @} end of zpc_datastore*/
#endif // ZPC_DATASTORE_FIXT_H
