/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// Includes from this component
#include "zwave_command_class_version.h"

// Generic includes
#include <assert.h>
#include <stdbool.h>

// Interfaces includes
#include "ZW_classcmd.h"
#include "zwave_command_class_version_types.h"

// Includes from other components
#include "zwave_command_handler.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_utils.h"
#include "zwave_rx.h"
#include "zwave_controller_transport.h"
#include "zwave_controller_command_class_indices.h"

#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_resolver.h"

#include "zwave_unid.h"
#include "zwave_command_classes_utils.h"
#include "zwapi_init.h"
#include "zpc_config.h"
#include "uic_version.h"

#include "sl_log.h"
#define LOG_TAG "zwave_command_class_version"

#define NUM_FW_TARGETS 1
// Default value if If no hardware version is provided (Version CC v1)
#define DEFAULT_HARDWARE_VERSION 1

// Frame parsing indices:
#define REPORT_LIBRARY_TYPE_INDEX               2
#define REPORT_ZWAVE_PROTOCOL_VERSION_INDEX     3
#define REPORT_ZWAVE_PROTOCOL_SUB_VERSION_INDEX 4
#define REPORT_FIRMWARE_0_VERSION_INDEX         5
#define REPORT_FIRMWARE_0_SUB_VERSION_INDEX     6
#define REPORT_HARDWARE_VERSION_INDEX           7
#define REPORT_NUMBER_OF_TARGETS_INDEX          8
#define REPORT_FIRST_ADDITIONAL_TARGET_INDEX    9

// Attribute macro, shortening those long defines:
#define ATTRIBUTE(type) ATTRIBUTE_CC_VERSION_##type

// Constants
static const attribute_store_type_t version_report_attribute[]
  = {ATTRIBUTE(VERSION_REPORT_DATA)};

static const attribute_store_type_t version_attributes_v3[]
  = {ATTRIBUTE(SDK_VERSION),
     ATTRIBUTE(APPLICATION_FRAMEWORK_API_VERSION),
     ATTRIBUTE(APPLICATION_FRAMEWORK_BUILD_NUMBER),
     ATTRIBUTE(HOST_INTERFACE_VERSION),
     ATTRIBUTE(HOST_INTERFACE_BUILD_NUMBER),
     ATTRIBUTE(ZWAVE_PROTOCOL_BUILD_NUMBER),
     ATTRIBUTE(APPLICATION_BUILD_NUMBER)};

// Forward declarations
static sl_status_t zwave_command_class_version_command_class_version_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len);

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Visits other endpoints under the same NodeID and verify if a particular
 * Command Class version is known
 *
 * If the version of a command class is known from another endpoint, it will be
 * copied to the current endpoint.
 *
 * @param node  Command Class version attribute.
 *
 * @returns true if the version of a Command Class is known and has been copied
 *          from another endpoint
 * @returns false otherwise
 */
static bool is_command_class_version_already_known(attribute_store_node_t node)
{
  // Get the EndpointID/NodeID nodes:
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_NODE_ID);

  // Visit all endpoints
  for (size_t i = 0; i < attribute_store_get_node_child_count(node_id_node);
       i++) {
    attribute_store_node_t current_endpoint_id_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               i);
    if (current_endpoint_id_node == ATTRIBUTE_STORE_INVALID_NODE) {
      break;
    }
    if (current_endpoint_id_node == endpoint_id_node) {
      continue;
    }

    attribute_store_node_t command_class_version_node
      = attribute_store_get_node_child_by_type(
        current_endpoint_id_node,
        attribute_store_get_node_type(node),
        0);

    if (true
        == attribute_store_is_value_defined(command_class_version_node,
                                            REPORTED_ATTRIBUTE)) {
      // Copy the reported value under our endpoint.
      //attribute_store
      attribute_store_copy_value(command_class_version_node,
                                 node,
                                 REPORTED_ATTRIBUTE);
      return true;
    }
  }
  return false;
}

