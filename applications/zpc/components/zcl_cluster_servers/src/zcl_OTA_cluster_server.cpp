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
// Includes from this component
#include "zcl_OTA_cluster_server.hpp"

// UIC Includes
#include "ota.hpp"
#include "attribute.hpp"
#include "attribute_resolver.h"
#include "sl_log.h"

// ZPC Includes
#include "attribute_store_defined_attribute_types.h"
#include "zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_attribute_id.h"
#include "zwave_command_class_firmware_update.h"
#include "zwave_unid.h"
#include "zpc_config.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_command_class_version_types.h"
#include "zpc_attribute_store.h"
#include "zwave_unid.h"

// Generic includes
#include <sstream>
#include <iomanip>
#include <map>

using namespace attribute_store;

// Constants
constexpr const char *TAG               = "zcl_OTA_cluster_server";
constexpr int OTA_GET_TIMEOUT_SECONDS   = 10;
constexpr uint16_t OTA_CLUSTER_REVISION = 1;

/**
 * @brief Extract the target ID from the UIID string
          I.e. get the 00 out of "ZWave-0000-0002-0004-00-01"
 *
 * @param uiid
 * @return int
 */
static int get_target_from_uiid(const std::string &uiid)
{
  size_t last_dash = 0, last_2_dash = 0;
  for (size_t idx = 0; idx < uiid.size(); idx++) {
    if (uiid[idx] == '-') {
      last_2_dash = last_dash;
      last_dash = idx;
    }
  }
  if ((last_2_dash != 0) && (last_dash != 0)) {
    std::string target_str = uiid.substr(last_2_dash + 1, last_dash - last_2_dash - 1);
    return std::stoi(target_str);
  } else {
    sl_log_error(TAG, "Unable to resolve target from uiid");
    return -1;
  }
}

/**
 * @brief Return the version string form a given node and firmware target.
 *
 * @param node Z-Wave node attribute
 * @param fw_target_id
 * @return std::string version string, returns and emptry string if version info could not be retrieved.
 */
static std::string get_target_version(attribute node, int fw_target_id)
{
  try {
    attribute ep
      = node.child_by_type_and_value<zwave_endpoint_id_t>(ATTRIBUTE_ENDPOINT_ID,
                                                          0);
    attribute version_data
      = ep.child_by_type(ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA);
    attribute fw_target = version_data.child_by_type(
      ATTRIBUTE_CC_VERSION_FIRMWARE);
    int32_t fw_version
      = fw_target.child_by_type(ATTRIBUTE_CC_VERSION_FIRMWARE_VERSION)
          .reported<int32_t>();
    std::stringstream ss;
    ss << ((fw_version >> 16) & 0xff) << "." << ((fw_version >> 8) & 0xff)
       << "." << ((fw_version >> 0) & 0xff);
    return ss.str();

  } catch (std::invalid_argument &e) {
    sl_log_error(TAG, "Fail to get target version due to %s", e.what());
    return "";
  }
}

/**
 * @brief Get the target info object
 *
 * @param fw_target Value of this attribute must be a firmware target
 * @param unid      output unid
 * @param endpoint  output endpoint
 * @param uiid      output UIID
 * @return sl_status_t
 */
static sl_status_t get_target_info(attribute fw_target,
                                   unid_t &unid,
                                   zwave_endpoint_id_t &endpoint,
                                   std::string &uiid)
{
  try {
    //Get the attribute we need
    attribute ep_node = fw_target.first_parent(ATTRIBUTE_ENDPOINT_ID);
    attribute zw_node = ep_node.parent();
    attribute ep0_node
      = zw_node.child_by_type_and_value<uint8_t>(ATTRIBUTE_ENDPOINT_ID, 0);
    attribute manufacturer_id
      = ep0_node.child_by_type(ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID);
    attribute product_id
      = ep0_node.child_by_type(ATTRIBUTE_MANUFACTURER_SPECIFIC_PRODUCT_TYPE_ID);
    attribute product_type_id
      = ep0_node.child_by_type(ATTRIBUTE_MANUFACTURER_SPECIFIC_PRODUCT_ID);
    attribute version_report_node
      = ep0_node.child_by_type(ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA);
    attribute hardware_version = version_report_node.child_by_type(
      ATTRIBUTE_CC_VERSION_HARDWARE_VERSION);

    if (ep_node.is_valid() && zw_node.is_valid() && ep0_node.is_valid()
        && manufacturer_id.is_valid() && product_id.is_valid()
        && product_type_id.is_valid() && hardware_version.is_valid()) {
      // Generate the UUID
      std::stringstream ss;
      ss << "ZWave" << std::hex << std::setfill('0');
      ss << "-" << std::setw(4) << manufacturer_id.reported<int32_t>();
      ss << "-" << std::setw(4) << product_type_id.reported<int32_t>();
      ss << "-" << std::setw(4) << product_id.reported<int32_t>();
      ss << "-" << std::setw(2) << fw_target.reported<int32_t>();
      ss << "-" << std::setw(2)
         << hardware_version.reported<zwave_version_hardware_version_t>();
      uiid = ss.str();

      // Get the nodeid and the endpoint
      zwave_unid_from_node_id(zw_node.reported<zwave_node_id_t>(), unid);
      endpoint = ep_node.reported<uint8_t>();

      return SL_STATUS_OK;
    }
  } catch (const std::invalid_argument &) {
    //Just ignore
  }

  sl_log_debug(TAG,
               "Unable to locate all attributes required to publish OTA info.");
  return SL_STATUS_FAIL;
}

