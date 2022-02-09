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
 * @defgroup network_monitor_span_persistence Network Monitor S2 SPAN persistence
 * @ingroup network_monitor
 * @brief This sub-module provides an API to save and restore SPAN data.
 *
 * The SPAN data is saved for each individual NodeID.
 *
 * It is saved only at exit, and loaded again at initialization.
 *
@startuml{attribute_store_span_persistence.png} "SPAN persistence in the attribute store" width=10cm
title SPAN persistence in the attribute store
allow_mixing
skinparam objectBorderColor black

package "Attribute Store" <<Database>> {
  object "HomeID" as home_id #fffae6
  home_id : ID = ATTRIBUTE_HOME_ID
  home_id : value = Desired: [], Reported: [8C CE FB E6]

  object "ZPC NodeID" as zpc_node #f2ffe6
  zpc_node : Attribute Type = ATTRIBUTE_NODE_ID
  zpc_node : value = Desired: [], Reported: [01]

  package "Other Nodes"{
    object "NodeID" as node #f2ffe6
    node : Attribute Type = ATTRIBUTE_NODE_ID
    node : value = Desired: [], Reported: [03]

    object "SPAN Entry" as span_entry #FEFEFE
    span_entry : Attribute Type = ATTRIBUTE_S2_SPAN_ENTRY
    span_entry : value = Desired: [] - Reported: [00,3c,8c,fa,31,85,33,eb,db,70,5f,62,b4,65,95,53,91,fb,82,87,49,88,15,d5,ef,7c,dc,4e,36,ee,28,23,4b,d4,9f,01]
  }
}
home_id *-- zpc_node
home_id *-- node
node *-- span_entry

@enduml
 * The MPAN data is saved under the ZPC NodeID, and the group membership of each
 * node is saved under each individual node.
 *
 * It is saved only at exit, and loaded again at initialization.
 *
@startuml{attribute_store_mpan_persistence.png} "MPAN persistence in the attribute store" width=10cm
title MPAN persistence in the attribute store
allow_mixing
skinparam objectBorderColor black

package "Attribute Store" <<Database>> {

  object "HomeID" as home_id #fffae6
  home_id : ID = ATTRIBUTE_HOME_ID
  home_id : value = Desired: [], Reported: [8C CE FB E6]

  object "ZPC NodeID" as zpc_node #f2ffe6
  zpc_node : Attribute Type = ATTRIBUTE_NODE_ID
  zpc_node : value = Desired: [], Reported: [01]

  object "ZPC MPAN table" as mpan_table #FEFEFE
  mpan_table : Attribute Type = ATTRIBUTE_S2_MPAN_TABLE
  mpan_table : value = Desired: [], Reported: []

  object "ZPC MPAN entry 1" as mpan_entry_1 #FEFEFE
  mpan_entry_1 : Attribute Type = ATTRIBUTE_S2_MPAN_ENTRY
  mpan_entry_1 : value = Desired: [], Reported: [1a,f0,78,62,83,14,bd,42,4c,c5,41,24,f4,02,f2,d4,00,00,01,01]

  object "ZPC MPAN entry 2" as mpan_entry_2 #FEFEFE
  mpan_entry_2 : Attribute Type = ATTRIBUTE_S2_MPAN_ENTRY
  mpan_entry_2 : value = Desired: [], Reported: [5d,8e,47,ce,df,43,47,1f,f4,72,5d,84,43,6d,44,f3,00,00,02,01]

  package "Other Nodes"{
    object "NodeID" as node #f2ffe6
    node : Attribute Type = ATTRIBUTE_NODE_ID
    node : value = Desired: [], Reported: [03]

    object "Multicast Group list" as multicast_group_list #FEFEFE
    multicast_group_list : Attribute Type = ATTRIBUTE_MULTICAST_GROUP_LIST
    multicast_group_list : value = Desired: [], Reported: []

    object "Multicast Group" as multicast_group_1 #FEFEFE
    multicast_group_1 : Attribute Type = ATTRIBUTE_MULTICAST_GROUP
    multicast_group_1 : value = Desired: [], Reported: [01]

    object "Multicast Group" as multicast_group_2 #FEFEFE
    multicast_group_2 : Attribute Type = ATTRIBUTE_MULTICAST_GROUP
    multicast_group_2 : value = Desired: [], Reported: [02]
  }
}

home_id *-- zpc_node

zpc_node *-- mpan_table
mpan_table *-- mpan_entry_1
mpan_table *-- mpan_entry_2

home_id *-- node
node *-- multicast_group_list
multicast_group_list *-- multicast_group_1
multicast_group_list *-- multicast_group_2

@enduml
 *
 *
 * @{
 */

#ifndef NETWORK_MONITOR_SPAN_PERSISTENCE_H
#define NETWORK_MONITOR_SPAN_PERSISTENCE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Saves all the SPAN data from Z-Wave S2 in the Attribute Store
 */
void network_monitor_store_span_table_data();

/**
 * @brief Restores all the SPAN data to Z-Wave S2 from the Attribute Store
 */
void network_monitor_restore_span_table_data();

/**
 * @brief Saves all the MPAN data owned by the ZPC from Z-Wave S2 in the Attribute Store
 */
void network_monitor_store_mpan_table_data();

/**
 * @brief Restores all the MPAN data owned by the ZPC to Z-Wave S2 from the Attribute Store
 */
void network_monitor_restore_mpan_table_data();

#ifdef __cplusplus
}
#endif

#endif  //NETWORK_MONITOR_SPAN_PERSISTENCE_H
/** @} end network_monitor_span_persistence */
