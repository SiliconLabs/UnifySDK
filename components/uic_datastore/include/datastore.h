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

/**
 * @defgroup unify_datastore Unify Datastore
 * @ingroup unify_components
 * @brief Persistent Unify Datastore
 *
 * Persistent datastore, that allows for storing key-value pair.
 * The users of this component must handle updating own values if format change
 * between versions.
 * Keys are zero terminated strings
 *
 * @{
 */

#ifndef DATASTORE_H
#define DATASTORE_H

#include <inttypes.h>
#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef const char *datastore_key_t;

/**
 * @brief Initialize the Datastore component.
 *
 * If the datastore init fails,
 * then there is no reason to call @ref datastore_teardown()
 * If it initialized successful,
 * then @ref datastore_teardown() shall be called before exiting
 *
 * @param database_path Path to the database file,
 *                      The directory the path must exist, or init will fail
 *                      (or ":memory:" to use in memory database)
 *
 * @return sl_status_t
 *                    SL_STATUS_OK for success
 *                    SL_STATUS_FAIL if an error occurred
 */

sl_status_t datastore_init(const char *database_path);

/**
 * @brief start a datastore transaction
 * 
 * With transactions, datastore calls will be cached so that they can be commited
 * in one go. This increases the throughput of data.
 * Only one transaction at the time can be open, otherwise `SL_STATUS_ABORT` is returned.
 * 
 * 
 *  * @return sl_status_t
 *                    SL_STATUS_OK for success
 *                    SL_STATUS_IN_PROGRESS if there is an ongoing transaction
 *                    SL_STATUS_FAIL if an error occurred
 **/
sl_status_t datastore_start_transaction();

/**
 * @brief commit an open transaction
 * 
 *  * @return sl_status_t
 *                   SL_STATUS_OK for success
 *                   SL_STATUS_FAIL if an error occurred
**/
sl_status_t datastore_commit_transaction();

/**
 * @brief Tear down the Datastore component.
 *
 * Shall be called before exiting the application to close database file
 * @return sl_status_t SL_STATUS_OK for success, SL_STATUS_ERROR for failure
 */
sl_status_t datastore_teardown();

/**
 * @brief Store integer in the persistent datastore.
 *
 * @param key Key to store value under, must be unique
 * @param value Value to store under key
 * @return sl_status_t SL_STATUS_OK if successful, SL_STATUS_FAIL if failure
 */
sl_status_t datastore_store_int(datastore_key_t key, int64_t value);

/**
 * @brief Fetch integer from the persistent datastore.
 *
 * @param key Key to fetch value under
 * @param value Destination for the key
 * @return sl_status_t
 *            SL_STATUS_OK if successful,
 *            SL_STATUS_NOT_FOUND if key is not found,
 *            SL_STATUS_FAIL if other failure,
 */
sl_status_t datastore_fetch_int(datastore_key_t key, int64_t *value);

/**
 * @brief Store array in the persistent datastore.
 *
 * NOTE: If a record already exists for the given key, the stored value will be
 * overwritten by the value provided.
 *
 * NOTE: If casting larger data types into the array, the component doing this
 * shall be able to handle different endianess, as because of back-up/restore
 * it shall be supported to move the database from Big Endian to Little Endian
 * and vice versa.
 * For uint8_t arrays there are no issues, as they are unaffected by endianess
 *
 * @param key Key to store value under, must be unique
 * @param value Value to store under key
 * @param size Size of value array
 * @return sl_status_t SL_STATUS_OK if successful, SL_STATUS_FAIL if failure
 */
sl_status_t datastore_store_arr(datastore_key_t key,
                                const uint8_t *value,
                                unsigned int size);

/**
 * @brief Fetch array from the persistent datastore.
 *
 * @param key Key to fetch value under
 * @param value Destination for the key
 * @param size In: the size of value, Out: Number of bytes written to value
 * @return sl_status_t
 *            SL_STATUS_OK if successful,
 *            SL_STATUS_NOT_FOUND if key is not found,
 *            SL_STATUS_WOULD_OVERFLOW if fetched data is larger than size
 *            SL_STATUS_FAIL if other failure,
 */
sl_status_t
  datastore_fetch_arr(datastore_key_t key, uint8_t *value, unsigned int *size);

/**
 * @brief Check if the datastore contains an array value for given key.
 *
 * @param key Key to check if is in the datastore
 * @return true if key is present in the datatore else false
 */
bool datastore_contains_arr(datastore_key_t key);

/**
 * @brief Check if the datastore contains an int value for given key.
 *
 * @param key Key to check if is in the datastore
 * @return true if key is present in the datatore else false
 */
bool datastore_contains_int(datastore_key_t key);

#ifdef __cplusplus
}
#endif

#endif  //DATASTORE_H
/** @} end datastore */
