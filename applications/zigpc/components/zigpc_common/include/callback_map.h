/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

/*******************************************************************************
 * Callback Map
 *
 * A callback map implementation. Stores a callback function associated with a
 * given character string. Uses a list of map pairs and a size to store all
 * relevant information. MAX_MAP_SIZE and MAX_KEY_SIZE are used to limit the
 * memory operations to better comply with MISRA standards.
 *
 ******************************************************************************/

#ifndef ZIGPC_SIMPLE_MAP_H
#define ZIGPC_SIMPLE_MAP_H

#include "sl_status.h"
#include "base_map.h"

/**
 * create_callback_map
 * \brief Create a callback map based on a callback list of pairs.
 *
 * Creates a callback map. All contents are initially 0 or NULL. Consists of an array of pairs and the size of the array.
 * The MAX_MAP_SIZE is used for memory bounds of the array. All operations on this callback map are unoptimized. Insert
 * and retrieval are guaranteed.
**/
callback_map_t create_callback_map(void);

/**
 * insert_callback_map
 * \brief Insert a new key value pair into the callback map.
 *
 * Accepts a key-value (callback) pair. Inserts it into the last available space in the map.
 * If the key does not exist, it will increase the size of the map by 1. If the key exists,
 * it will replace the callback at that location. Returns SL_STATUS_FAILURE if it fails, SL_STATUS_OK on success
 *
 * @param map - the map where we wish to insert the key-value. Should not be null.
 * @param key - the key of a pair associated with a callback
 * @param callback - the function accessed by the key
 *
 * @return an SL_STATUS representing the success of the operation. SL_STATUS_OK on success, SL_STATUS_FAILURE on failure
 *         Fails if any of the pointers are NULL. Fails if there is no longer space in the map
**/
sl_status_t insert_callback_map(callback_map_t *map,
                                const char *key,
                                zcl_command_t callback);

/**
 * insert_pair_callback_map
 * \brief Insert a map pair directly into a given map
 *
 * Accepts a pair to directly insert into a map. Inserts into the last available space in the map.
 * Fails if either of the pointers are NULL or under the same conditions as the insert_callback_map
 * function (no available space, key or callback is NULL). Will replace the callback if the key
 * already exists. Returns SL_STATUS_OK on success, SL_STATUS_FAILURE or SL_STATUS_NULL_POINTER on failure.
 *
 * @param map - the map where we wish to insert the key-value - should not be null
 * @param pair - the pair containing a key-value that we wish to insert - all values should be
 *               populated
 *
 * @return an SL_STATUS representing the success of the operation. SL_STATUS_OK on success, SL_STATUS_FAILURE
 *         or SL_STATUS_NULL_POINTER on failure
**/
sl_status_t insert_pair_callback_map(callback_map_t *map,
                                     const zcl_key_value_t *pair);

/**
 * remove_callback_map
 * \brief Remove a pair with a given key from a map.
 *
 * Accepts a key and removes the pair with this key from a provided map. It is assumed that only one
 * key will exist in the map. If this assumption fails, it will remove the first found key. This
 * function will fail if the provided map, or key is NULL, or if the pair list itself is invalid. If
 * the key does not exist in the map, this function will not modify the map and return a success.
 * Returns a 0 for success and -1 on failure.
 *
 * @param map - the map to be modified
 * @param key - the key corresponding to the pair to be removed
 *
 * @return an SL_STATUS representing the success of the function. SL_STATUS_OK on succes, SL_STATUS_NULL_POINTER
 *         on failure
**/
sl_status_t remove_callback_map(callback_map_t *map, const char *key);

/**
 * retrieve_callback_map
 * \brief Retrieve the callback stored in the callback map by its key.
 *
 * Accepts a key and retrieves the callback stored by that key. If the key is found,
 * this function will return with a stored callback and will populate the given index
 * with the location where the pair is stored in the pair_list of the supplied map.
 * If the key or the map are NULL or invalid, or if the key is not found
 * in the map, the index will be populated with -1 signifying a failure. In this case,
 * the callback will point to NULL.
 *
 * @param map - the map to search for a key-value pair
 * @param key - the key to looking up the key-value pair
 * @param index - populated index of the found pair in the map. -1 if an error occurs
 *
 * @return the callback associated with the key. NULL if an error occurs
**/
zcl_command_t
  retrieve_callback_map(const callback_map_t *map, const char *key, int *index);

#endif  //ZIGPC_SIMPLE_MAP_H