static void
  add_command_class_version_attributes(zwave_command_class_t command_class,
                                       attribute_store_node_t endpoint_node)
{
  if (command_class < COMMAND_CLASS_FIRST_APPLICATION_CC) {
    return;
  }
  // Prepare an attribute array with 1 Command Class
  const attribute_store_type_t version_attribute[]
    = {ZWAVE_CC_VERSION_ATTRIBUTE(command_class)};

  // register a resolution rule
  attribute_resolver_register_rule(
    version_attribute[0],
    NULL,
    zwave_command_class_version_command_class_version_get);
  attribute_resolver_set_attribute_depth(version_attribute[0], 0);

  // Add it into the attribute store.
  attribute_store_add_if_missing(endpoint_node,
                                 version_attribute,
                                 COUNT_OF(version_attribute));

  // If the node does not support Version CC, set the attribute value to 1
  zwave_node_id_t node_id = 0;
  attribute_store_network_helper_get_node_id_from_node(endpoint_node, &node_id);

  // Check version support on endpoint 0, root device.
  if (!is_version_cc_found(endpoint_node)) {
    zwave_cc_version_t default_version = 1;
    attribute_store_set_uint8_child_by_type(endpoint_node,
                                            version_attribute[0],
                                            REPORTED_ATTRIBUTE,
                                            default_version);
  }
}

static void parse_nif_and_create_command_class_version_attributes(
  const uint8_t *nif, uint8_t nif_length, attribute_store_node_t endpoint_node)
{
  zwave_command_class_t supported_cc_list[ZWAVE_MAX_NIF_SIZE] = {0};
  size_t supported_cc_list_length                             = 0;

  // Retrieve the CCs from the NIF to our array.
  // Do we want to save the version of controlled CCs ? I don't think so.
  zwave_parse_nif(nif,
                  nif_length,
                  supported_cc_list,
                  &supported_cc_list_length,
                  NULL,
                  NULL);

  for (size_t i = 0; i < supported_cc_list_length; i++) {
    add_command_class_version_attributes(supported_cc_list[i], endpoint_node);
  }
}

static void zwave_command_class_version_create_cc_version_attributes(
  zwave_node_id_t node_id, zwave_endpoint_id_t endpoint_id)
{
  // Find out the UNID of the node
  unid_t unid;
  zwave_unid_from_node_id(node_id, unid);

  // Retrieve the Attribute Store node for the endpoint:
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint_id);

  // Find the Non-Secure NIF under the endpoint
  attribute_store_node_t non_secure_nif_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE_ZWAVE_NIF,
                                             0);

  uint8_t nif[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  uint8_t nif_length = 0;

  if ((non_secure_nif_node != ATTRIBUTE_STORE_INVALID_NODE)
      && (SL_STATUS_OK
          == attribute_store_get_node_attribute_value(non_secure_nif_node,
                                                      REPORTED_ATTRIBUTE,
                                                      nif,
                                                      &nif_length))) {
    parse_nif_and_create_command_class_version_attributes(nif,
                                                          nif_length,
                                                          endpoint_node);
  }

  // Same process for the Secure NIF
  attribute_store_node_t secure_nif_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE_ZWAVE_SECURE_NIF,
                                             0);
  if (SL_STATUS_OK
      == attribute_store_get_node_attribute_value(secure_nif_node,
                                                  REPORTED_ATTRIBUTE,
                                                  nif,
                                                  &nif_length)) {
    parse_nif_and_create_command_class_version_attributes(nif,
                                                          nif_length,
                                                          endpoint_node);
  }
}

/**
 * @brief Retrieves (and create if needed) the firmware ID node for a given ID
 * under the version report data.
 *
 * @param version_report_data_node  Attribute Store node for the Version Report data
 * @param firmware_id               Firmware ID to look for and create
 *
 * @returns attribute_store_node_t
 */
