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
 * Hash Map
 *
 * A simple map implementation. Stores a callback function associated with a
 * given character string. Uses a list of map pairs and a size to store all
 * relevant information. MAX_MAP_SIZE and MAX_KEY_SIZE are used to limit the
 * memory operations to better comply with MISRA standards.
 *
 ******************************************************************************/

#ifndef ZIGPC_HASH_MAP_H
#define ZIGPC_HASH_MAP_H

#include "base_map.h"
#include "sl_status.h"

typedef unsigned int (*hash_function_t)(const char *key);

struct hash_map {
  callback_map_t base;
  hash_function_t hash_func;
};
typedef struct hash_map hash_map_t;

/**
 * hash_fnv1a
 * \brief An implementation of the FNV-1A hashing algorithm used as the hashing function of this map
 *
 * Determines the index corresponding to the key accepted as input. This index can be used to store
 * key-value pairs in the hash map.
 *
 * @param key - a character string representing the key where to store items in a map / array
 *
 * @return the index where to store a key-value pair in a map / array
**/
unsigned int hash_fnv1a(const char *key);

/**
 * create_hash_map
 * \brief Create a simple map based on a simple list of pairs.
 *
 * Creates a simple map. All contents are initially 0 or NULL. Consists of an array of pairs and the size of the array.
 * The MAX_MAP_SIZE is used for memory bounds of the array. All operations on this simple map are unoptimized. Insert
 * and retrieval are guaranteed.
**/
hash_map_t create_hash_map();

/**
 * insert_hash_map
 * \brief Insert a new key value pair into the simple map.
 *
 * Accepts a key-value (callback) pair. Inserts it into the last available space in the map.
 * If the key does not exist, it will increase the size of the map by 1. If the key exists,
 * it will replace the callback at that location. Returns SL_STATUS_FAIL or SL_STATUS_NULL_POINTER if it fails, SL_STATUS_OK on success.
 *
 * @param map - the map where we wish to insert the key-value. Should not be null.
 * @param key - the key of a pair associated with a callback
 * @param callback - the function accessed by the key
 *
 * @return an sl_status_t representing the success of the operation. SL_STATUS_OK on success,
 * SL_STATUS_FAIL / SL_STATUS_NULL_POINTER on failure. Fails if any of the pointers are NULL.
 * Fails if there is no longer space in the map
**/
sl_status_t
  insert_hash_map(hash_map_t *map, const char *key, zcl_command_t callback);

/**
 * insert_pair_hash_map
 * \brief Insert a map pair directly into a given map.
 *
 * Accepts a pair to directly insert into a map. Inserts into the last available space in the map
 * Fails if either of the pointers are NULL or under the same conditions as the insert_hash_map
 * function (no available space, key or callback is NULL). Will replace the callback if the key
 * already exists. Returns a 0 on success, a -1 on failure.
 *
 * @param map - the map where we wish to insert the key-value - should not be null
 * @param pair - the pair containing a key-value that we wish to insert - all values should be
 *               populated
 *
 * @return an sl_status_t representing the success of the operation. STATUS_OK on success,
 * STATUS_FAIL on failure.
**/
sl_status_t insert_pair_hash_map(hash_map_t *map, const zcl_key_value_t *pair);

/**
 * remove_hash_map
 * \brief Remove a pair with a given key from a map.
 *
 * Accepts a key and removes the pair with this key from a provided map. It is assumed that only one
 * key will exist in the map. If this assumption fails, it will remove the first found key. This
 * function will fail if the provided map or the key is NULL, or if the pair list itself is invalid. If
 * the key does not exist in the map, this function will not modify the map and return a success.
 * Returns a 0 for success and -1 on failure.
 *
 * @param map - the map to be modified
 * @param key - the key corresponding to the pair to be removed
 *
 * @return an sl_status_t representing the status of the operation. SL_STATUS_OK on success, SL_STATUS_FAIL
 * on failure
**/
sl_status_t remove_hash_map(hash_map_t *map, const char *key);

/**
 * retrieve_hash_map
 * \brief Retrieve the callback stored in the simple map by its key.
 *
 * Accepts a key and retrieves the callback stored by that key. If the key is found,
 * this function will return with stored callback and will populate the given index
 * with the location where the pair is stored in the pair_list of the supplied map.
 * If the key or the map are NULL or invalid, or if the key is not found
 * in the map, the index will be populated with -1 signifying a failure. In this case,
 * the callback will point to NULL.
 *
 * @param map - the map to search for a key-value pair
 * @param key - the key to looking up the key-value pair
 *
 * @return the callback associated with the key. NULL if an error occurs
**/
zcl_command_t retrieve_hash_map(const hash_map_t *map, const char *key);

#endif  //ZIGPC_HASH_MAP_H
