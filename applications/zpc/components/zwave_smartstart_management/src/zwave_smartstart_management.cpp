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
#include "smartstart_management.hpp"
#include "zwave_smartstart_management.h"
#include "zwave_network_management.h"
#include "zpc_converters.h"
#include "zpc_config.h"
#include "zwave_unid.h"
#include "zwave_utils.h"
#include "zwave_controller_callbacks.h"
#include "zwave_controller.h"
#include "s2_keystore.h"

// Contiki includes
#include "sys/ctimer.h"
#include "sys/clock.h"

// Generic includes
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>

// Unify includes
#include "sl_log.h"
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute.hpp"
#include "zpc_attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace smartstart;

constexpr char LOG_TAG[] = "zwave_smartstart_management";

// Representation of the different protocols in the PreferredProtocol array
// of the SmartStart list
///< Z-Wave Long Range
constexpr char ZWAVE_LONG_RANGE_STRING_REPRESENTATION[] = "Z-Wave Long Range";
///< Z-Wave
constexpr char ZWAVE_STRING_REPRESENTATION[] = "Z-Wave";

const std::map<std::string, zwave_protocol_t, std::less<>>
  preferred_protocol_map
  = {{std::string(ZWAVE_CONFIG_REPRESENTATION), PROTOCOL_ZWAVE},
     {std::string(ZWAVE_LONG_RANGE_CONFIG_REPRESENTATION),
      PROTOCOL_ZWAVE_LONG_RANGE}};

const int PROTOCOL_DISCOVERY_COOLDOWN = 4;  // seconds

/// Struct listing the protocol capabilities of a Node/DSK
using node_protocol_capabilities_t = struct {
  // Z-Wave is supported
  bool zwave;
  // Z-Wave Long Range is supported
  bool zwave_long_range;
};

/**
 * @brief List of possible states for the protocol discovery
 */
enum class protocol_discover_state_t {
  DISCOVERED_ZERO,
  DISCOVERED_ONE,
  DISCOVERED_ALL
};

class ProtocolDiscoveryEntry
{
  public:
  // Timer to implement cooldown on SmartStart Prime detection
  struct ctimer timer;
  // Struct to denote the protocol capabilities
  node_protocol_capabilities_t capabilities;

  void add_capability(zwave_protocol_t protocol)
  {
    switch (protocol) {
      case PROTOCOL_ZWAVE_LONG_RANGE:
        capabilities.zwave_long_range = true;
        break;

      case PROTOCOL_ZWAVE:
        capabilities.zwave = true;
        break;

      default:
        // Unknown protocol, do nothing
        break;
    }
  }

  bool has_capability(zwave_protocol_t protocol)
  {
    bool support = false;
    switch (protocol) {
      case PROTOCOL_ZWAVE_LONG_RANGE:
        support = capabilities.zwave_long_range;
        break;

      case PROTOCOL_ZWAVE:
        support = capabilities.zwave;
        break;

      default:
        support = false;
        break;
    }
    return support;
  }

  ~ProtocolDiscoveryEntry()
  {
    ctimer_stop(&timer);
  }
};

std::unordered_map<std::string, std::shared_ptr<ProtocolDiscoveryEntry>>
  protocol_discovery_db;

static void zwave_smartstart_management_on_node_deleted(zwave_node_id_t node_id)
{
  if (node_id == 0) {
    sl_log_debug(
      LOG_TAG,
      "No SmartStart list update for a node removed from a foreign network.");
    return;
  }

  unid_t unid;
  zwave_unid_from_node_id(node_id, unid);
  Management::get_instance()->notify_node_removed(std::string(unid));
}

/**
   * @brief Search for home_id in DSK (9th-12th byte)
   *
   * @param dsk DSK to search in
   * @param home_id HomeID to search for in Host endianess
   * @return true home_id found
   * @return false home_id not found
   */
static bool is_homeid_in_dsk(zwave_dsk_t dsk, zwave_home_id_t home_id)
{
  zwave_home_id_t be_home_id = htonl(home_id);
  uint8_t buf[4];
  memcpy(buf, &(dsk[8]), 4);
  buf[0] |= 0xC0;
  buf[3] &= 0xFE;
  if (memcmp(buf, &be_home_id, 4) == 0) {
    return true;
  }
  return false;
}