static attribute_store_node_t get_or_create_firmware_id_node(
  attribute_store_node_t version_report_data_node, uint32_t firmware_id)
{
  attribute_store_node_t firmware_id_node
    = attribute_store_get_node_child_by_value(version_report_data_node,
                                              ATTRIBUTE(FIRMWARE),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&firmware_id,
                                              sizeof(firmware_id),
                                              0);

  if (firmware_id_node == ATTRIBUTE_STORE_INVALID_NODE) {
    firmware_id_node
      = attribute_store_add_node(ATTRIBUTE(FIRMWARE), version_report_data_node);
    attribute_store_set_reported(firmware_id_node,
                                 &firmware_id,
                                 sizeof(firmware_id));
  }
  return firmware_id_node;
}

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  zwave_command_class_version_handle_command_class_report_command(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length < 4) {
    // Frame too short?? Just ignore it and return SUCCESS to supervision
    // Since it is a report.
    return SL_STATUS_OK;
  }

  // Find the NodeID in the attribute store.
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(
      connection_info->remote.node_id);

  // Fill up the Version of the Command Class for all endpoints
  // that support this command class.
  // (Root device answers also on behalf of endpoints here)
  uint32_t current_child = 0;

  attribute_store_node_t endpoint_id_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             current_child);
  current_child += 1;

  while (endpoint_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    // Set the CC version on the version attributes of all endpoints.
    // If the endpoint does not have the version CC this operation
    // has no effect
    attribute_store_set_uint8_child_by_type(
      endpoint_id_node,
      ZWAVE_CC_VERSION_ATTRIBUTE(frame_data[2]),
      REPORTED_ATTRIBUTE,
      frame_data[3]);

    endpoint_id_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               current_child);
    current_child += 1;
  }

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_version_handle_capabilities_report_command(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length < 3) {
    // Frame too short?? Just ignore it and return SUCCESS to supervision
    // Since it is a report.
    return SL_STATUS_OK;
  }

  // Find out the unid of the sending node:
  unid_t supporting_node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id,
                          supporting_node_unid);

  // Get the corresponding Attribute Store entry for this unid / endpoint:
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(
      supporting_node_unid,
      connection_info->remote.endpoint_id);

  bool get_zwave_software
    = (frame_data[2]
       & VERSION_CAPABILITIES_REPORT_PROPERTIES1_Z_WAVE_SOFTWARE_BIT_MASK_V3)
      > 0;
  attribute_store_set_uint8_child_by_type(endpoint_node,
                                          ATTRIBUTE(ZWAVE_SOFTWARE_GET_SUPPORT),
                                          REPORTED_ATTRIBUTE,
                                          get_zwave_software);

  if (get_zwave_software == true) {
    attribute_store_add_if_missing(endpoint_node,
                                   version_attributes_v3,
                                   COUNT_OF(version_attributes_v3));
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_version_handle_software_report_command(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 25) {
    // FIXME: We should parse what is available and not bail out if the frame
    // is missing e.g. the last 2 bytes
    return SL_STATUS_OK;
  }

  uint32_t sdk_version                  = 0;
  uint32_t zwave_api_version            = 0;
  uint32_t zwave_api_built_version      = 0;
  uint32_t host_interface_version       = 0;
  uint32_t host_interface_built_version = 0;
  uint32_t protocol_built_version       = 0;
  uint32_t application_build_number     = 0;

  // Find out the unid of the sending node:
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);
  // Some data is repeated from the version report, except that it is more precise
  attribute_store_node_t version_report_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION_REPORT_DATA),
                                             0);

  sdk_version = (frame_data[2] << 16) | (frame_data[3] << 8) | (frame_data[4]);
  attribute_store_set_uint32_child_by_type(endpoint_node,
                                           ATTRIBUTE(SDK_VERSION),
                                           REPORTED_ATTRIBUTE,
                                           sdk_version);

  zwave_api_version
    = (frame_data[5] << 16) | (frame_data[6] << 8) | (frame_data[7]);
  attribute_store_set_uint32_child_by_type(
    endpoint_node,
    ATTRIBUTE(APPLICATION_FRAMEWORK_API_VERSION),
    REPORTED_ATTRIBUTE,
    zwave_api_version);

  zwave_api_built_version = (frame_data[8] << 8) | (frame_data[9]);
  attribute_store_set_uint32_child_by_type(
    endpoint_node,
    ATTRIBUTE(APPLICATION_FRAMEWORK_BUILD_NUMBER),
    REPORTED_ATTRIBUTE,
    zwave_api_built_version);

  host_interface_version
    = (frame_data[10] << 16) | (frame_data[11] << 8) | (frame_data[12]);
  attribute_store_set_uint32_child_by_type(endpoint_node,
                                           ATTRIBUTE(HOST_INTERFACE_VERSION),
                                           REPORTED_ATTRIBUTE,
                                           host_interface_version);

  host_interface_built_version = (frame_data[13] << 8) | (frame_data[14]);
  attribute_store_set_uint32_child_by_type(
    endpoint_node,
    ATTRIBUTE(HOST_INTERFACE_BUILD_NUMBER),
    REPORTED_ATTRIBUTE,
    host_interface_built_version);

  // Protocol version is under the version report.
  uint32_t zwave_protocol_version
    = (frame_data[15] << 16) | (frame_data[16] << 8) | (frame_data[17]);
  attribute_store_set_child_reported(version_report_node,
                                     ATTRIBUTE(ZWAVE_PROTOCOL_VERSION),
                                     &zwave_protocol_version,
                                     sizeof(zwave_protocol_version));

  protocol_built_version = (frame_data[18] << 8) | (frame_data[19]);
  attribute_store_set_uint32_child_by_type(
    endpoint_node,
    ATTRIBUTE(ZWAVE_PROTOCOL_BUILD_NUMBER),
    REPORTED_ATTRIBUTE,
    protocol_built_version);

  // Application version is actually Firmware 0 version, but with 3 digits.
  // Overwrite this value from the Version Report as it is more precise.
  attribute_store_node_t firmware_node
    = get_or_create_firmware_id_node(version_report_node, 0);
  uint32_t firmware_version
    = frame_data[20] << 16 | (frame_data[21] << 8) | (frame_data[22]);
  // 0 means unused, so not override the data from the Version Report
  if (firmware_version != 0) {
    attribute_store_set_child_reported(firmware_node,
                                       ATTRIBUTE(FIRMWARE_VERSION),
                                       &firmware_version,
                                       sizeof(firmware_version));
  }
  application_build_number = (frame_data[23] << 8) | (frame_data[24]);
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(APPLICATION_BUILD_NUMBER),
                                     &application_build_number,
                                     sizeof(application_build_number));

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_version_handle_report_command(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Find out the unid of the sending node:
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_node_t version_report_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION_REPORT_DATA),
                                             0);

  // Mark that we have received a Version Report
  const uint8_t report_received = 1;
  attribute_store_set_reported(version_report_node,
                               &report_received,
                               sizeof(report_received));

  if (frame_length > REPORT_LIBRARY_TYPE_INDEX) {
    uint8_t zwave_library = frame_data[REPORT_LIBRARY_TYPE_INDEX];
    attribute_store_set_child_reported(version_report_node,
                                       ATTRIBUTE(ZWAVE_LIBRARY_TYPE),
                                       &zwave_library,
                                       sizeof(zwave_library));
  }

  if (frame_length > REPORT_ZWAVE_PROTOCOL_SUB_VERSION_INDEX) {
    uint32_t zwave_protocol_version
      = (frame_data[REPORT_ZWAVE_PROTOCOL_VERSION_INDEX] << 16)
        | (frame_data[REPORT_ZWAVE_PROTOCOL_SUB_VERSION_INDEX] << 8);
    attribute_store_set_child_reported(version_report_node,
                                       ATTRIBUTE(ZWAVE_PROTOCOL_VERSION),
                                       &zwave_protocol_version,
                                       sizeof(zwave_protocol_version));
  }

  if (frame_length > REPORT_FIRMWARE_0_SUB_VERSION_INDEX) {
    attribute_store_node_t firmware_node
      = get_or_create_firmware_id_node(version_report_node, 0);

    uint32_t firmware_version
      = (frame_data[REPORT_FIRMWARE_0_VERSION_INDEX] << 16)
        | (frame_data[REPORT_FIRMWARE_0_SUB_VERSION_INDEX] << 8);
    attribute_store_set_child_reported(firmware_node,
                                       ATTRIBUTE(FIRMWARE_VERSION),
                                       &firmware_version,
                                       sizeof(firmware_version));
  }
  zwave_version_hardware_version_t hardware_version
    = DEFAULT_HARDWARE_VERSION;  // If its Version CC version 1 then there wont
                                 // be any hardware version so assume 1

  if (frame_length > REPORT_HARDWARE_VERSION_INDEX) {
    hardware_version = frame_data[REPORT_HARDWARE_VERSION_INDEX];
  }
  // Note: even if the version frame is empty, the ATTRIBUTE_CC_VERSION_HARDWARE_VERSION
  // attribute will be set to 1
  attribute_store_set_child_reported(version_report_node,
                                     ATTRIBUTE(HARDWARE_VERSION),
                                     &hardware_version,
                                     sizeof(hardware_version));

  uint8_t number_of_targets = 0;
  if (frame_length > REPORT_NUMBER_OF_TARGETS_INDEX) {
    number_of_targets = frame_data[REPORT_NUMBER_OF_TARGETS_INDEX];
  }

  for (uint8_t i = 1; i <= number_of_targets; i++) {
    uint8_t version_index = REPORT_FIRST_ADDITIONAL_TARGET_INDEX + (i - 1) * 2;
    uint8_t sub_version_index
      = REPORT_FIRST_ADDITIONAL_TARGET_INDEX + (i - 1) * 2 + 1;

    if (frame_length < sub_version_index) {
      break;
    }

    attribute_store_node_t firmware_node
      = get_or_create_firmware_id_node(version_report_node, i);
    uint32_t firmware_version = (frame_data[version_index] << 16)
                                | (frame_data[sub_version_index] << 8);
    attribute_store_set_child_reported(firmware_node,
                                       ATTRIBUTE(FIRMWARE_VERSION),
                                       &firmware_version,
                                       sizeof(firmware_version));
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_classes_version_handler_version_get(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 2 || frame_data[1] != VERSION_GET_V3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  zwapi_protocol_version_information_t zw_library_version = {0};
  zwapi_get_protocol_version(&zw_library_version);
  uint8_t major_firmware_0_version = 0;
  uint8_t minor_firmware_0_version = 0;
  zwapi_get_app_version(&major_firmware_0_version, &minor_firmware_0_version);

  const ZW_VERSION_REPORT_1BYTE_V3_FRAME report
    = {.cmdClass                         = COMMAND_CLASS_VERSION_V3,
       .cmd                              = VERSION_REPORT_V3,
       .zWaveLibraryType                 = zwapi_get_library_type(),
       .zWaveProtocolVersion             = zw_library_version.major_version,
       .zWaveProtocolSubVersion          = zw_library_version.minor_version,
       .firmware0Version                 = major_firmware_0_version,
       .firmware0SubVersion              = minor_firmware_0_version,
       .hardwareVersion                  = zpc_get_config()->hardware_version,
       .numberOfFirmwareTargets          = NUM_FW_TARGETS,
       .variantgroup1.firmwareVersion    = UIC_VERSION_MAJOR,
       .variantgroup1.firmwareSubVersion = UIC_VERSION_MINOR};

  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (const uint8_t *)&report);
}

