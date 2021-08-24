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

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "callback_map.h"

callback_map_t create_callback_map()
{
  callback_map_t new_map;
  new_map.size = 0;

  int i;
  for (i = 0; i < MAX_MAP_SIZE; i++) {
    new_map.pair_list[i].command.cluster_id = ZCL_INVALID_CLUSTER_ID;
    new_map.pair_list[i].command.command_id = ZCL_INVALID_COMMAND_ID;
  }

  return new_map;
}

sl_status_t insert_pair_callback_map(callback_map_t *map,
                                     const zcl_key_value_t *pair)
{
  sl_status_t valid = SL_STATUS_FAIL;

  bool is_valid_pointer = (map != NULL) && (pair != NULL);

  if (is_valid_pointer) {
    valid = insert_callback_map(map, pair->key, pair->command);
  } else {
    valid = SL_STATUS_NULL_POINTER;
  }

  return valid;
}

sl_status_t insert_callback_map(callback_map_t *map,
                                const char *key,
                                const zcl_command_t command)
{
  sl_status_t valid      = SL_STATUS_FAIL;
  unsigned int next_size = map->size + 1;

  bool is_valid_pointer
    = (map != NULL) && (key != NULL) && is_valid_zcl_command(command);
  bool is_valid_size = next_size < MAX_MAP_SIZE;

  int index = 0;
  (void)retrieve_callback_map(map, key, &index);

  if (is_valid_pointer && is_valid_size) {
    if (index == -1) {
      map->size = next_size;
      index     = next_size - 1;
      strncpy(map->pair_list[index].key, key, MAX_KEY_SIZE);
    }

    map->pair_list[index].command = command;

    valid = SL_STATUS_OK;
  } else {
    valid = SL_STATUS_FAIL;
  }

  return valid;
}

zcl_command_t
  retrieve_callback_map(const callback_map_t *map, const char *key, int *index)
{
  *index                = -1;
  zcl_command_t command = {0};

  bool is_valid_pointer = (map != NULL) && (key != NULL);

  if (is_valid_pointer) {
    unsigned int i;
    int retrieved = -1;

    for (i = 0; (i < map->size) && (retrieved != 0); i++) {
      const char *map_key = map->pair_list[i].key;
      retrieved           = strncmp(map_key, key, MAX_KEY_SIZE);

      if (retrieved == 0) {
        command = map->pair_list[i].command;
        *index  = i;
      }
    }
  }

  return command;
}

sl_status_t remove_callback_map(callback_map_t *map, const char *key)
{
  sl_status_t valid     = SL_STATUS_FAIL;
  bool is_valid_pointer = (map != NULL) && (key != NULL);

  if (is_valid_pointer) {
    int index = 0;
    (void)retrieve_callback_map(map, key, &index);
    if (index != -1) {
      int i;
      for (i = index; i < map->size - 2; i++) {
        strncpy(map->pair_list[i].key, map->pair_list[i + 1].key, MAX_KEY_SIZE);
        map->pair_list[i].command = map->pair_list[i + 1].command;
      }

      const char null_char = '\0';

      strncpy(map->pair_list[map->size - 1].key, &null_char, MAX_KEY_SIZE);
      map->pair_list[map->size - 1].command = null_command;
      map->size                             = map->size - 1;
    }

    valid = SL_STATUS_OK;
  } else {
    valid = SL_STATUS_NULL_POINTER;
  }

  return valid;
}