/**
 * @brief Helper to read from attribute store and pubish
 *
 * @tparam T
 * @tparam F
 * @param func
 * @param node
 * @param change
 */
template<typename T, typename F>
void generic_update(F func, attribute node, attribute_store_change_t change)
{
  unid_t unid;
  uint8_t endpoint;
  std::string uiid;

  attribute fw_target = node.parent().child_by_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_TARGET);

  if (fw_target.is_valid()
      && (SL_STATUS_OK == get_target_info(fw_target, unid, endpoint, uiid))) {
    try {
      func(unid, endpoint, uiid.c_str(), node.reported<T>());
    } catch (const std::invalid_argument &e) {
      //FIXME: These messages are not so helpful and occur often
      sl_log_debug(TAG, "%s", e.what());
    }
  }
}

static void ota_state_update(attribute_store_node_t __node,
                             attribute_store_change_t change)
{
  unid_t unid;
  uint8_t endpoint;
  std::string uiid;

  attribute node = __node;
  attribute fw_target
    = node.child_by_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_TARGET);

  if (fw_target.is_valid()
      && (SL_STATUS_OK == get_target_info(fw_target, unid, endpoint, uiid))) {
    try {
      uic_ota::update_status(unid,
                             endpoint,
                             uiid.c_str(),
                             node.reported<uic_ota::status_t>());
    } catch (const std::invalid_argument &e) {
      //FIXME: These messages are not so helpful and occur often
      sl_log_debug(TAG, "%s", e.what());
    }
  }
}

static void ota_status_update(attribute_store_node_t node,
                              attribute_store_change_t change)
{
  generic_update<uic_ota::last_error_t>(uic_ota::update_last_error,
                                        node,
                                        change);
}

static void ota_size_update(attribute_store_node_t node,
                            attribute_store_change_t change)
{
  generic_update<uint32_t>(uic_ota::update_size, node, change);
}

static void ota_offset_update(attribute_store_node_t node,
                              attribute_store_change_t change)
{
  generic_update<uint32_t>(uic_ota::update_offset, node, change);
}

static void ota_apply_after_update(attribute_store_node_t node,
                                   attribute_store_change_t change)
{
  generic_update<time_t>(uic_ota::update_apply_after, node, change);
}

/**
 * @brief Monitor the creation for firmware targets, if we have a target
 * we can generate the UIID, hence we can register ourselves with the OTA mqtt client.
 *
 */
static void firmware_id_update(attribute_store_node_t node,
                               attribute_store_change_t change)
{
  unid_t unid;
  zwave_endpoint_id_t endpoint;
  std::string uiid;

  if (SL_STATUS_OK == get_target_info(node, unid, endpoint, uiid)) {
    if (change == ATTRIBUTE_UPDATED) {
      sl_log_debug(TAG,
                   "Subscribing to available firmware images for UIID %s",
                   uiid.c_str());
      uic_ota::subscribe_unid(unid, uiid);
    } else if (change == ATTRIBUTE_DELETED) {
      sl_log_debug(TAG,
                   "Stop listening to available firmware images for UIID %s",
                   uiid.c_str());
      uic_ota::unsubscribe_unid(unid, uiid);
    }
  }
}

