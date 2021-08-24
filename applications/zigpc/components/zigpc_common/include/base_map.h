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
 * Base Map
 * 
 * A simple map implementation. Stores a callback function associated with a
 * given character string. Uses a list of map pairs and a size to store all
 * relevant information. MAX_MAP_SIZE and MAX_KEY_SIZE are used to limit the 
 * memory operations to better comply with MISRA standards.
 *
 ******************************************************************************/

#ifndef ZIGPC_BASE_MAP_H
#define ZIGPC_BASE_MAP_H

#define MAX_MAP_SIZE 1024
#define MAX_KEY_SIZE 64

#include "zigpc_common_zigbee.h"

/**
 * zcl_key_value_t
**/
typedef struct zcl_key_value {
  char key[MAX_KEY_SIZE];
  zcl_command_t command;
} zcl_key_value_t;

/** \struct simple_map
 * A structure for holding a list of key-value pairs and keeping track of the 
 * number of pairs contained.
**/
struct callback_map {
  zcl_key_value_t pair_list
    [MAX_MAP_SIZE];  ///< An array of pairs. Bounded by the MAX size for MISRA considerations
  unsigned int
    size;  ///< The number of populated pairs in the map. The last index of the list is size-1
};
typedef struct callback_map callback_map_t;

#endif  //ZIGPC_BASE_MAP_H
