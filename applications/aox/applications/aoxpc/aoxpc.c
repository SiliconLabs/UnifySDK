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
// Standard includes
#include <stdlib.h>

// Unify components
#include "sl_log.h"
#include "process.h"
#include "dotdot_mqtt.h"

// AoXPC
#include "aoxpc.h"
#include "aoxpc_config.h"
#include "aoxpc_configuration_handler.h"
#include "aoxpc_unid.h"
#include "aox_locator_configuration.h"
#include "ucl_mqtt_helper.h"
#include "ncp.h"
#include "aoxpc_datastore_fixt.h"

// Gecko SDK
#include "sl_bt_api.h"
#include "aoa_db.h"
#include "aoa_cte.h"

#ifdef AOA_ANGLE
#include "aoa_angle.h"
#include "aoxpc_correction.h"
#endif // AOA_ANGLE

#define LOG_TAG "aoxpc"

#define panic()  process_post(PROCESS_BROADCAST, PROCESS_EVENT_EXIT, 0)

static void sl_bt_on_event(sl_bt_msg_t *evt);
static void aoxpc_teardown(void);

// AoXPC process definition
PROCESS(aoxpc_process, "AoXPC process");

// AoXPC process implementation
PROCESS_THREAD(aoxpc_process, ev, data)
{
  PROCESS_BEGIN()
  while (1) {
    if (ev == PROCESS_EVENT_EXIT) {
      aoxpc_teardown();
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}

sl_status_t aoxpc_fixt_setup(void)
{
  process_start(&aoxpc_process, NULL);
  ncp_set_cb(sl_bt_on_event);
  aox_locator_register_on_configuration_updated_callback(
    &aoxpc_configuration_handler_on_aox_locator_configuration_updated);
  return SL_STATUS_OK;
}

void aoa_cte_on_iq_report(aoa_db_entry_t *tag, aoa_iq_report_t *iq_report)
{
  aoa_id_t tag_unid;

  aoa_address_to_id(tag->address.addr, tag->address_type, tag_unid);

  // Fetch the AoXPC UNID
  aoa_id_t locator_unid = {};
  if (SL_STATUS_OK != get_aoxpc_unid(locator_unid)) {
    sl_log_error(LOG_TAG,
                 "Could not retrieve AoXPC UNID. "
                 "MQTT publish will be corrupted");
  }

#ifndef AOA_ANGLE
  // Assemble and publish command.
  uic_mqtt_dotdot_aox_locator_command_iq_report_fields_t command;
  command.tag_unid = tag_unid;
  command.channel = iq_report->channel;
  command.rssi = iq_report->rssi;
  command.sequence = iq_report->event_counter;
  command.samples_count = iq_report->length;
  command.samples = iq_report->samples;

  uic_mqtt_dotdot_aox_locator_publish_generated_iq_report_command(
    locator_unid,
    AOXPC_ENDPOINT_ID,
    &command);

#else // AOA_ANGLE
  enum sl_rtl_error_code ec;
  aoa_angle_t angle;
  ec = aoa_calculate((aoa_state_t *)tag->user_data,
                     iq_report, &angle,
                     locator_unid);
  if (ec == SL_RTL_ERROR_ESTIMATION_IN_PROGRESS) {
    // No valid angles are available yet.
    return;
  }
  if (ec != SL_RTL_ERROR_SUCCESS) {
    sl_log_error(LOG_TAG, "aoa_calculate failed: %d", ec);
    return;
  }

  // Store the latest sequence number for the tag.
  tag->sequence = iq_report->event_counter;

  // Assemble and publish command.
  uic_mqtt_dotdot_aox_locator_command_angle_report_fields_t command;
  command.tag_unid = tag_unid;
  command.direction.Azimuth = angle.azimuth;
  command.direction.Elevation = angle.elevation;
  command.direction.Distance = angle.distance;
  // Standard deviation estimation not supported yet.
  command.deviation.Azimuth = 0;
  command.deviation.Elevation = 0;
  command.deviation.Distance = 0;
  command.sequence = angle.sequence;

  uic_mqtt_dotdot_aox_locator_publish_generated_angle_report_command(
    locator_unid,
    AOXPC_ENDPOINT_ID,
    &command);
#endif // AOA_ANGLE
}

// Bluetooth stack event handler
static void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  const char *config_file = NULL;

  // Track bt system reset
  static bool first_bt_system_reset = true;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Print boot message.
      sl_log_info(LOG_TAG, "Bluetooth stack booted: v%d.%d.%d-b%d",
                  evt->data.evt_system_boot.major,
                  evt->data.evt_system_boot.minor,
                  evt->data.evt_system_boot.patch,
                  evt->data.evt_system_boot.build);
      // Get BT Address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      if (sc != SL_STATUS_OK) {
        sl_log_error(LOG_TAG, "sl_bt_system_get_identity_address failed: 0x%04x", sc);
        panic();
        return;
      }
      sl_log_info(LOG_TAG, "Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X",
                  address_type ? "static random" : "public device",
                  address.addr[5],
                  address.addr[4],
                  address.addr[3],
                  address.addr[2],
                  address.addr[1],
                  address.addr[0]);

      if (first_bt_system_reset == true) {
        // Execute this part only after first reset
        first_bt_system_reset = false;

        set_aoxpc_address(address.addr, address_type);

        aoa_id_t locator_unid = {};
        if (SL_STATUS_OK != get_aoxpc_unid(locator_unid)) {
          sl_log_error(LOG_TAG,
                       "Could not retrieve AoXPC UNID. "
                       "Cannot publish AoXPC node state.");
        }

        publish_node_state(locator_unid, true);

        #ifdef AOA_ANGLE
        aoa_angle_config_t *config;
        aoa_angle_add_config(locator_unid, &config);
        // Share antenna array configuration with the CTE component.
        aoa_cte_config.antenna_array = &config->antenna_array;
        #endif // AOA_ANGLE

        // Load the previous AoXLocator configuration from the attribute store
        // Config file is loaded afterwards, which will overwrite the persisted values
        if (SL_STATUS_OK
            != aox_locator_load_attributes_from_attribute_store()) {
          sl_log_warning(LOG_TAG,
                         "Could not load previous AoXLocator attributes "
                         "from the Attribute Store ");
        }

        // Parse config file if given
        if (CONFIG_STATUS_OK
            != config_get_as_string(CONFIG_KEY_AOXPC_CONFIG_FILE,
                                    &config_file)) {
          sl_log_error(LOG_TAG,
                       "Failed to retrieve configuration for %s",
                       CONFIG_KEY_AOXPC_CONFIG_FILE);
        }

        if ((config_file != NULL) && (strlen(config_file) > 0)) {
          aoxpc_configuration_handler_parse_config_file(config_file);
        } else {
          // No config file, tell the configuration component that the
          // config was just "updated" from what was loaded from the
          // attribute store
          aoxpc_configuration_handler_on_aox_locator_configuration_updated();
        }

        // Make sure that the attributes are published at startup even if
        // the database file is empty and no config file is provided.
        publish_aox_locator_attributes_to_mqtt();

        // Assuming all dotdot MQTT callbacks are set, ask to publish our
        // supported commands for all clusters.
        sl_log_debug(LOG_TAG, "Publishing SupportedCommands for all clusters");
        uic_mqtt_dotdot_publish_supported_commands(locator_unid,
                                                   AOXPC_ENDPOINT_ID);
      }
      break;
    // -------------------------------
    // Add further event handlers here.
  }
  // Call the CTE specific event handler.
  sc = aoa_cte_bt_on_event(evt);
  if (sc != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Not recoverable status 0x%04x", sc);
    panic();
  }
}