/**
 * @brief function that parses the ZPC config string into a vector of zwave_protocol_t
 *
 * @returns a vector of zwave_protocol_t based on the configuration
*/
static std::vector<zwave_protocol_t> get_preferred_protocol_list_from_config()
{
  std::vector<zwave_protocol_t> protocols;
  try {
    std::string protocols_string(
      zpc_get_config()->inclusion_protocol_preference);
    std::vector<std::string> protocol_elements;
    boost::algorithm::split(protocol_elements,
                            protocols_string,
                            boost::is_any_of(","));

    for (auto const &element: protocol_elements) {
      if (preferred_protocol_map.count(element)) {
        protocols.push_back(preferred_protocol_map.at(element));
      }
    }
  } catch (const std::exception &exc) {
    sl_log_error(
      LOG_TAG,
      "Cannot parse configuration preferred protocols. Exception: %s",
      exc.what());
  }
  return protocols;
}

/**
 * @brief function that helps deciding what is the preferred inclusion protocol from the config
 *
 * @param capabilities   node_protocol_capabilities_t indicating what protocols
 *                       are supported by the node.
 *
 * @returns zwave_protocol_t the protocol that we should include the node with.
*/
static zwave_protocol_t get_preferred_inclusion_protocol_from_config(
  node_protocol_capabilities_t capabilities)
{
  std::vector<zwave_protocol_t> protocol_list
    = get_preferred_protocol_list_from_config();

  for (auto protocol: protocol_list) {
    if (protocol == PROTOCOL_ZWAVE && capabilities.zwave) {
      return protocol;
    }
    if (protocol == PROTOCOL_ZWAVE_LONG_RANGE
        && capabilities.zwave_long_range) {
      return protocol;
    }
  }

  return PROTOCOL_UNKNOWN;
}

/**
 * @brief function that helps deciding what is the preferred inclusion
 * protocol from the SmartStart list
 *
 * @param preferred_protocols           Preferred protocols given from the
 *                                      SmartStart list match.
 * @param capabilities   node_protocol_capabilities_t indicating what protocols
 *                       are supported by the node.
 *
 * @returns zwave_protocol_t the protocol that we should include the node with.
*/
static zwave_protocol_t get_preferred_inclusion_protocol_from_list(
  std::vector<std::string> preferred_protocols,
  node_protocol_capabilities_t capabilities)
{
  for (uint8_t i = 0; i < preferred_protocols.size(); i++) {
    if (0
          == preferred_protocols[i].compare(
            ZWAVE_LONG_RANGE_STRING_REPRESENTATION)
        && (capabilities.zwave_long_range == true)) {
      return PROTOCOL_ZWAVE_LONG_RANGE;
    }
    if (0 == preferred_protocols[i].compare(ZWAVE_STRING_REPRESENTATION)
        && (capabilities.zwave == true)) {
      return PROTOCOL_ZWAVE;
    }
  }
  return PROTOCOL_UNKNOWN;
}

/**
 * @brief function that verifies if we have several candidates (from the config)
 * or SmartStart list to include a node. If yes, discovery on the node itself
 * has to be performed
 *
 * @param preferred_protocols           Preferred protocols given from the
 *                                      SmartStart list match.
 *
 * @returns true if there is more than 1 protocol to include the node.
 *          false if 0 or 1 protocol to include the node (i.e. no discovery needed)
*/
static bool
  has_several_protocol_candidates(std::vector<std::string> preferred_protocols)
{
  if (preferred_protocols.empty()) {
    return (get_preferred_protocol_list_from_config().size() > 1);
  } else {
    return (preferred_protocols.size() > 1);
  }
}

/**
 * @brief Returns the current state of protocol discovery for a given DSK
 *
 * @param dsk       The DSK to return the protocol discovery state for
 * @return protocol_discover_state_t
 */