static sl_status_t zwave_command_classes_version_handler_version_cc_get(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 3 || frame_data[1] != VERSION_COMMAND_CLASS_GET_V3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  zwave_cc_version_t version_of_cc
    = zwave_controller_transport_is_encapsulation_cc(frame_data[2]);
  if (version_of_cc == 0) {
    version_of_cc = zwave_command_handler_get_version(frame_data[2]);
  }

  const ZW_VERSION_COMMAND_CLASS_REPORT_V3_FRAME report
    = {.cmdClass              = COMMAND_CLASS_VERSION_V3,
       .cmd                   = VERSION_COMMAND_CLASS_REPORT_V3,
       .requestedCommandClass = frame_data[2],
       .commandClassVersion   = version_of_cc};

  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (const uint8_t *)&report);
}

static sl_status_t zwave_command_classes_version_handler_capabilities_get(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 2 || frame_data[1] != VERSION_CAPABILITIES_GET_V3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  const ZW_VERSION_CAPABILITIES_REPORT_V3_FRAME report = {
    .cmdClass = COMMAND_CLASS_VERSION_V3,
    .cmd      = VERSION_CAPABILITIES_REPORT_V3,
    .properties1
    = 0x07  // Notify that the ZPC supports the Version Z-Wave Software Get Command.
  };

  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (const uint8_t *)&report);
}