static void firmware_version_update(attribute_store_node_t _fw_version_node,
                                    attribute_store_change_t change)
{
  unid_t unid;
  uint8_t endpoint;
  std::string uiid;

  attribute fw_version_node = _fw_version_node;

  //FIXME: So this only works if the version info is updated after the manufacturer specific is probed,
  // Should we really use the mapper to check this? like
  // r'UIID_READY = e'^.MANUFACTURER_ID & e'^.PRODUCT_TYPE_ID & e'^.PRODUCT_ID & e'FIRMWARE_VERSION
  if ((change == ATTRIBUTE_UPDATED)
      && (SL_STATUS_OK
          == get_target_info(fw_version_node.parent(), unid, endpoint, uiid))) {
    uint32_t fw_version = fw_version_node.reported<uint32_t>();

    std::stringstream ss;
    ss << ((fw_version >> 16) & 0xff) << "." << ((fw_version >> 8) & 0xff)
       << "." << ((fw_version >> 0) & 0xff);

    uic_ota::update_current_version(unid, endpoint, uiid.c_str(), ss.str());
  }
}

static void start_update(const std::string &uiid,
                         const std::string &unid,
                         const std::string &target_version,
                         unsigned long apply_after)
{
  int firmware_target = get_target_from_uiid(uiid);

  if (firmware_target < 0) {
    return;
  }

  attribute node
    = attribute_store_network_helper_get_node_id_node(unid.c_str());
  std::string current_version     = get_target_version(node, firmware_target);
  zwave_endpoint_id_t endpoint_id = 0;
  if (current_version != target_version) {
    std::stringstream ss;
    ss << "New Firmware image is available. ";
    ss << "UNID: " << unid;
    ss << " UIID: " << uiid;
    ss << " Target Version: " << target_version;
    ss << " Current version: " << current_version;
    sl_log_info(TAG, "%s", ss.str().c_str());

    uic_ota::update_target_version(unid.c_str(),
                                   endpoint_id,
                                   uiid.c_str(),
                                   target_version);

    //FIXME: after the OTA api has been updated a lambda expression is no longer needed here
    uic_ota::get_by_unid(
      unid,
      uiid,
      [firmware_target, unid, apply_after, endpoint_id](
        const uic_ota::image_ready_result_t result,
        const std::string &path) {
        zwave_node_id_t node_id = 0;

        zwave_unid_to_node_id(unid.c_str(), &node_id);

        if ((result != uic_ota::image_ready_result_t::OK)
            || (SL_STATUS_OK
                != command_class_firmware_update_initiate_firmware_update(
                  node_id,
                  endpoint_id,
                  firmware_target,
                  apply_after,
                  path.c_str()))) {
          sl_log_info(TAG,
                      "Failed to start firmware update for UNID %s",
                      unid.c_str());
        }
      });
  }
}

/**
 * @brief Attempts to publish all the OTA information after an endpoint has been
 *        interviewed.
 *
 * @param nif_node  The attribute store node of the NIF
 * @param change    The attribute store change applied to the NIF
 */
static void
  on_endpoint_interview_completed(attribute_store_node_t endpoint_node)
{
  // Do not get called again for this endpoint:
  attribute_resolver_clear_resolution_listener(
    endpoint_node,
    &on_endpoint_interview_completed);

  try {
    // Pretend that all the firmware data has just been updated.
    attribute ep(endpoint_node);
    attribute fwu = ep.child_by_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU);

    // Firmware attributes (1..N targets)
    attribute fwu_fw = fwu.child_by_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW);
    size_t index     = 0;
    while (fwu_fw.is_valid()) {
      firmware_id_update(fwu_fw, ATTRIBUTE_UPDATED);
      fwu_fw = fwu.child_by_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW, index);
      index += 1;
    }

    // Firmware Transfer
    attribute fw_transfer
      = fwu.child_by_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER);
    if (fw_transfer.is_valid()) {
      ota_state_update(fw_transfer, ATTRIBUTE_UPDATED);
    }

    // Firmware Transfer last status
    attribute fw_transfer_last_status = fw_transfer.child_by_type(
      ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_LAST_STATUS);
    if (fw_transfer_last_status.is_valid()) {
      ota_status_update(fw_transfer_last_status, ATTRIBUTE_UPDATED);
    }

    // Firmware Transfer Offset
    attribute fw_transfer_offset = fw_transfer.child_by_type(
      ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_OFFSET);
    if (fw_transfer_offset.is_valid()) {
      ota_offset_update(fw_transfer_offset, ATTRIBUTE_UPDATED);
    }

    // Firmware Transfer Size
    attribute fw_transfer_size = fw_transfer.child_by_type(
      ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_SIZE);
    if (fw_transfer_size.is_valid()) {
      ota_size_update(fw_transfer_size, ATTRIBUTE_UPDATED);
    }

    // Firmware Transfer Apply after
    attribute fw_transfer_apply_after = fw_transfer.child_by_type(
      ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_APPLY_TIMESTAMP);
    if (fw_transfer_apply_after.is_valid()) {
      ota_apply_after_update(fw_transfer_apply_after, ATTRIBUTE_UPDATED);
    }

    // Version updates for firmwares
    attribute version_report_data
      = ep.child_by_type(ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA);
    attribute firmware_target(ATTRIBUTE_STORE_INVALID_NODE);
    attribute firmware_target_version(ATTRIBUTE_STORE_INVALID_NODE);
    index = 0;
    do {
      firmware_target
        = version_report_data.child_by_type(ATTRIBUTE_CC_VERSION_FIRMWARE,
                                            index);
      firmware_target_version
        = firmware_target.child_by_type(ATTRIBUTE_CC_VERSION_FIRMWARE_VERSION);

      if (firmware_target_version.is_valid()) {
        firmware_version_update(firmware_target_version, ATTRIBUTE_UPDATED);
      }
      index += 1;
    } while (firmware_target_version.is_valid());

  } catch (const std::exception &e) {
    sl_log_debug(TAG,
                 "Could not gather necessary data at the end of the interview "
                 "for Attribute Store endpoint node %d",
                 endpoint_node);
  }
}