static protocol_discover_state_t
  get_protocol_discovery_state(const std::string &dsk)
{
  if (protocol_discovery_db.find(dsk) == protocol_discovery_db.end()) {
    return protocol_discover_state_t::DISCOVERED_ZERO;
  }
  auto entry = protocol_discovery_db[dsk].get();
  if (entry->has_capability(PROTOCOL_ZWAVE_LONG_RANGE)
      && entry->has_capability(PROTOCOL_ZWAVE)) {
    return protocol_discover_state_t::DISCOVERED_ALL;
  } else if (entry->has_capability(PROTOCOL_ZWAVE_LONG_RANGE)
             || entry->has_capability(PROTOCOL_ZWAVE)) {
    return protocol_discover_state_t::DISCOVERED_ONE;
  }
  return protocol_discover_state_t::DISCOVERED_ZERO;
}

/**
 * @brief Verify the home ID of SmartStart joining node with provisioning list.
 *
 * This function should be called after receiving SmartStart inclusion request.
 * Depending on the result of query in the provisioning list(local cache) and
 * whether the node is already included, this function may initiate the
 * SmartStart inclusion or ignore the request.
 *
 * @param home_id This field is used to indicate the home ID of joining node.
 * @param already_included This field is used to indicate if the joining node is
 * already included.
 * @param node_info This field is used to indicate the node info structure.
 */
