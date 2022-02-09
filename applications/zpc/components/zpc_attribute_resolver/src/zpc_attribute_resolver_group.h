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
 * @defgroup attribute_resolver_group_send ZPC Attribute Resolver Group Send
 * @ingroup zpc_attribute_resolver_internals
 * @brief Transmit messages to multiple destinations at once.
 *
 * @startuml zwave_multicast
participant SomeTask as t1
participant AttributeStore as as
participant AttributeResolver as ar
participant AttributeResolverGroup as ag
participant "Z-Wave CommandClasses" as cc
participant "Z-Wave API" as zw

... Init ...
ag -> ar : attribute_resolver_register_set_rule_listener(&on_new_set_rule_registered)

... Command Class Init ...
note over cc
    Each CC will register rules on Desired Z-Wave
    attributes, in this example only one rule
    registration is shown.
end note
loop For each registered rule from the Z-Wave Command Class handler
    cc -> ar : attribute_resolver_register_rule(...)
    ar -> ag : on_new_set_rule_registered(...)
    ag -> as : attribute_store_register_callback_by_type_and_state(on_settable_attribute_update, ...)
    note over ag
        Attribute Resolver Group registers a callback for
        when the attribute value is set.
    end note
end

... Desired attribute set ...
note over t1
    Task sets a number of attributes. This could
    e.g. be a command for a ZCL group. In this
    example the task will set the same desired
    value for the same attribute type on 2 nodes
    (could e.g. be On to a
    binary switch).
end note

t1 -> as : attribute_store_set_node_attribute_value(1, DESIRED, ON, 1)
as -> ag : on_settable_attribute_update(...)
note over ag
    Attribute Group Resolver stores the node
    in the <i>multicast_candidates</i> list
end note
t1 -> as : attribute_store_set_node_attribute_value(2, DESIRED, ON, 1)
as -> ag : on_settable_attribute_update(...)
note over ag
    Attribute Group Resolver stores the node
    in the <i>multicast_candidates</i> list
end note

note over ar
    Attribute Resolver will on its next scan
    detect the Desired attribute changes and
    call <i>attribute_resolver_send()</i>, which
    calls <i>zpc_attribute_resolver_send_group()</i>
end note
ar -> ag : zpc_attribute_resolver_send_group(1)
note over ag
    Attribute Store Group will loop through the
    <i>multicast_candidates</i> list and find the
    desired attributes, that can be multicasted.
    Each attribute, that can be multicasted over
    Z-Wave will be sent here, and the attributes
    will be marked added to the attribute store
    resolution list.
end note
ag -> zw : zwave_tx_assign_group(...)
ag -> zw : zwave_tx_scheme_get_node_tx_options(...)
alt supervision
    ag -> zw : zwave_command_class_supervision_send_data(...)
else no supervision
    ag -> zw : zwave_tx_send_data(...)
end
ag -> ar : attribute_resolver_associate_node_with_tx_sessions_id()

ar -> ag : zpc_attribute_resolver_send_group(2)
note over ag
    When the attribute resolver calls the 2nd send
    the attribute is already sent as a multicast,
    and the <i>zpc_attribute_resolver_send_group()</i>
    function will return immediately.
end note
@enduml
 *
 * @{
 */

#ifndef ZPC_ATTRIBUTE_RESOLVER_GROUP_H
#define ZPC_ATTRIBUTE_RESOLVER_GROUP_H

#include "attribute_store.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the resolver group sub-component
 * @returns SL_STATUS_OK if successful. Any other error code otherwise.
 */
sl_status_t zpc_attribute_resolver_group_init();

/**
 * @brief Teardown the resolver group sub-component.
 * @returns 0
 */
int zpc_attribute_resolver_group_teardown();

/**
 * @brief Verifies if the node can be part of a multicast transmission,
 *        and if yes, trigger it.
 *
 * @param node  The attribute store node to resolve.
 * @returns SL_STATUS_NOT_SUPPORTED if the node won't be resolved with multicast
 *          transmissions. SL_STATUS_OK if the node will be resolved with
 *          multicast transmission.
 */
sl_status_t zpc_attribute_resolver_send_group(attribute_store_node_t node);

/**
 * @brief Helper function that logs the state of the resolver group component
 *
 * Print out the list of candidates and the list of pools
 */
void zpc_attribute_resolver_group_state_log();

#ifdef __cplusplus
}
#endif

#endif  //ZPC_ATTRIBUTE_RESOLVER_GROUP_H
/** @} end zpc_attribute_resolver_group */
