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
 * @defgroup zwave_command_class_configuration Configuration Command Class
 * @ingroup command_classes
 * @brief Configuration Command Class handlers and control functions
 *
 * This module provide the control of the Configuration Command Class.
 *
 * For v1-v2 nodes, no automatic discovery of the configuration parameter is
 * performed, and we rely on another component to indicate which configuration
 * parameters are supported, or to be "probed".
 *
 * Supporting nodes from v1 to v4 all share the same model, all attributes
 * are created and default values are assumed.
 * The attribute store model is as follows:
 *
@startuml{attribute_store_configuration_command_class.png} "Configuration data model" width=10cm
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_type
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

  object "Next Supported Parameter ID" as next_parameter_id #FFFFFF
  next_parameter_id : Attribute Type = ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID)
  next_parameter_id : value = Desired: [] - Reported: [5]

  object "Bulk Support" as bulk_support #FFFFFF
  bulk_support : Attribute Type = ATTRIBUTE(BULK_SUPPORT)
  bulk_support : value = Desired: [] - Reported: [5]

  package "For each Parameter" <<Rectangle>> {

    object "Parameter ID" as parameter_id #FFFFFF
    parameter_id : Attribute Type = ATTRIBUTE(PARAMETER_ID)
    parameter_id : value = Desired: [] - Reported: [1]

    object "Size" as parameter_size #FFFFFF
    parameter_size : Attribute Type = ATTRIBUTE(PARAMETER_SIZE)
    parameter_size : value = Desired: [] - Reported: [2]

    object "Format" as parameter_format #FFFFFF
    parameter_format : Attribute Type = ATTRIBUTE(PARAMETER_FORMAT)
    parameter_format : value = Desired: [] - Reported: [2]

    object "Name" as parameter_name #FFFFFF
    parameter_name : Attribute Type = ATTRIBUTE(PARAMETER_NAME)
    parameter_name : value = Desired: [] - Reported: ["Parameter 1"]

    object "Info" as parameter_info #FFFFFF
    parameter_info : Attribute Type = ATTRIBUTE(PARAMETER_INFO)
    parameter_info : value = Desired: [] - Reported: ["Toggles the relay mode"]

    object "Value" as parameter_value #FFFFFF
    parameter_value : Attribute Type = ATTRIBUTE(PARAMETER_VALUE)
    parameter_value : value = Desired: [12] - Reported: [23]

    object "Min Value" as parameter_min_value #FFFFFF
    parameter_min_value : Attribute Type = ATTRIBUTE(PARAMETER_MIN_VALUE)
    parameter_min_value : value = Desired: [] - Reported: [0]

    object "Max Value" as parameter_max_value #FFFFFF
    parameter_max_value : Attribute Type = ATTRIBUTE(PARAMETER_MAX_VALUE)
    parameter_max_value : value = Desired: [] - Reported: [100]

    object "Default Value" as parameter_default_value #FFFFFF
    parameter_default_value : Attribute Type = ATTRIBUTE(PARAMETER_DEFAULT_VALUE)
    parameter_default_value : value = Desired: [] - Reported: [1]

    object "Advanced" as parameter_advanced #FFFFFF
    parameter_advanced : Attribute Type = ATTRIBUTE(PARAMETER_ADVANCED)
    parameter_advanced : value = Desired: [] - Reported: [1]

    object "Read-Only" as parameter_read_only #FFFFFF
    parameter_read_only : Attribute Type = ATTRIBUTE(PARAMETER_READ_ONLY)
    parameter_read_only : value = Desired: [] - Reported: [0]

    object "Altering capabilities" as parameter_altering_capabilities #FFFFFF
    parameter_altering_capabilities : Attribute Type = ATTRIBUTE(PARAMETER_ALTERING_CAPABILITIES)
    parameter_altering_capabilities : value = Desired: [] - Reported: [0]
  }

}

node *-- endpoint
endpoint *-- version
endpoint *-- next_parameter_id
endpoint *-- bulk_support
endpoint *-- parameter_id

next_parameter_id -[hidden]- parameter_id

parameter_id *-- parameter_size
parameter_id *-- parameter_format
parameter_id *-- parameter_name

parameter_size -[hidden]- parameter_info
parameter_format -[hidden]- parameter_value
parameter_name -[hidden]- parameter_min_value

parameter_id *-- parameter_info
parameter_id *-- parameter_value
parameter_id *-- parameter_min_value

parameter_info -[hidden]- parameter_max_value
parameter_value -[hidden]- parameter_default_value
parameter_min_value -[hidden]- parameter_advanced

parameter_id *-- parameter_max_value
parameter_id *-- parameter_default_value
parameter_id *-- parameter_advanced

parameter_max_value -[hidden]- parameter_read_only
parameter_default_value -[hidden]- parameter_altering_capabilities

parameter_id *-- parameter_read_only
parameter_id *-- parameter_altering_capabilities

@enduml

 *
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_CONFIGURATION_H
#define ZWAVE_COMMAND_CLASS_CONFIGURATION_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function initialize the Configuration Command Class handler
 *
 * @return SL_STATUS_OK on success, any other error code for an error.
 */
//sl_status_t zwave_command_class_configuration_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_CONFIGURATION_H
/** @} end zwave_command_class_configuration */