static void zwave_smartstart_management_on_inclusion_request(
  zwave_home_id_t home_id,
  bool already_included,
  const zwave_node_info_t *node_info,
  zwave_protocol_t protocol)
{
  const Entry *matched     = nullptr;
  zwave_dsk_t dsk_internal = {0};
  for (const auto &pair: Management::get_instance()->get_cache()) {
    if (SL_STATUS_OK
          == zpc_converters_dsk_str_to_internal(pair.second.dsk.c_str(),
                                                dsk_internal)
        && is_homeid_in_dsk(dsk_internal, home_id)) {
      matched = &pair.second;
      break;
    }
  }

  if (matched == nullptr) {
    sl_log_debug(LOG_TAG,
                 "No match in the SmartStart list for NWI HomeID %X",
                 home_id);
    return;
  }

  // If we received an INIF, push the info back to the SmartStart List.
  if (already_included == true) {
    sl_log_info(LOG_TAG, "Received an INIF from NWI HomeID %X. ", home_id);
    Management::get_instance()->set_manual_intervention_required(matched->dsk,
                                                                 true);
    return;
  } else {
    Management::get_instance()->set_manual_intervention_required(matched->dsk,
                                                                 false);
  }

  if (has_several_protocol_candidates(matched->preferred_protocols) == true) {
    protocol_discover_state_t state
      = get_protocol_discovery_state(matched->dsk);
    switch (state) {
      case protocol_discover_state_t::DISCOVERED_ZERO: {
        // First time we discover protocol for this node, let's start the timer and fill in the capability
        std::shared_ptr<ProtocolDiscoveryEntry> entry
          = std::make_shared<ProtocolDiscoveryEntry>();
        entry.get()->add_capability(protocol);
        ctimer_set(&entry.get()->timer,
                   PROTOCOL_DISCOVERY_COOLDOWN * CLOCK_SECOND,
                   NULL,
                   0);
        protocol_discovery_db[matched->dsk] = entry;
        sl_log_debug(LOG_TAG,
                     "Discovered %s capability for NWI HomeID %X",
                     zwave_get_protocol_name(protocol),
                     home_id);
        return;
      }

      case protocol_discover_state_t::DISCOVERED_ONE: {
        // Entry exists but found new capability, reset the timer and fill in the capability
        if (protocol_discovery_db[matched->dsk].get()->has_capability(protocol)
            == false) {
          ctimer_restart(&protocol_discovery_db[matched->dsk].get()->timer);
          protocol_discovery_db[matched->dsk].get()->add_capability(protocol);
          sl_log_debug(LOG_TAG,
                       "Discovered %s capability for NWI HomeID %X",
                       zwave_get_protocol_name(protocol),
                       home_id);
          // Early exit if we found all the capabilities.
          if (protocol_discover_state_t::DISCOVERED_ALL
              == get_protocol_discovery_state(matched->dsk)) {
            goto done_protocol_discovery;
          }
          return;
        } else {
          // Entry exists with the same discovered capability and timer is expired. We are done.
          if (ctimer_expired(
                &protocol_discovery_db[matched->dsk].get()->timer)) {
            goto done_protocol_discovery;
          }
          // Entry exists with the same discovered capability but timer is not expired. Ignore it.
          sl_log_debug(
            LOG_TAG,
            "Re-discovered %s capability for NWI HomeID %X too early. "
            "This may be from NWI repeaters. Ignoring it.",
            zwave_get_protocol_name(protocol),
            home_id);
          return;
        }
      }

      case protocol_discover_state_t::DISCOVERED_ALL: {
        goto done_protocol_discovery;
      }

      default: {
        // This should not happen.
        sl_log_warning(LOG_TAG, "Invalid protocol discovery state %d", state);
        return;
      }
    }

done_protocol_discovery:
    sl_log_debug(LOG_TAG,
                 "Protocol discovery for NWI Home ID %X is done.",
                 home_id);
  }

  // node capabilities make sense only when we have multiple candidates
  zwave_protocol_t preferred_protocol = PROTOCOL_UNKNOWN;
  if (has_several_protocol_candidates(matched->preferred_protocols) == true) {
    node_protocol_capabilities_t node_capabilities
      = protocol_discovery_db[matched->dsk].get()->capabilities;

    if (matched->preferred_protocols.empty()) {
      preferred_protocol
        = get_preferred_inclusion_protocol_from_config(node_capabilities);
    } else {
      preferred_protocol = get_preferred_inclusion_protocol_from_list(
        matched->preferred_protocols,
        node_capabilities);
    }
  } else {
    if (matched->preferred_protocols.empty()) {
      preferred_protocol = get_preferred_protocol_list_from_config().at(0);
    } else {
      if (0
          == matched->preferred_protocols.at(0).compare(
            ZWAVE_LONG_RANGE_STRING_REPRESENTATION)) {
        preferred_protocol = PROTOCOL_ZWAVE_LONG_RANGE;
      } else if (0
                 == matched->preferred_protocols.at(0).compare(
                   ZWAVE_STRING_REPRESENTATION)) {
        preferred_protocol = PROTOCOL_ZWAVE;
      }
    }
  }
  if (preferred_protocol == PROTOCOL_UNKNOWN) {
    sl_log_info(LOG_TAG,
                "Preferred Inclusion Protocol is not supported by the NWI "
                "HomeID %X. No inclusion will take place\n",
                home_id);
    return;
  }

  if (protocol != preferred_protocol) {
    sl_log_debug(
      LOG_TAG,
      "Received SmartStart Prime on a different protocol (%s) than the "
      "preferred protocol (%s) in the SmartStart list. Ignoring it.\n",
      zwave_get_protocol_name(protocol),
      zwave_get_protocol_name(preferred_protocol));
    return;
  }

  // We have a match, we are happy proceed with inclusion
  if (matched->include == true && matched->device_unid.empty()) {
    zwave_network_management_add_node_with_dsk(dsk_internal,
                                               KEY_CLASS_ALL,
                                               preferred_protocol);
  }
}

static void
  zwave_smartstart_management_on_node_added(sl_status_t status,
                                            const zwave_node_info_t *nif,
                                            zwave_node_id_t node_id,
                                            const zwave_dsk_t dsk,
                                            zwave_keyset_t granted_keys,
                                            zwave_kex_fail_type_t kex_fail_type,
                                            zwave_protocol_t inclusion_protocol)
{
  if (SL_STATUS_OK == status
      && (ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE == kex_fail_type)) {
    unid_t unid;
    char dsk_str[100];
    zpc_converters_dsk_to_str(dsk, dsk_str, sizeof(dsk_str));
    zwave_unid_from_node_id(node_id, unid);
    Management::get_instance()->notify_node_added(std::string(dsk_str),
                                                  std::string(unid));

    // Also indicate that this node does not need any manual intervention
    // to join the network (in case it did)
    Management::get_instance()->set_manual_intervention_required(
      std::string(dsk_str),
      false);
  }
}