/**************************************************************************//**
 * Tag added callback.
 *****************************************************************************/
void aoa_db_on_tag_added(aoa_db_entry_t *tag)
{
#ifdef AOA_ANGLE
  enum sl_rtl_error_code ec;

  tag->user_data = malloc(sizeof(aoa_state_t));
  if (NULL == tag->user_data) {
    sl_log_critical(LOG_TAG, "Failed to allocate memory for aoa state.");
    panic();
    return;
  }

  aoa_id_t locator_unid = {};
  if (SL_STATUS_OK != get_aoxpc_unid(locator_unid)) {
    sl_log_critical(LOG_TAG,
                    "Could not retrieve AoXPC UNID. "
                    "aoa_init_rtl will fail.");
    panic();
    return;
  }

  ec = aoa_init_rtl((aoa_state_t *)tag->user_data, locator_unid);
  if (ec != SL_RTL_ERROR_SUCCESS) {
    sl_log_critical(LOG_TAG, "aoa_init_rtl failed: %d", ec);
    panic();
    return;
  }
#endif // AOA_ANGLE

  size_t tags = aoa_db_get_number_of_tags();
  sl_log_info(LOG_TAG, "New tag added (%zu): %02X:%02X:%02X:%02X:%02X:%02X",
              tags,
              tag->address.addr[5],
              tag->address.addr[4],
              tag->address.addr[3],
              tag->address.addr[2],
              tag->address.addr[1],
              tag->address.addr[0]);
}

static void aoxpc_teardown(void)
{
  // Check if UNID is initialized, if yes, unpublish its node state
  aoa_id_t locator_unid = {};
  if (SL_STATUS_OK == get_aoxpc_unid(locator_unid)) {
    publish_node_state(locator_unid, false);
  }
  // Remove all retained topics except ucl/by-unid/<aoxpc_unid>/State
  uic_mqtt_unretain_by_regex("^(?!ucl\\/by-unid\\/.*\\/State$).*");
}

/**************************************************************************//**
 * Tag removed callback.
 *****************************************************************************/
void aoa_db_on_tag_removed(aoa_db_entry_t *tag)
{
#ifdef AOA_ANGLE
  aoa_id_t locator_unid = {};
  if (SL_STATUS_OK != get_aoxpc_unid(locator_unid)) {
    sl_log_critical(LOG_TAG,
                    "Could not retrieve AoXPC UNID. "
                    "aoa_deinit_rtl will fail.");
    panic();
    return;
  }

  enum sl_rtl_error_code ec;
  ec = aoa_deinit_rtl((aoa_state_t *)tag->user_data, locator_unid);
  if (ec != SL_RTL_ERROR_SUCCESS) {
    sl_log_critical(LOG_TAG, "aoa_deinit_rtl failed: %d", ec);
    panic();
    return;
  }
  free(tag->user_data);
#endif // AOA_ANGLE
  sl_log_info(LOG_TAG, "Tag removed: %02X:%02X:%02X:%02X:%02X:%02X",
              tag->address.addr[5],
              tag->address.addr[4],
              tag->address.addr[3],
              tag->address.addr[2],
              tag->address.addr[1],
              tag->address.addr[0]);
}
