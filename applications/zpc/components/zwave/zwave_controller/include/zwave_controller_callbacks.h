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
 * @defgroup zwave_controller_callbacks Z-Wave Controller Callbacks
 * @ingroup zwave_controller_component
 * @brief zwave_controller callbacks registration system.
 *
 * This modules routes events from the Z-Wave Controller sub-blocks (such as
 * Z-Wave RX or network management) and dispatches them to other components that
 * have registered a callback for this particular event. Refer to @ref
 * zwave_controller_register_callbacks
 *
 * <b>Example usage:</b>
 * <DIV>
 * <CODE>
 *    zwave_controller_callbacks_t my_callbacks = {NULL};<BR>
 *    my_callbacks.on_new_suc = my_on_new_suc_function;<BR>
 *    zwave_controller_register_callbacks(my_callbacks);<BR>
 * </CODE>
 * </DIV>
 * After this, my function will now be called when the SUC NodeID changes.
 * Changing the list of functions to invoke can be done runtime:
 * <DIV>
 * <CODE>
 *    my_callbacks.on_node_information = my_on_node_information_function;<BR>
 *    my_callbacks.on_new_suc = NULL;<BR>
 * </CODE>
 * </DIV>
 * The user component must de-register callbacks if the variable is to be destroyed
 * <DIV>
 * <CODE>
 *    zwave_controller_deregister_callbacks(my_callbacks);<BR>
 *    // Now my_callbacks can be destroyed
 * </CODE>
 * </DIV>
 * @{
 */

#ifndef ZWAVE_CONTROLLER_CALLBACKS_H
#define ZWAVE_CONTROLLER_CALLBACKS_H

#include "zwave_controller_types.h"
#include "zwave_rx.h"
#include "zwave_network_management.h"
#include "zwapi_protocol_transport.h"

#include "zwave_generic_types.h"

/**
 * @brief The following structure holds all the callbacks this module will emit.
 *
 */
typedef struct {
  /// This function is called when the network management state changes
  ///
  void (*on_state_updated)(zwave_network_management_state_t state);

  /// This function is called when an error occurs in a network management
  /// operation
  void (*on_error)(zwave_network_management_error_t error);

  /// This function is called when a node has been added to the network and has
  /// been assigned a node ID. Secure provisioning had not yet completed
  /// when this function is called
  /// @param node_id        NodeID of the newly included node.
  /// @param included_by_us Set to true if we included the node, false if another controller included it.
  /// @param inclusion_protocol Set to Z-wave protocol of the inclusion  from \ref zwave_protocol_t
  void (*on_node_id_assigned)(zwave_node_id_t node_id,
                              bool included_by_us,
                              zwave_protocol_t inclusion_protocol);

  /// A node has been deleted from the network
  /// @param node_id NodeID of the node that was removed from our network.
  ///                The value 0 can be used and indicates that a node has been
  ///                excluded from a foreign network.
  ///
  void (*on_node_deleted)(zwave_node_id_t node_id);

  /// This function is called when a node has been assigned its security keys.
  /// This is the end of the protocol part of the inclusion process.
  /// This function is trigged both on initiated with the and_node, add_node_with_dsk
  /// add_node_proxy_inclusion.
  /// @param status Status of the provisioning
  /// @param nif_length length of the node information structure
  /// @param nif points to a node information structure
  /// @param node_id Node ID of the new node.
  /// @param dsk DSK of the new node.
  /// @param granted_keys is used to indicate which network keys were granted during bootstrapping.
  /// @param kex_fail_type This field is used to indicate which error occurred in case S2
  /// @param inclusion_protocol Set to Z-wave protocol of the inclusion
  /// bootstrapping was not successful
  void (*on_node_added)(sl_status_t status,
                        const zwave_node_info_t *nif,
                        zwave_node_id_t node_id,
                        const zwave_dsk_t dsk,
                        zwave_keyset_t granted_keys,
                        zwave_kex_fail_type_t kex_fail_type,
                        zwave_protocol_t inclusion_protocol);

  /// This function is called then the controller enters a new network.
  /// this can happen either after a controller set_default or after
  /// a learn mode operation.
  ///
  /// @param home_id Home ID of the new network.
  /// @param node_id Node ID in the new network.
  /// @param granted_keys is used to indicate which network keys were granted during bootstrapping.
  /// @param kex_fail_type This field is used to indicate which error occurred in case S2
  /// bootstrapping was not successful
  void (*on_new_network_entered)(zwave_home_id_t home_id,
                                 zwave_node_id_t node_id,
                                 zwave_keyset_t granted_keys,
                                 zwave_kex_fail_type_t kex_fail_type);

  /// This function is used to inform which S2 keys have been requested
  /// during S2 bootstrapping, the receiver must call
  /// zwave_network_management_keys_set with the granted keys
  ///
  /// @param csa This field is used to indicate if the joining node is
  /// requesting CSA
  /// @param keys This field is used to advertise the requested keys by
  /// the joining node.
  void (*on_keys_report)(bool csa, zwave_keyset_t keys);

  /// This function is called durring a S2 inclusion, the receiver must
  /// call zwave_network_management_dsk_set with the full dsk of the node
  /// @param input_length Number of leading bytes in the dsk which has been
  /// blanked out.
  /// @param dsk The dsk reported by the device, which input_length bytes
  /// set to zero.
  /// @param keys This field is used to advertise the requested keys by
  /// the joining node.
  void (*on_dsk_report)(uint8_t input_length,
                        zwave_dsk_t dsk,
                        zwave_keyset_t keys);

  /// This callback is fired when a frame has been received and fully
  /// de-encapsulated/decrypted. The application can use this frame.
  ///
  /// @param connection_info Information about the source and destination
  ///                        node if the frame, as well as information about
  ///                        how the frame was encapsulated
  ///
  /// @param rx_options      Receive specific information
  ///
  /// @param frame_data      Pointer to de-encapsulated data
  ///
  /// @param frame_length    Length of data
  ///
  void (*on_application_frame_received)(
    const zwave_controller_connection_info_t *connection_info,
    const zwave_rx_receive_options_t *rx_options,
    const uint8_t *frame_data,
    uint16_t frame_length);

  /// A SmartStart inclusion request was received.
  ///
  /// @param home_id           SmartStart DSK derived HomeId of the Smart
  ///                          Start node wanting to be included
  /// @param already_included  Node is already included into another network.
  ///
  /// @param node_info         Node information frame of the sending node.
  ///
  /// @param inclusion_protocol Used to indicate which protocol is intended for inclusion
  ///
  void (*on_smart_start_inclusion_request)(uint32_t home_id,
                                           bool already_included,
                                           const zwave_node_info_t *node_info,
                                           zwave_protocol_t inclusion_protocol);

  /// node has sent its node info either unsolicited or as a response to a
  /// ZW_RequestNodeInfo call
  /// @param node_id The source of the node information frame.
  ///
  /// @param node_info         Node information frame of the sending node.
  ///
  void (*on_node_information)(zwave_node_id_t node_id,
                              const zwave_node_info_t *node_info);

  /// The SIS node Id was updated
  /// @param suc_node_id Node id of the new SUC
  ///
  void (*on_new_suc)(zwave_node_id_t suc_node_id);

  /// Node Info Request has failed
  /// @param node_id NodeID for which the node info request failed
  void (*on_node_info_req_failed)(zwave_node_id_t node_id);

  /// Z-Wave TX Group was deleted.
  /// @param group_id Group ID that was deleted.
  void (*on_multicast_group_deleted)(zwave_multicast_group_id_t group_id);
  /// Request Node Neighbor Discovery
  /// @param status represents Neighbor Discovery Status
  void (*on_request_neighbor_update)(uint8_t status);
  /// Notify that a singlecast frame transmission has been completed.
  /// @param transmission_successful Indicates if the transmission succeeded or failed.
  /// @param tx_report               Pointer to Tx Report data.
  /// @param node_id                 NodeID for which frame transmission was done
  void (*on_frame_transmission)(bool transmission_successful,
                                const zwapi_tx_report_t *tx_report,
                                zwave_node_id_t node_id);
  /// Notify that we received a frame from a NodeID. It can be both Singlecast,
  /// Multicast, or special frames (e.g. NIF).
  ///
  /// @param node_id         Z-Wave NodeID that sent us a frame.
  ///
  void (*on_rx_frame_received)(zwave_node_id_t node_id);
} zwave_controller_callbacks_t;

/// Function type for a Z-Wave Controller reset step
/// The function MUST return SL_STATUS_OK if it will invoke the zwave_controller_on_reset_step_complete later.
/// Any other status if it will not invoke the zwave_controller_on_reset_step_complete
typedef sl_status_t (*zwave_controller_reset_step_t)(void);

/**
 * @brief Maximum number of callbacks structures which may be registered
 */
#define ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS 20
/**
 * @brief Minimum Priority for a reset step
 */
#define ZWAVE_CONTROLLER_RESET_STEP_MIN_PRIORITY 0

/// Clean-up Associations to ourselves in the network
#define ZWAVE_CONTROLLER_CLEAN_UP_ASSOCIATIONS_STEP_PRIORITY 1
/// Notify the lifeline destinations that we are resetting.
#define ZWAVE_CONTROLLER_DEVICE_RESET_LOCALLY_STEP_PRIORITY 2
/// Clean up the SmartStart as we exit the network
#define ZWAVE_CONTROLLER_SMART_START_LIST_UPDATE_STEP_PRIORITY 3
/// Empty and flush the Tx Queue
#define ZWAVE_CONTROLLER_TX_FLUSH_RESET_STEP_PRIORITY 4
/// reset the network for the Z-Wave API. This should be last.
#define ZWAVE_CONTROLLER_ZWAVE_NETWORK_MANAGEMENT_RESET_STEP_PRIORITY 5

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Let another component register its callbacks for Z-Wave Controller
 * events.
 *
 * This function will register the pointer of a ' \ref
 * zwave_controller_callbacks_t variable passed by another component.
 *
 * Another component registering callbacks must provide a valid/allocated
 * pointer. \ref zwave_controller_callbacks_t members left to NULL will not be
 * called.
 *
 * Callbacks must not take a long time. If they need further processing or to
 * trigger a long procedure, they must copy the data to their respective
 * component and/or trigger an asynchronous \ref contiki event.
 *
 * @param callbacks \ref callbacks pointer for the zwave_controller component.
 * All callbacks are executed in the context of the zwave_controller process.
 *
 * The caller may provide a callback struct pointer containing a set of NULL and
 * valid functions. The \ref zwave_controller Component keeps the pointer value,
 * so a component can subsequently deregister a function by changing it to NULL
 * or a new value in its zwave_controller_callbacks_t variable.
 *
 * If all functions are NULL in a zwave_controller_callbacks_t variable, the
 * component should deregister its callback variable by calling \ref
 * zwave_controller_deregister_callbacks.
 *
 * Components must deregister their callbacks before shutting down and
 * destroying the zwave_controller_callbacks_t variables passed to this
 * function.
 *
 * @returns  \ref SL_STATUS_OK if the callback pointer has been registered.
 * @returns  \ref SL_STATUS_FAIL if the callback pointer is invalid
 * @returns  \ref SL_STATUS_NO_MORE_RESOURCE if the maximum number of
 *           subscribers registered callbacks.
 */
sl_status_t zwave_controller_register_callbacks(
  const zwave_controller_callbacks_t *callbacks);

/**
 * @brief Let another component deregister its callbacks for Z-Wave Controller
 * events
 *
 * This function will deregister a pointer to a \ref
 * zwave_controller_callbacks_t array.
 * @param callbacks \ref zwave_controller_callbacks_t callbacks pointer.
 *
 * @returns \ref SL_STATUS_OK if the callback pointer has been deregistered or
 *          was already unregistered.
 * @returns \ref SL_STATUS_FAIL if an error happened e.g. invalid value.
 */
sl_status_t zwave_controller_deregister_callbacks(
  const zwave_controller_callbacks_t *callbacks);

/**
 * @brief Let another component register a step to perform when being reset
 *
 * This function will register a pointer to a \ref
 * zwave_controller_reset_step_t array.
 * @param step_function \ref zwave_controller_reset_step_t function to invoke
 * when being reset.
 * @param priority      Priority of the step. Step 0 is executed first,
 *                      step 1 second, etc. This value MUST be between 0 and
 *                      ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS - 1
 *
 * @returns SL_STATUS_OK if the callback pointer has been registered with the
 *          requested priority
 *          SL_STATUS_FAIL if an error happened,
 *          e.g. 2 steps with the same priority are registered.
 */
sl_status_t zwave_controller_register_reset_step(
  const zwave_controller_reset_step_t step_function, int32_t priority);

/**
 * @brief Informs the Z-Wave Controller that a Reset Step is completed.
 *
 * @param priority      Priority of the step that was just completed.
 *
 */
void zwave_controller_on_reset_step_complete(int32_t priority);

/**
 * @brief Reset all callback structures.
 *
 */
void zwave_controller_callbacks_init();

#ifdef __cplusplus
}
#endif
/** @} end zwave_controller_callbacks */

#endif  //ZWAVE_CONTROLLER_CALLBACKS_H
