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
#include "zcl_rf_telemetry_cluster_server.h"

// Includes from the Z-Wave controller
#include "zwave_controller_callbacks.h"
#include "zwave_network_management.h"
#include "zwave_rx.h"

// ZPC Includes
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwapi_protocol_basis.h"
#include "zwave_api_transport.h"

// Unify Includes
#include "sl_log.h"
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_supported_generated_commands.h"
#include "dotdot_mqtt_generated_commands.h"
#include "attribute_store_helper.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_defined_attribute_types.h"

// Generic includes
#include <string.h>
#include <stdlib.h>

#define LOG_TAG "zcl_zwave_telemetry_cluster_server"

/// Default value for Tx Report enabled the first time we create the attribute.
#define DEFAULT_TX_REPORT_ENABLED false

/// Under which ZPC endpoint the RF Telemetry is available.
#define ZPC_ENDPOINT_ID 0

#define ATTRIBUTE(type) \
  DOTDOT_ATTRIBUTE_ID_PROTOCOL_CONTROLLER_RF_TELEMETRY_##type
///////////////////////////////////////////////////////////////////////////////
// Private helper functions functions.
//////////////////////////////////////////////////////////////////////////////
static void create_rf_telemetry_attributes()
{
  // Make sure we have the TxReportEnabled ZCL attribute created under the ZPC.
  attribute_store_node_t zpc_endpoint
    = get_zpc_endpoint_id_node(ZPC_ENDPOINT_ID);

  attribute_store_node_t tx_report_enabled
    = attribute_store_get_first_child_by_type(zpc_endpoint,
                                              ATTRIBUTE(TX_REPORT_ENABLED));

  if (tx_report_enabled == ATTRIBUTE_STORE_INVALID_NODE) {
    tx_report_enabled_t value = DEFAULT_TX_REPORT_ENABLED;
    attribute_store_set_child_reported(zpc_endpoint,
                                       ATTRIBUTE(TX_REPORT_ENABLED),
                                       &value,
                                       sizeof(value));
  }

  // Make sure we have the PTIEnabled ZCL attribute created under the ZPC.
  attribute_store_node_t pti_enabled_node
    = attribute_store_get_first_child_by_type(zpc_endpoint,
                                              ATTRIBUTE(PTI_ENABLED));

  if (pti_enabled_node == ATTRIBUTE_STORE_INVALID_NODE) {
    if (zwapi_is_pti_supported()) {
      pti_enabled_t value = zwapi_is_pti_enabled();
      attribute_store_set_child_reported(zpc_endpoint,
                                         ATTRIBUTE(PTI_ENABLED),
                                         &value,
                                         sizeof(value));
    }
  } else {
    if (!zwapi_is_pti_supported()) {
      sl_log_debug(LOG_TAG,
                   "PTI is not supported. Removing the attribute store node");
      attribute_store_delete_node(pti_enabled_node);
    }
  }
}

/**
 * @brief Publishes the RFTelemetry supported generated commands.
 *
 */
static void publish_rf_telemetry_supported_generated_commands()
{
  attribute_store_node_t zpc_endpoint
    = get_zpc_endpoint_id_node(ZPC_ENDPOINT_ID);

  unid_t zpc_unid;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_unid_from_node(zpc_endpoint,
                                                           zpc_unid)) {
    sl_log_warning(LOG_TAG,
                   "Cannot retrieve ZPC UNID, SupportedGeneratedCommands will "
                   "not be published for RFTelemetry cluster");
    return;
  }

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_supported_commands_t
    command_list
    = {.tx_report = true};
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands(
    zpc_unid,
    &command_list);
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions.
//////////////////////////////////////////////////////////////////////////////
static void on_rf_telemetry_tx_report_enabled_desired_update(
  attribute_store_node_t node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // Whenever the desired value of the RF Telemetry is updated, we just align the reported
  if (attribute_store_is_value_defined(node, DESIRED_ATTRIBUTE)) {
    attribute_store_set_reported_as_desired(node);
  }
}

