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

#ifndef FAILING_NODE_MONITOR_H
#define FAILING_NODE_MONITOR_H
#include <zwave_node_id_definitions.h>
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @defgroup failing_node_monitor Failing Node Monitor 
 * @ingroup network_monitor 
 * @ingroup zpc_components
 * @brief The Failing node monitor
 *
 * @startuml
  [*] -> Network_Monitor
  state Failing_Node_Monitor {
    State Wait#magenta
        state FailingNodeList #lightblue {
            state Node1 #Green
    Node1: next ping, last ping
            state Node2 #Green
    Node2: next ping, last ping
            state Node3 #Green
    Node3: next ping, last ping
    
        }
    Network_Monitor -> Insert: Node offline
    Network_Monitor -> Init: Initialization
    Init-[#blue,bold]->FailingNodeList
    Insert-[#blue,bold]->FailingNodeList
    Network_Monitor -> Remove : Node online
    Insert: Insert node id into failing node list.
    Insert->Check_FailingList
    Check_FailingList-[#blue,bold]->FailingNodeList
    
    Check_FailingList: Check if node is in failing list.
    Check_FailingList: If it exists, remove it
    Check_FailingList-->Adjust
    Adjust:Adjust next ping timer of all nodes in failing node list according to current timer.
    Adjust-->Check_Node_Type
    state Check_Node_Type << fork >>
    Check_Node_Type --> AL
    Check_Node_Type --> FL
    AL:set initial timer value to 4 seconds
    FL: set initial timer value to 40 seconds
    
    state Send_NOP << join >>
    AL--> Send_NOP
    FL--> Send_NOP
    Insert: Adjust all timer for all existing nodes.
    Insert: Sort the failing node list
    
    state Send_NOP {
      Send_NOP_First-->Change_Intervals
      Change_Intervals-[#blue,bold]->FailingNodeList
      Change_Intervals: For AL: double the timer value
      Change_Intervals: For FL: quadraple the timer value
      Send_NOP_First: send NOP to first item in the list
      Change_Intervals-->And_wait
      And_wait-[#magenta,bold]->Wait
    }
    
    
    Wait -[#magenta,bold]-> Send_NOP: Send NOP again
    Wait: wait for timer value
    state Remove {
      Remove_Failing_List-[#blue,bold]->FailingNodeList
      Remove_Failing_List: remove node from failing node list
      Remvoe_Attribute_Store: Remove from Attribute store
      Stop_Timer: Stop timer if Failing Node list is empty
      Stop_Timer-[#blue,bold]->FailingNodeList
      Stop_Timer-[#magenta,bold]->Wait
      Remove_Failing_List-->Remvoe_Attribute_Store
      Remvoe_Attribute_Store-->Stop_Timer
    }
  }
  Init-->Attribute_Store: Read failing nodes if any \nand their last interval
  Remvoe_Attribute_Store->Attribute_Store: Remove value
  AL-->Attribute_Store: Store value
  FL-->Attribute_Store: Store value
  Check_Node_Type-->ZWave_utils: zwave_get_operating_mode()
  ZWave_utils-->Check_Node_Type
  Send_NOP_First --> Zwave_Controller
  Zwave_Controller: Call zwave_tx to send NOP
  Zwave_Controller-->[*]
  @enduml
 */
 
/**
 * @brief Stop monitoring the failing node by
 * removing the node id from Failing list monitor list 
 */
void stop_monitoring_failing_node(zwave_node_id_t node_id);
/**
 * @brief Start monitoring the failing node by 
 * inserting the node id to Failing list monitor list 
 *
 * On insertion, the Always listeing nodes are first pinged after an interval of 
 * 4 seconds then the intrval is doubled on failure of nop sent every time.
 *
 * On insertion, the Frequently listeing nodes are first pinged after an interval of 
 * 40 seconds then the intrval is doubled on failure of nop sent every time.
 *
 * Max ping interval for nodes is 24 hours.
 *
 * This also store the interval in datastore everytime the interval changes
 */
void start_monitoring_failing_node(zwave_node_id_t node_id);

/**
 * @brief Print the the failing node monitor list.
 *
 */
void print_failing_node_monitor_list();

/**
 * @brief Load the failing node monitor list.
 *
 * This function will load any existing failing nodes and their last 
 * interval from the datastore on call of this function. Which is called in ZPC
 * booting 
 */
void failing_node_monitor_list_load();
/**
 * @brief Clear the failing node monitor list and stop the timer.
 *
 * This MUST be called on set default
 *
 */
void failing_node_monitor_list_clear();
#ifdef __cplusplus
}
#endif
/** @} end failing_network_monitor */

#endif  //FAILING_NODE_MONITOR_H
