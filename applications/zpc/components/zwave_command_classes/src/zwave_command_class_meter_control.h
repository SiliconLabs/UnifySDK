/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zwave_command_class_meter_control Meter Command Class Control
 * @ingroup command_classes
 * @brief Meter Command Class control functions
 *
 * This module provide the control of the Meter Command Class.
 *
 * The attribute store model is as follows:
 *
 @startuml{attribute_store_meter_command_class.png} "Meter data model" width=10cm
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_METER_type
endlegend

package "Attribute Store" <<Database>> {
  object "NodeID" as node #f2ffe6
  node : Attribute Type = ATTRIBUTE_NODE_ID
  node : value = Desired: [], Reported: [03]

  object "Endpoint Attribute" as endpoint #e6fff7
  endpoint : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint : value = Desired: [] - Reported: [04]

  object "Version" as version #FEFEFE
  version : Attribute Type = ATTRIBUTE(VERSION)
  version : value = Desired: [] - Reported: [2]

  object "Meter Reset Supported" as meter_reset_supported #FFFFFF
  meter_reset_supported : Attribute Type = ATTRIBUTE(RESET_SUPPORTED)
  meter_reset_supported : value = Desired: [] - Reported: [1]

  object "Meter Supported Rate Type" as meter_supported_rate_types #FFFFFF
  meter_supported_rate_types : Attribute Type = ATTRIBUTE(SUPPORTED_RATE_TYPES)
  meter_supported_rate_types : value = Desired: [] - Reported: [3]

  object "Meter Supported Scales" as meter_supported_scales #FFFFFF
  meter_supported_scales : Attribute Type = ATTRIBUTE(SUPPORTED_SCALES)
  meter_supported_scales : value = Desired: [] - Reported: [5]

  package "For each Meter Type" <<Rectangle>> {

    object "Meter Type" as meter_type #FFFFFF
    meter_type : Attribute Type = ATTRIBUTE(TYPE)
    meter_type : value = Desired: [] - Reported: [0]

    package "For each Meter Scale" <<Rectangle>> {

      object "Scale" as meter_scale #FFFFFF
      meter_scale : Attribute Type = ATTRIBUTE(SCALE)
      meter_scale : value = Desired: [] - Reported: [1]

      package "For each Meter Rate Type" <<Rectangle>> {

      object "Meter Rate Type" as meter_rate_type #FFFFFF
      meter_rate_type : Attribute Type = ATTRIBUTE(RATE_TYPE)
      meter_rate_type : value = Desired: [] - Reported: [5]

      object "Value" as meter_value #FFFFFF
      meter_value : Attribute Type = ATTRIBUTE(VALUE)
      meter_value : value = Desired: [] - Reported: [1]

      object "Precision" as meter_precision #FFFFFF
      meter_precision : Attribute Type = ATTRIBUTE(PRECISION)
      meter_precision : value = Desired: [] - Reported: [1]
      }
    }
  }
}

node *-- endpoint
endpoint *-- version
endpoint *-- meter_reset_supported
endpoint *-- meter_supported_scales
endpoint *-- meter_supported_rate_types

version -[hidden]- meter_reset_supported
meter_reset_supported -[hidden]- meter_supported_scales
meter_supported_scales -[hidden]- meter_supported_rate_types

endpoint *-- meter_type
meter_type *-- meter_scale
meter_scale *-- meter_rate_type
meter_rate_type *-- meter_value
meter_rate_type *-- meter_precision
@enduml

 *
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_METER_CONTROL_H
#define ZWAVE_COMMAND_CLASS_METER_CONTROL_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function initialize the Meter Command Class control handler
 *
 * @return SL_STATUS_OK on success, any other error code for an error.
 */
sl_status_t zwave_command_class_meter_control_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_METER_CONTROL_H
/** @} end zwave_command_class_meter_control */
