#ifndef NM_STATE_MACHINE_H
#define NM_STATE_MACHINE_H

//#include <nm_stub.h>
#include "zwave_network_management_state.h"
#include "zwave_network_management.h"
#include "zwapi_protocol_controller.h"
#include "sys/etimer.h"
#include "sl_status.h"

#define NODE_INFO_MAX_SIZE 64

typedef enum {

  NM_EV_ADD_LEARN_READY,
  NM_EV_ADD_NODE_FOUND,
  NM_EV_ADD_CONTROLLER,
  NM_EV_ADD_PROTOCOL_DONE,
  NM_EV_ADD_END_NODE,
  NM_EV_ADD_FAILED,
  NM_EV_ADD_NOT_PRIMARY,
  NM_EV_ADD_NODE_STATUS_DONE,
  NM_EV_NODE_ADD,
  NM_EV_TIMEOUT,
  /** See on_inclusion_complete */
  NM_EV_SECURITY_DONE,
  NM_EV_S0_STARTED,
  /** NM_EV_ADD_SECURITY_REQ_KEYS event is triggered (from on_keys_request)
   * when kex get (key request) is received. So that the user can be notified
   * to grant keys prompt
   */
  NM_EV_ADD_SECURITY_REQ_KEYS,
  /* NM_EV_ADD_SECURITY_KEY_CHALLENGE is called (fro on_dsk_challenge)when
   * public key report is received from other side. NM FSM calls
   * zwave_controller_on_dsk_report() to send the request user
   */
  NM_EV_ADD_SECURITY_KEY_CHALLENGE,
  NM_EV_NODE_ADD_S2,

  /** NM_EV_ADD_SECURITY_KEYS_SET event is triggered when the user grants
   * keys with zwave_network_management_keys_set()
   * NM FSM calls zwave_s2_key_grant() to notify S2 about the granted keys
   */
  NM_EV_ADD_SECURITY_KEYS_SET,
  /** NM_EV_ADD_SECURITY_DSK_SET is triggered (from zwave_network_management_dsk_set)
   * to Indicate to the S2 bootstrapping controller whether the DSK is accepted
   * and report the user input when needed
   */
  NM_EV_ADD_SECURITY_DSK_SET,
  NM_EV_REPLACE_FAILED_START,
  NM_EV_REPLACE_FAILED_STOP,
  NM_EV_REPLACE_FAILED_DONE,
  NM_EV_REPLACE_FAILED_FAIL,
  NM_EV_REPLACE_FAILED_START_S2,
  NM_EV_LEARN_SET,
  NM_EV_LEARN_STOP,
  NM_EV_REQUEST_NODE_LIST,
  NM_EV_REQUEST_FAILED_NODE_LIST,
  NM_EV_PROXY_COMPLETE,
  NM_EV_START_PROXY_INCLUSION,
  NM_EV_START_PROXY_REPLACE,
  NM_EV_NODE_INFO,
  NM_EV_FRAME_RECEIVED,
  NM_EV_NODE_ADD_SMART_START,
  NM_EV_REMOVE_FAILED,
  NM_EV_REMOVE_FAILED_OK,
  NM_EV_REMOVE_FAILED_FAIL,
  NM_EV_SMART_START_ENABLE,
  NM_EV_SET_DEFAULT,
  NM_EV_SET_DEFAULT_COMPLETE,
  NM_EV_NODE_REMOVE,
  NM_EV_NODE_REMOVE_FAILED,
  NM_EV_NODE_REMOVE_COMPLETE,
  NM_EV_ABORT,
  NM_EV_NOP_FAIL,
  NM_EV_NOP_SUCCESS,
  NM_EV_REQUEST_NODE_NEIGHBOR_REQUEST,
  NM_EV_MAX,  //This MUST always to be last entry in this enum
} nm_event_t;

/**
 * Control structure for the Network Management State machine (NMS).
 * \ingroup NW_CMD_handler
 */
typedef struct network_mgmt_state {
  //cached Home and Node id for the current network
  zwave_home_id_t cached_home_id;
  zwave_node_id_t cached_node_id;
  zwave_nodemask_t cached_node_list;

  uint8_t tx_options;
  /** The node nms is currently working on:
   * - The node being added in add node.
   * - The node including the zipgateway in learn mode.
   * - The node being probed to reply to a NodeInfoCachedGet.
   * - etc. */
  zwave_node_id_t tmp_node;
  /*This buffer is global, because the node info is only present in the */
  zwave_network_management_state_t state;
  /** Sub-state flags on the current state. */
  int flags;
  uint8_t buf_len;
  uint8_t count; // counter used to send multiple explore_request_inclusion/exclusion frame

  /* Used for testing when middleware has finished probing a newly
   * included SmartStart node */

  /// The DSK we expect, this is used in SmartStart inclusion
  zwave_dsk_t expected_dsk;
  /// The DSK reported by the node, this is pushed in libs2
  zwave_dsk_t reported_dsk;
  /// Verified DSK input to be used in S2 inclusion  for Authenticated Security Classes
  zwave_dsk_t verified_dsk_input;
  /// Number of bytes in the reported dsk which has been blanked
  uint8_t reported_dsk_blanked;
  /// Requested keys keys requested by a node
  zwave_keyset_t requested_keys;
  /// If the node requested csa
  bool requested_csa;
  /// If the S2 bootstrapping is accepted
  bool accepted_s2_bootstrapping;
  /// If csa request is accepted
  bool accepted_csa;

  zwave_kex_fail_type_t kex_fail_type;
  zwave_keyset_t granted_keys;

  zwave_protocol_t inclusion_protocol;
  zwave_node_id_t node_id;
  sl_status_t status;

  zwave_node_info_t node_info;
  ///True if SmartStart should be enabled after returning to idle.
  bool smart_start_enabled;
  struct etimer timer;
  // Requested Learn mode
  uint8_t learn_mode_intent;
  uint8_t is_controller;
  /// Proxy inclusion step for identifying if the
  /// request inclusion step is INCLUSION or INCLUSION_REPLACE
  uint8_t proxy_inclusion_step;
} network_mgmt_state_t;