/**
 * @brief Listens for NIF changes to detect what to publish for
 *        the OTA cluster.
 *
 * On NIF created, we just wait until the end of the interview.
 * We make sure that previous publications are gone.
 *
 * @param nif_node  The attribute store node of the NIF
 * @param change    The attribute store change applied to the NIF
 */
static void on_nif_update(attribute_store_node_t nif_node,
                          attribute_store_change_t change)
{
  // Consider the node to be interviewing if
  // a. NIF is just created
  // b. NIF is updated and empty.
  bool node_under_interview = false;
  if (change == ATTRIBUTE_CREATED) {
    node_under_interview = true;
  } else if ((change == ATTRIBUTE_UPDATED)
             && (attribute_store_is_value_defined(nif_node, REPORTED_ATTRIBUTE)
                 == false)) {
    node_under_interview = true;
  }

  if (node_under_interview == true) {
    // Somebody is interviewing an endpoint. Set a listener on the Endpoint and
    // try to publish OTA data when the interview is over.
    attribute_store_node_t endpoint_node
      = attribute_store_get_first_parent_with_type(nif_node,
                                                   ATTRIBUTE_ENDPOINT_ID);
    attribute_resolver_set_resolution_listener(
      endpoint_node,
      &on_endpoint_interview_completed);
  } else if (change == ATTRIBUTE_DELETED) {
    // We are removing a node from the network or about to re-interview.
    // Delete the old OTA publications for that node.
    unid_t node_unid;
    if (SL_STATUS_OK
        != attribute_store_network_helper_get_unid_from_node(nif_node,
                                                             node_unid)) {
      return;
    }

    uic_ota::unsubscribe_all_unids_uiid(std::string(node_unid));
  }
}

static void image_available(const uic_ota::meta_t &meta)
{
  start_update(meta.uiid, meta.unid, meta.version, meta.apply_after);
}

sl_status_t zcl_OTA_cluster_server_init()
{
  attribute_store_register_callback_by_type(
    ota_state_update,
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER);

  attribute_store_register_callback_by_type(
    ota_status_update,
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_LAST_STATUS);

  attribute_store_register_callback_by_type(
    ota_offset_update,
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_OFFSET);
  attribute_store_register_callback_by_type(
    ota_size_update,
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_SIZE);

  attribute_store_register_callback_by_type(
    ota_apply_after_update,
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_APPLY_TIMESTAMP);

  attribute_store_register_callback_by_type(
    firmware_id_update,
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW);

  attribute_store_register_callback_by_type(
    firmware_version_update,
    ATTRIBUTE_CC_VERSION_FIRMWARE_VERSION);

  attribute_store_register_callback_by_type(
    firmware_version_update,
    ATTRIBUTE_CC_VERSION_HARDWARE_VERSION);

  attribute_store_register_callback_by_type(on_nif_update, ATTRIBUTE_ZWAVE_NIF);

  return uic_ota::init(image_available,
                       zpc_get_config()->ota_cache_path,
                       zpc_get_config()->ota_cache_size * 1024,
                       OTA_GET_TIMEOUT_SECONDS,
                       OTA_CLUSTER_REVISION);
}