static sl_status_t zwave_command_classes_version_handler_version_software_get(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 2 || frame_data[1] != VERSION_ZWAVE_SOFTWARE_GET_V3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  uint8_t host_interface_version_major = 0;
  uint8_t host_interface_version_minor = 0;
  zwapi_get_app_version(&host_interface_version_major,
                        &host_interface_version_minor);
  // Z-Wave Protocol versions
  zwapi_protocol_version_information_t zw_protocol_version = {0};
  zwapi_get_protocol_version(&zw_protocol_version);

  const ZW_VERSION_ZWAVE_SOFTWARE_REPORT_V3_FRAME report = {
    .cmdClass    = COMMAND_CLASS_VERSION_V3,
    .cmd         = VERSION_ZWAVE_SOFTWARE_REPORT_V3,
    .sdkVersion1 = UIC_VERSION_MAJOR,
    .sdkVersion2 = UIC_VERSION_MINOR,
    .sdkVersion3 = UIC_VERSION_REV,
    // Application Framework fields do not apply for ZPC, therefore, we set them to 0.
    .applicationFrameworkApiVersion1  = 0,
    .applicationFrameworkApiVersion2  = 0,
    .applicationFrameworkApiVersion3  = 0,
    .applicationFrameworkBuildNumber1 = 0,
    .applicationFrameworkBuildNumber2 = 0,
    .hostInterfaceVersion1            = host_interface_version_major,
    .hostInterfaceVersion2            = host_interface_version_minor,
    .hostInterfaceVersion3            = 0,
    // Host Interface BuildNumber does not provided by Z-Wave API, therefore, we set it to 0.
    .hostInterfaceBuildNumber1 = 0,
    .hostInterfaceBuildNumber2 = 0,
    .zWaveProtocolVersion1     = zw_protocol_version.major_version,
    .zWaveProtocolVersion2     = zw_protocol_version.minor_version,
    .zWaveProtocolVersion3     = zw_protocol_version.revision_version,
    .zWaveProtocolBuildNumber1 = zw_protocol_version.build_number >> 8,
    .zWaveProtocolBuildNumber2 = zw_protocol_version.build_number & 0xFF,
    // Since ZPC application run on host CPU, Application Version fields do not apply for ZPC.
    .applicationVersion1     = 0,
    .applicationVersion2     = 0,
    .applicationVersion3     = 0,
    .applicationBuildNumber1 = 0,
    .applicationBuildNumber2 = 0};

  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (const uint8_t *)&report);
}