extern network_mgmt_state_t nms;
//struct ctimer cancel_timer;

typedef struct smartstart_event_data {
  zwave_dsk_t dsk;
  zwave_keyset_t granted_keys;
  zwave_protocol_t preferred_inclusion;
} smartstart_event_data_t;

void nm_fsm_post_event(nm_event_t ev, void *event_data);
/*****************************************************************************/
/**
 * Sub-state flag for the Network Management State machine.
 *
 * NMS is processing an S2 addition (Smart-Start or simple S2).
 * \ingroup NW_CMD_handler
 */
#define NMS_FLAG_S2_ADD 1
/**
 * Sub-state flag for the Network Management State machine.
 *
 * NMS is processing a proxy inclusion or proxy replace.
 * \ingroup NW_CMD_handler
 */
#define NMS_FLAG_PROXY_INCLUSION 2
/**
 * Sub-state flag for the Network Management State machine.
 *
 * The ZW_LEARN_MODE_RETURN_INTERVIEW_STATUS was set on the
 * #LEARN_MODE_SET command, i.e., #LEARN_MODE_INTERVIEW_COMPLETED is
 * requested.
 * \ingroup NW_CMD_handler
 * \see Learn Mode Set Command (Network Management Basic Node Command
 * Class, version 2 and up)
 */
#define NMS_FLAG_LEARNMODE_NEW 4
/**
 * Sub-state flag for the Network Management State machine.
 *
 * The ZW_SET_LEARN_MODE_NWI was set on the #LEARN_MODE_SET command.
 *
 * \see Learn Mode Set Command (Network Management Basic Node Command
 * Class).
 * \ingroup NW_CMD_handler
 */
#define NMS_FLAG_LEARNMODE_NWI 8
/**
 * Sub-state flag for the Network Management State machine.
 *
 * The ZW_SET_LEARN_MODE_NWE was set on the #LEARN_MODE_SET command.
 *
 * \see Learn Mode Set Command (Network Management Basic Node Command
 * Class).
 * \ingroup NW_CMD_handler
 */
#define NMS_FLAG_LEARNMODE_NWE 0x10
/**
 * Sub-state flag for the Network Management State machine.
 *
 * After LEARN_MODE_DONE, NMS has determined that it is neither being
 * included nor excluded, so it must be processing a controller
 * replication (or controller shift).
 *
 * \ingroup NW_CMD_handler
 */
#define NMS_FLAG_CONTROLLER_REPLICATION 0x20
/**
 * Sub-state flag for the Network Management State machine.
 *
 * NMS is processing a Smart-Start addition.
 *
 * \ingroup NW_CMD_handler
 */
#define NMS_FLAG_SMART_START_INCLUSION 0x40
/**
 * Sub-state flag for the Network Management State machine.
 *
 * Inclusion and S2 inclusion have succeeded, so NMS should include
 * the DSK when sending NODE_ADD_STATUS.
 *
 * \ingroup NW_CMD_handler
 */
#define NMS_FLAG_REPORT_DSK 0x80
/**
 * Sub-state flag for the Network Management State machine.
 *
 * \ingroup NW_CMD_handler
 */
#define NMS_FLAG_CSA_INCLUSION 0x100
/**
 * @brief Initialize the Z-Wave network management state machine 
 *
 * Sets \ref zwave_s2_network_callbacks_t
 * Calls \ref zwave_s2_set_network_callbacks and \ref zwave_s0_set_network_callbacks
 * Resets \ref network_mgmt_state_t nms
 * Sets the Network Management state to \ref NM_IDLE
 * Print the HomeID, NodeID and DSK to console and Print node list
 * Stops all ongoing Network management operations
 *
 */
void nm_state_machine_init();
/**
 * @brief Teardown the Z-Wave network management state machine 
 * Resets \ref network_mgmt_state_t nms
 * Sets the Network Management state to \ref NM_IDLE
 * Stops all ongoing Network management operations
 *
 */
 
void nm_state_machine_teardown();

#endif
