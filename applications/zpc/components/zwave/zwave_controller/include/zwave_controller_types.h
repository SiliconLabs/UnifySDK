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
 * @defgroup zwave_controller_types Z-Wave Controller Types
 * @ingroup zwave_controller_component
 * @brief Z-Wave Controller shared type definitions
 **
 * @{
 */


#ifndef ZWAVE_CONTROLLER_TYPES_H
#define ZWAVE_CONTROLLER_TYPES_H

#include <stdbool.h>
#include <stdint.h>
// Z-Wave definitions
#include "zwave_node_id_definitions.h"
#include "zwave_rf_region.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief TODO: move this
 *
 * This is a temporary resting place for special z-wave types.
 * These should be documents and some types may need a specific api.
 */

#define ZWAVE_DSK_LENGTH                                   16
#define ZWAVE_NODE_MASK_LENGTH                             29
#define ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH 150

typedef uint8_t zwave_dsk_t[ZWAVE_DSK_LENGTH];
typedef zwave_nodemask_t zwave_node_list_t;
typedef uint8_t zwave_multicast_group_id_t;
typedef zwave_rf_region_t zwave_controller_region_t;
typedef uint8_t zwave_endpoint_id_t;
typedef uint16_t zwave_command_class_t;
typedef uint8_t zwave_command_t;

/**
 * @brief Structure holding a Z-Wave node info frame.
 *
 */
typedef struct {
  /// This represents the first byte of the NIF, also known as "capability"
  /// It is composed of several subfields that can be read using the
  /// ZWAVE_NODE_INFO_LISTENING_PROTOCOL_* masks. For a complete field
  /// description, refer to @ref application_cc_spec, section Node Information Frame.
  uint8_t listening_protocol;

  /// This represents the first byte of the NIF, also known as "security"
  /// It is composed of several subfields that can be read using the
  /// ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_* masks. For a complete field
  /// description, refer to @ref application_cc_spec, section Node Information Frame.
  uint8_t optional_protocol;

  /// This field indicates the Basic Device Class of the actual node.
  /// The Basic Device Classes are listed in @ref device_class_spec.
  /// This byte is omitted by End Nodes and will be set to 0x00 in this case.
  uint8_t basic_device_class;

  /// This field indicates the Generic Device Class of the actual node.
  /// The Generic Device Classes are listed in @ref device_class_spec for
  /// Z-Wave and @ref device_type_spec_v2, @ref device_type_spec for Z-Wave Plus
  uint8_t generic_device_class;

  /// This field indicates the Specific Device Class of the actual node.
  /// The Specific Device Classes are listed in @ref device_class_spec for
  /// Z-Wave and @ref device_type_spec, @ref device_type_spec_v2 for Z-Wave Plus
  uint8_t specific_device_class;

  /// Length of the command class list
  uint8_t command_class_list_length;

  /// List of command classes supported by the device.
  ///
  zwave_command_class_t
    command_class_list[ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH];
} zwave_node_info_t;
#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_CONTROLLER_TYPES_H
/** @} end zwave_controller_types */