static sl_status_t zwave_command_class_version_support_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_VERSION_V3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case VERSION_GET_V3:
      return zwave_command_classes_version_handler_version_get(connection,
                                                               frame_data,
                                                               frame_length);

    case VERSION_COMMAND_CLASS_GET_V3:
      return zwave_command_classes_version_handler_version_cc_get(connection,
                                                                  frame_data,
                                                                  frame_length);

    case VERSION_CAPABILITIES_GET_V3:
      return zwave_command_classes_version_handler_capabilities_get(
        connection,
        frame_data,
        frame_length);

    case VERSION_ZWAVE_SOFTWARE_GET_V3:
      return zwave_command_classes_version_handler_version_software_get(
        connection,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

static sl_status_t zwave_command_class_version_control_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_VERSION_V3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case VERSION_COMMAND_CLASS_REPORT_V3:
      return zwave_command_class_version_handle_command_class_report_command(
        connection,
        frame_data,
        frame_length);

    case VERSION_REPORT_V3:
      return zwave_command_class_version_handle_report_command(connection,
                                                               frame_data,
                                                               frame_length);

    case VERSION_CAPABILITIES_REPORT_V3:
      return zwave_command_class_version_handle_capabilities_report_command(
        connection,
        frame_data,
        frame_length);

    case VERSION_ZWAVE_SOFTWARE_REPORT_V3:
      return zwave_command_class_version_handle_software_report_command(
        connection,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_version_command_class_version_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  // Here we do not want to interview the same data 10.000 times.
  // Verify if another endpoint in the same node has the Version CC information
  // saved somewhere. If yes, just copy it.
  if (true == is_command_class_version_already_known(node)) {
    *frame_len = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Find out which CC we need to find the version of.
  attribute_store_type_t version_cc_version_attribute
    = attribute_store_get_node_type(node);
  zwave_command_class_t requested_command_class
    = (version_cc_version_attribute >> 8) & 0xFFFF;

  // We cannot ask for the version of extended CCs.
  if (requested_command_class >= EXTENDED_COMMAND_CLASS_IDENTIFIER_START) {
    zwave_cc_version_t default_version = 1;
    attribute_store_set_reported(node,
                                 &default_version,
                                 sizeof(default_version));
    *frame_len = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  ZW_VERSION_COMMAND_CLASS_GET_FRAME *version_cc_get_frame
    = (ZW_VERSION_COMMAND_CLASS_GET_FRAME *)frame;
  version_cc_get_frame->cmdClass = COMMAND_CLASS_VERSION_V3;
  version_cc_get_frame->cmd      = VERSION_COMMAND_CLASS_GET_V3;
  version_cc_get_frame->requestedCommandClass
    = (uint8_t)requested_command_class;
  *frame_len = sizeof(ZW_VERSION_COMMAND_CLASS_GET_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_version_get(attribute_store_node_t node,
                                                   uint8_t *frame,
                                                   uint16_t *frame_len)
{
  if (ATTRIBUTE(VERSION_REPORT_DATA) != attribute_store_get_node_type(node)) {
    assert(0);
  }

  ZW_VERSION_GET_V3_FRAME *version_get_frame = (ZW_VERSION_GET_V3_FRAME *)frame;
  version_get_frame->cmdClass                = COMMAND_CLASS_VERSION_V3;
  version_get_frame->cmd                     = VERSION_GET_V3;
  *frame_len                                 = sizeof(ZW_VERSION_GET_V3_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_version_capabilities_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  assert(ATTRIBUTE(ZWAVE_SOFTWARE_GET_SUPPORT)
         == attribute_store_get_node_type(node));

  ZW_VERSION_CAPABILITIES_GET_V3_FRAME *version_capabilities_get_frame
    = (ZW_VERSION_CAPABILITIES_GET_V3_FRAME *)frame;
  version_capabilities_get_frame->cmdClass = COMMAND_CLASS_VERSION_V3;
  version_capabilities_get_frame->cmd      = VERSION_CAPABILITIES_GET_V3;
  *frame_len = sizeof(ZW_VERSION_CAPABILITIES_GET_V3_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_version_software_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  assert(ATTRIBUTE(APPLICATION_FRAMEWORK_API_VERSION)
         == attribute_store_get_node_type(node));

  ZW_VERSION_ZWAVE_SOFTWARE_GET_V3_FRAME *zwave_software_get_frame
    = (ZW_VERSION_ZWAVE_SOFTWARE_GET_V3_FRAME *)frame;
  zwave_software_get_frame->cmdClass = COMMAND_CLASS_VERSION_V3;
  zwave_software_get_frame->cmd      = VERSION_ZWAVE_SOFTWARE_GET_V3;
  *frame_len = sizeof(ZW_VERSION_ZWAVE_SOFTWARE_GET_V3_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute update callbacks
///////////////////////////////////////////////////////////////////////////////

static void on_version_command_class_found_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_CREATED) {
    return;
  }

  assert(ATTRIBUTE(FOUND) == attribute_store_get_node_type(updated_node));

  // We just detected for the first time the version CC as supported.
  // Go around and unresolve all CC version attribute that were set to
  // 1.
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_NODE_ID);
  uint32_t endpoint_index = 0;
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             endpoint_index);
  endpoint_index++;

  while (endpoint_node != ATTRIBUTE_STORE_INVALID_NODE) {
    for (uint8_t i = 0; i < attribute_store_get_node_child_count(endpoint_node);
         i++) {
      attribute_store_node_t current_child
        = attribute_store_get_node_child(endpoint_node, i);

      if (IS_ATTRIBUTE_TYPE_VERSION_CC_ATTRIBUTE(
            attribute_store_get_node_type(current_child))) {
        attribute_store_undefine_reported(current_child);
      }
    }

    // Go for the next endpoint
    endpoint_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               endpoint_index);
    endpoint_index++;
  }
}

static void on_nif_attribute_update(attribute_store_node_t updated_node,
                                    attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED || change == ATTRIBUTE_CREATED) {
    return;
  }

  if ((ATTRIBUTE_ZWAVE_NIF != attribute_store_get_node_type(updated_node))
      && (ATTRIBUTE_ZWAVE_SECURE_NIF
          != attribute_store_get_node_type(updated_node))) {
    assert(0);
  }

  // Find the Z-Wave NodeID and endpoint.
  zwave_node_id_t node_id         = 0;
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_zwave_ids_from_node(updated_node,
                                                                &node_id,
                                                                &endpoint_id)) {
    return;
  }

  // Verify if the node supports the version Command Class
  if (zwave_node_supports_command_class(COMMAND_CLASS_VERSION,
                                        node_id,
                                        endpoint_id)) {
    // If so, ensure we marked it as "found" under the NodeID
    attribute_store_node_t node_id_node
      = attribute_store_get_first_parent_with_type(updated_node,
                                                   ATTRIBUTE_NODE_ID);

    const attribute_store_type_t attribute[] = {ATTRIBUTE(FOUND)};
    attribute_store_add_if_missing(node_id_node,
                                   attribute,
                                   COUNT_OF(attribute));
  }

  zwave_command_class_version_create_cc_version_attributes(node_id,
                                                           endpoint_id);
}

static void on_zwave_cc_version_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }
  // Confirm that ATTRIBUTE_COMMAND_CLASS_VERSION_VERSION attribute is created
  assert(ATTRIBUTE_COMMAND_CLASS_VERSION_VERSION
         == attribute_store_get_node_type(updated_node));
  zwave_cc_version_t supported_version = 0;
  if (SL_STATUS_OK
      != attribute_store_read_value(updated_node,
                                    REPORTED_ATTRIBUTE,
                                    &supported_version,
                                    sizeof(supported_version))) {
    return;
  };

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  attribute_store_add_if_missing(endpoint_node,
                                 version_report_attribute,
                                 COUNT_OF(version_report_attribute));

  if (supported_version >= 3) {
    const attribute_store_type_t attributes[]
      = {ATTRIBUTE(ZWAVE_SOFTWARE_GET_SUPPORT)};
    attribute_store_add_if_missing(endpoint_node,
                                   attributes,
                                   COUNT_OF(attributes));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
bool is_version_cc_found(attribute_store_node_t node)
{
  const attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_NODE_ID);

  const attribute_store_node_t version_cc_found
    = attribute_store_get_node_child_by_type(node_id_node, ATTRIBUTE(FOUND), 0);

  return (version_cc_found != ATTRIBUTE_STORE_INVALID_NODE);
}

sl_status_t zwave_command_class_version_init()
{
  attribute_store_register_callback_by_type(
    on_version_command_class_found_attribute_update,
    ATTRIBUTE(FOUND));

  attribute_store_register_callback_by_type_and_state(on_nif_attribute_update,
                                                      ATTRIBUTE_ZWAVE_NIF,
                                                      REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    on_nif_attribute_update,
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type(
    on_zwave_cc_version_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_VERSION_VERSION);

  attribute_resolver_register_rule(ATTRIBUTE(VERSION_REPORT_DATA),
                                   NULL,
                                   zwave_command_class_version_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(ZWAVE_SOFTWARE_GET_SUPPORT),
    NULL,
    zwave_command_class_version_capabilities_get);

  attribute_resolver_register_rule(ATTRIBUTE(APPLICATION_FRAMEWORK_API_VERSION),
                                   NULL,
                                   zwave_command_class_version_software_get);

  attribute_resolver_register_rule(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BASIC),
    NULL,
    zwave_command_class_version_command_class_version_get);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler = &zwave_command_class_version_support_handler;
  handler.control_handler = &zwave_command_class_version_control_handler;
  handler.minimal_scheme  = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_VERSION_V3;
  handler.version                    = VERSION_VERSION_V3;
  handler.command_class_name         = "Version";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
