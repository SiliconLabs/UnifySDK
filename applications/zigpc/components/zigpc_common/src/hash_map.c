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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "hash_map.h"

#define FNV1A_PRIME \
  16777619  //This is a special prime required for 32bit FNV1A hashing
#define FNV1A_OFFSET \
  2166136261  //This is a special offset that guarantees that FNV1A is unique

hash_map_t create_hash_map()
{
  hash_map_t new_map;
  new_map.base.size = 0;
  new_map.hash_func = hash_fnv1a;

  int i;
  for (i = 0; i < MAX_MAP_SIZE; i++) {
    new_map.base.pair_list[i].command.cluster_id = ZCL_INVALID_CLUSTER_ID;
    new_map.base.pair_list[i].command.command_id = ZCL_INVALID_COMMAND_ID;
  }

  return new_map;
}

unsigned int hash_fnv1a(const char *key)
{
  unsigned int hash = FNV1A_OFFSET;

  if (key != NULL) {
    unsigned int i = 0;
    char data      = key[i];

    while ((data != '\0') && (i < MAX_KEY_SIZE)) {
      data = key[i];
      hash = hash ^ data;
      hash = hash * FNV1A_PRIME;

      i = i + 1;
    }
  }

  hash = hash % MAX_MAP_SIZE;
  return hash;
}

sl_status_t
  insert_hash_map(hash_map_t *map, const char *key, zcl_command_t command)
{
  sl_status_t valid      = SL_STATUS_FAIL;
  unsigned int next_size = map->base.size + 1;

  bool is_valid_pointer
    = (map != NULL) && (key != NULL) && is_valid_zcl_command(command);
  bool is_valid_size = (next_size < MAX_MAP_SIZE);

  if (is_valid_pointer && is_valid_size) {
    unsigned int index = map->hash_func(key);
    char *destination  = map->base.pair_list[index].key;

    strncpy(destination, key, MAX_KEY_SIZE);
    map->base.pair_list[index].command = command;
    map->base.size                     = next_size;
    valid                              = SL_STATUS_OK;
  } else {
    valid = SL_STATUS_FAIL;
  }

  return valid;
}

sl_status_t insert_pair_hash_map(hash_map_t *map, const zcl_key_value_t *pair)
{
  sl_status_t valid     = SL_STATUS_FAIL;
  bool is_valid_pointer = (map != NULL) && (pair != NULL);

  if (is_valid_pointer) {
    valid = insert_hash_map(map, pair->key, pair->command);
  } else {
    valid = SL_STATUS_NULL_POINTER;
  }

  return valid;
}

zcl_command_t retrieve_hash_map(const hash_map_t *map, const char *key)
{
  zcl_command_t command = {0};

  if ((map != NULL) && (key != NULL)) {
    unsigned int index = map->hash_func(key);
    command            = map->base.pair_list[index].command;
  }

  return command;
}

sl_status_t remove_hash_map(hash_map_t *map, const char *key)
{
  sl_status_t valid     = SL_STATUS_FAIL;
  bool is_valid_pointer = (map != NULL) && (key != NULL);

  if (is_valid_pointer) {
    const char null_char = '\0';

    unsigned int index = map->hash_func(key);

    map->base.pair_list[index].command = null_command;
    strncpy(map->base.pair_list[index].key, &null_char, MAX_KEY_SIZE);

    valid = SL_STATUS_OK;
  } else {
    valid = SL_STATUS_NULL_POINTER;
  }

  return valid;
}