static zwave_controller_callbacks_t smartstart_callbacks = {
  .on_node_deleted = zwave_smartstart_management_on_node_deleted,
  .on_node_added   = zwave_smartstart_management_on_node_added,
  .on_smart_start_inclusion_request
  = zwave_smartstart_management_on_inclusion_request,
};

/**
 * @brief Verify if a given DSK is already in our network.
 *
 * @param dsk_internal the DSK to look up in our network.
 *
 * @returns true if a node with the same DSK is included in our current network
 * false if no match were found
 */
static bool
  is_dsk_already_included_in_current_network(const zwave_dsk_t &dsk_internal)
{
  using namespace attribute_store;
  attribute home_id_node = get_zpc_network_node();
  attribute dsk_node;
  // Iterate over NODE_ID, find DSK
  for (auto node_id_node: home_id_node.children(ATTRIBUTE_NODE_ID)) {
    dsk_node = node_id_node.child_by_type(ATTRIBUTE_S2_DSK, 0);
    if (!dsk_node.is_valid()) {
      // No DSK information, no luck.
      continue;
    }
    std::vector<uint8_t> dsk_data;
    try {
      dsk_data = dsk_node.reported<std::vector<uint8_t>>();
    } catch (const std::invalid_argument &e) {
      sl_log_warning(LOG_TAG,
                     "Failed to read the DSK reported"
                     "value for attribute %d: %s",
                     e.what(),
                     dsk_node);
      continue;
    }
    // Compare DSK. If DSK already in network, get UNID
    if (0 == memcmp(&dsk_data[0], &dsk_internal[0], sizeof(dsk_internal))) {
      unid_t n_unid;
      zwave_node_id_t node_id_data = node_id_node.reported<uint16_t>();

      zwave_unid_from_node_id(node_id_data, n_unid);

      char dsk_str[100];
      zpc_converters_dsk_to_str(dsk_internal, dsk_str, sizeof(dsk_str));

      sl_log_info(LOG_TAG,
                  "DSK %s added in the provisioning list is already "
                  "in the network (UNID: %s). No inclusion will take place.",
                  dsk_str,
                  n_unid);
      Management::get_instance()->notify_node_added(std::string(dsk_str),
                                                    std::string(n_unid));
      return true;
    }
  }
  return false;
}

/**
 * @brief Verify if DSKs are already in the network.
 *
 * Iterate through the SmartStart list and verify if some DSK are not already in
 * our network. When that's the case, they will be updated with the correct UNID.
 *
 * @returns true if SmartStart should be enabled and at least 1 DSK is not in the network
 * false otherwise.
 */
static bool does_one_entry_need_inclusion()
{
  bool has_dsk_awaiting_inclusion = false;
  zwave_dsk_t dsk_internal        = {0};

  for (const auto &[key, value]: Management::get_instance()->get_cache()) {
    if (SL_STATUS_OK
          == zpc_converters_dsk_str_to_internal(value.dsk.c_str(), dsk_internal)
        && value.device_unid.empty()) {
      if (!is_dsk_already_included_in_current_network(dsk_internal)) {
        has_dsk_awaiting_inclusion = true;
      }
    }
  }
  return has_dsk_awaiting_inclusion;
}

void has_entries_awaiting_inclusion(bool value)
{
  if (value) {
    // Lets iterate over Entry and check if DSK already in network
    bool activate_smart_start = true;

    // Do not do anything with SmartStart if we are resetting.
    if (!zwave_controller_is_reset_ongoing()) {
      activate_smart_start = does_one_entry_need_inclusion();
    }

    zwave_network_management_enable_smart_start_add_mode(activate_smart_start);
  } else {
    zwave_network_management_enable_smart_start_add_mode(false);
  }
}

sl_status_t zwave_smartstart_management_setup_fixt(void)
{
  zwave_controller_register_callbacks(&smartstart_callbacks);
  unid_t zpc_unid;
  zwave_unid_from_node_id(zwave_network_management_get_node_id(), zpc_unid);
  Management::get_instance()->init(zpc_unid, has_entries_awaiting_inclusion);
  return SL_STATUS_OK;
}

#ifdef __cplusplus
}
#endif