static void
  on_rf_telemetry_pti_enabled_desired_update(attribute_store_node_t node,
                                             attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // Find our UNID.
  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
  unid_t zpc_unid;
  zwave_unid_from_node_id(zpc_node_id, zpc_unid);

  pti_enabled_t rf_telemetry_pti_enabled
    = dotdot_get_protocol_controller_rf_telemetry_pti_enabled(
      zpc_unid,
      ZPC_ENDPOINT_ID,
      DESIRED_ATTRIBUTE);

  // If the attribute store and the Z-Wave module say the same
  // No need of setting PTI in Z-Wave module
  if (rf_telemetry_pti_enabled != zwapi_is_pti_enabled()) {
    if (zwapi_set_radio_pti(rf_telemetry_pti_enabled) != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Could not set PTI to %d\n",
                   (int)rf_telemetry_pti_enabled);
      if (!zwapi_is_pti_supported()) {
        sl_log_debug(LOG_TAG, "PTI is not supported\n");
      }
      // Roll back the desired value as it was rejected
      attribute_store_set_desired_as_reported(node);
      return;
    }
    zwapi_soft_reset();
    zwave_rx_wait_for_zwave_api_to_be_ready();
    zwave_api_transport_reset();
  }
  // Whenever the desired value of the RF Telemetry is updated, we just align the reported
  if (attribute_store_is_value_defined(node, DESIRED_ATTRIBUTE)) {
    attribute_store_set_reported_as_desired(node);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Z-Wave Controller callback functions.
//////////////////////////////////////////////////////////////////////////////
static void
  zcl_zwave_telemetry_on_frame_transmission(bool transmission_successful,
                                            const zwapi_tx_report_t *tx_report,
                                            zwave_node_id_t node_id)
{
  // Find our UNID.
  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
  unid_t zpc_unid;
  zwave_unid_from_node_id(zpc_node_id, zpc_unid);

  tx_report_enabled_t rf_telemetry_tx_report_enabled
    = dotdot_get_protocol_controller_rf_telemetry_tx_report_enabled(
      zpc_unid,
      ZPC_ENDPOINT_ID,
      REPORTED_ATTRIBUTE);

  if (rf_telemetry_tx_report_enabled == 0) {
    return;
  }

  // No Tx Report ? (Can happen e.g. if Request NIF completed)
  if (tx_report == NULL) {
    return;
  }

  // Time to send the data upwards. Find the UNID of the destination.
  unid_t remote_node_unid;
  zwave_unid_from_node_id(node_id, remote_node_unid);

  // Convert some of the Z-Wave API Tx Report into the equivalent dotdot:
  uint8_t transmission_speed = ZCL_TX_REPORT_TRANSMISSION_SPEED_UNKNOWN;
  switch (tx_report->last_route_speed) {
    case ZWAVE_9_6_KBITS_S:
      transmission_speed
        = ZCL_TX_REPORT_TRANSMISSION_SPEED_Z_WAVE_9600_BITS_PER_SECOND;
      break;
    case ZWAVE_40_KBITS_S:
      transmission_speed
        = ZCL_TX_REPORT_TRANSMISSION_SPEED_Z_WAVE_40_KBITS_PER_SECOND;
      break;
    case ZWAVE_100_KBITS_S:
      transmission_speed
        = ZCL_TX_REPORT_TRANSMISSION_SPEED_Z_WAVE_100_KBITS_PER_SECOND;
      break;
    case ZWAVE_LONG_RANGE_100_KBITS_S:
      transmission_speed
        = ZCL_TX_REPORT_TRANSMISSION_SPEED_Z_WAVE_LONG_RANGE_100_KBITS_PER_SECOND;
      break;
    default:
      break;
  }

  unid_t last_link_functional_unid = "";
  if (tx_report->last_failed_link.from != 0) {
    zwave_unid_from_node_id(tx_report->last_failed_link.from,
                            last_link_functional_unid);
  }
  unid_t last_link_non_functional_unid = "";
  if (tx_report->last_failed_link.to != 0) {
    zwave_unid_from_node_id(tx_report->last_failed_link.to,
                            last_link_non_functional_unid);
  }

  // Prepare an array of UNIDs for the route.
  char **repeaters_unid = NULL;
  if (tx_report->number_of_repeaters > 0) {
    repeaters_unid = malloc(sizeof(char *) * tx_report->number_of_repeaters);
    for (uint8_t i = 0; i < tx_report->number_of_repeaters; i++) {
      repeaters_unid[i] = malloc(sizeof(unid_t));
      zwave_unid_from_node_id(tx_report->last_route_repeaters[i],
                              repeaters_unid[i]);
    }
  }

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_command_tx_report_fields_t
    fields
    = {};
  fields.sourceunid              = zpc_unid;
  fields.destinationunid         = remote_node_unid;
  fields.transmission_successful = transmission_successful;
  fields.transmission_time_ms    = tx_report->transmit_ticks * 10;
  fields.tx_powerd_bm            = tx_report->tx_power;
  fields.tx_channel              = tx_report->tx_channel_number;
  fields.routing_attempts        = tx_report->routing_attempts;
  fields.route_changed
    = (tx_report->route_scheme_state == ROUTINGSCHEME_CACHED_ROUTE) ? false
                                                                    : true;
  fields.transmission_speed           = transmission_speed;
  fields.measured_noise_floord_bm     = tx_report->measured_noise_floor;
  fields.last_route_repeaters_count   = tx_report->number_of_repeaters;
  fields.last_route_repeaters         = (const char **)repeaters_unid;
  fields.incoming_rssi_repeaters_count = tx_report->number_of_repeaters;
  fields.incoming_rssi_repeaters
    = (const int8_t *)tx_report->rssi_values.incoming;
  fields.ackrssi                               = tx_report->ack_rssi;
  fields.ack_channel                           = tx_report->ack_channel_number;
  fields.last_route_failed_link_functionalunid = last_link_functional_unid;
  fields.last_route_failed_link_non_functionalunid
    = last_link_non_functional_unid;
  fields.destination_ack_tx_powerd_bm
    = tx_report->destination_ack_mpdu_tx_power;
  fields.destination_ack_measuredrssi
    = tx_report->destination_ack_mpdu_measured_rssi;
  fields.destination_ack_measured_noise_floor
    = tx_report->destination_ack_mpdu_measured_noise_floor;

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command(
    zpc_unid,
    &fields);

  // Clear the malloc'ed memory again
  if (tx_report->number_of_repeaters > 0) {
    for (uint8_t i = 0; i < fields.last_route_repeaters_count; i++) {
      free(repeaters_unid[i]);
    }
    free(repeaters_unid);
  }
}

static zwave_controller_callbacks_t zcl_zwave_telemetry_callbacks
  = {.on_frame_transmission = &zcl_zwave_telemetry_on_frame_transmission};

///////////////////////////////////////////////////////////////////////////////
//  Init and teardown functions.
//////////////////////////////////////////////////////////////////////////////
sl_status_t zcl_rf_telemetry_cluster_server_init()
{
  // Ask the Z-Wave Controller to tell us about finished transmissions.
  zwave_controller_register_callbacks(&zcl_zwave_telemetry_callbacks);

  // Make sure we have the ZCL attributes under our endpoint
  create_rf_telemetry_attributes();

  // listen to tx report enabled attribute updates and accept them
  attribute_store_register_callback_by_type_and_state(
    &on_rf_telemetry_tx_report_enabled_desired_update,
    ATTRIBUTE(TX_REPORT_ENABLED),
    DESIRED_ATTRIBUTE);

  // listen to pti enabled attribute updates and accept them
  attribute_store_register_callback_by_type_and_state(
    &on_rf_telemetry_pti_enabled_desired_update,
    ATTRIBUTE(PTI_ENABLED),
    DESIRED_ATTRIBUTE);

  // Publish the supported Generated Commands
  publish_rf_telemetry_supported_generated_commands();

  // If we just initialized, make sure to publish the SupportedCommands
  attribute_store_node_t zpc_endpoint
    = get_zpc_endpoint_id_node(ZPC_ENDPOINT_ID);
  unid_t zpc_unid;
  if (SL_STATUS_OK
      == attribute_store_network_helper_get_unid_from_node(zpc_endpoint,
                                                           zpc_unid)) {
    uic_mqtt_dotdot_publish_supported_commands(zpc_unid, ZPC_ENDPOINT_ID);
  }

  return SL_STATUS_OK;
}
