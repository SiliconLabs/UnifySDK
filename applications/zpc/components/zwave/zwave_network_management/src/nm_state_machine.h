#ifndef NM_STATE_MACHINE_H
#define NM_STATE_MACHINE_H

//#include <nm_stub.h>
#include "zwave_network_management_types.h"

#include "zwave_network_management.h"
#include "zwave_network_management_process.h"
#include "zwapi_protocol_controller.h"
#include "sys/etimer.h"
#include "sl_status.h"

#define NODE_INFO_MAX_SIZE 64

/**
 * Control structure for the Network Management State machine (NMS).
 * \ingroup NW_CMD_handler
 */
typedef struct network_mgmt_state {
  // Cached information about our current network
  zwave_home_id_t cached_home_id;
  zwave_node_id_t cached_local_node_id;
  zwave_nodemask_t cached_node_list;
  // Bitmask cached from Get Controller Capabilities Command
  uint8_t controller_capabilities_bitmask;

  /** The node id on which Network Management is currently working on:
   * - The node being added in add node.
   * - The node being removed in remove node.
   * - The node id of the controller including on in learn mode
   * - The node being probed to reply to a NodeInfoCachedGet.
   * - etc. */
  zwave_node_id_t node_id_being_handled;
  /*This buffer is global, because the node info is only present in the */
  zwave_network_management_state_t state;
  /** Sub-state flags on the current state. */
  int flags;

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
  /// Flag to keep track if S2 boostrapping has been initiated
  bool s2_bootstrapping_started;

  zwave_kex_fail_type_t kex_fail_type;
  zwave_keyset_t granted_keys;

  zwave_protocol_t inclusion_protocol;
  sl_status_t status;

  zwave_node_info_t node_info;
  /// True if SmartStart should be enabled after returning to idle.
  bool smart_start_add_mode_enabled;
  /// Configures if we enter SmartStart learn mode when we are alone
  /// in our network and are idle.
  /// SmartStart Add mode takes precedence over SmartStart learn mode
  bool smart_start_learn_mode_enabled;

  struct etimer timer;
  // Requested Learn mode
  zwave_learn_mode_t learn_mode_intent;
  /// Indicates if the node being included is a controller.
  uint8_t is_controller;
  /// Proxy inclusion step for identifying if the
  /// request inclusion step is INCLUSION or INCLUSION_REPLACE
  uint8_t proxy_inclusion_step;

  /// Whether S0 inclusion/bootstrapping is enabled.
  bool s0_inclusion_enabled;
} network_mgmt_state_t;

extern network_mgmt_state_t nms;

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
 * LEARN_MODE_SET command, i.e., LEARN_MODE_INTERVIEW_COMPLETED is
 * requested.
 * \ingroup NW_CMD_handler
 * \see Learn Mode Set Command (Network Management Basic Node Command
 * Class, version 2 and up)
 */
#define NMS_FLAG_LEARNMODE_NEW 4
/**
 * Sub-state flag for the Network Management State machine.
 *
 * The LEARN_MODE_NWI was set on the LEARN_MODE_SET command.
 *
 * \see Learn Mode Set Command (Network Management Basic Node Command
 * Class).
 * \ingroup NW_CMD_handler
 */
#define NMS_FLAG_LEARNMODE_NWI 8
/**
 * Sub-state flag for the Network Management State machine.
 *
 * The LEARN_MODE_NWE was set on the LEARN_MODE_SET command.
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
