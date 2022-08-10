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
#include <vector>
#include <sstream>

#include <nlohmann/json.hpp>

#include "smartstart_management_mock.hpp"

#include "process.h"

const std::string SMARTSTART_LIST_TOPIC        = "ucl/SmartStart/List";
const std::string SMARTSTART_LIST_UPDATE_TOPIC = "ucl/SmartStart/List/Update";

std::string received_dsk;
std::string received_device_unid;
bool notify_node_added_called                = false;
bool notify_node_removed_called              = false;
bool set_manual_intervention_required_called = false;

sl_status_t notify_node_removed_return_value     = SL_STATUS_OK;
sl_status_t notify_node_added_return_value       = SL_STATUS_OK;
sl_status_t set_manual_intervention_return_value = SL_STATUS_OK;

#define LOG_TAG "smartstart_management"

PROCESS(smartstart_management_process, "SmartStart");

typedef enum { SMARTSTART_UPDATE_LIST_CACHE } smartstart_events;

namespace smartstart
{
Management *Management::_instance;

const std::unordered_map<std::string, Entry> &Management::get_cache() const
{
  return _smartstart_cache;
}

std::vector<Entry> Management::get_cache_entries(const Query &query)
{
  std::vector<Entry> entries;
  switch (query.query_key) {
    case QueryKey::dsk: {
      const std::string *query_value
        = std::get_if<std::string>(&query.query_value);
      if (query_value != nullptr) {
        if (query.query_type == QueryType::exact) {
          if (_smartstart_cache.find(*query_value) != _smartstart_cache.end()) {
            entries.push_back(_smartstart_cache[*query_value]);
          }
        } else if (query.query_type == QueryType::partial) {
          for (const auto &entry: _smartstart_cache) {
            if (entry.second.dsk.find(*query_value) != std::string::npos) {
              entries.push_back(entry.second);
            }
          }
        }
      }
      break;
    }
    case QueryKey::device_unid: {
      const std::string *query_value
        = std::get_if<std::string>(&query.query_value);
      if (query_value != nullptr) {
        for (const auto &entry: _smartstart_cache) {
          if (entry.second.device_unid.find(*query_value)
              != std::string::npos) {
            entries.push_back(entry.second);
          }
        }
      }
      break;
    }
    case QueryKey::include: {
      const bool *query_value = std::get_if<bool>(&query.query_value);
      if (query_value != nullptr) {
        for (const auto &entry: _smartstart_cache) {
          if ((entry.second.include == *query_value)
              && (entry.second.protocol_controller_unid.empty()
                  || entry.second.protocol_controller_unid
                       == this->_protocol_controller_unid)) {
            entries.push_back(entry.second);
          }
        }
      }
      break;
    }
  }
  return entries;
}

bool Management::has_entries_awaiting_inclusion()
{
  Query q(QueryType::exact, QueryKey::include, true);
  std::vector<Entry> entries_to_include = this->get_cache_entries(q);
  for (Entry entry: entries_to_include) {
    if (entry.include && entry.device_unid.empty()) {
      return true;
    }
  }
  return false;
}

sl_status_t Management::notify_node_added(const std::string &dsk,
                                          const std::string &device_unid)
{
  notify_node_added_called = true;
  received_dsk             = dsk;
  received_device_unid     = device_unid;
  return notify_node_added_return_value;
}

sl_status_t Management::notify_node_removed(const std::string &device_unid)
{
  notify_node_removed_called = true;
  received_device_unid       = device_unid;
  return notify_node_removed_return_value;
}

sl_status_t Management::set_manual_intervention_required(const std::string &dsk,
                                                         bool new_value)
{
  set_manual_intervention_required_called = true;
  received_dsk                            = dsk;
  return set_manual_intervention_return_value;
}

sl_status_t
  Management::update_smartstart_cache(const std::string &smartstart_list)
{
  std::unordered_map<std::string, Entry> _smartstart_cache_temp;

  bool has_entries_awaiting_inclusion = false;

  try {
    nlohmann::json jsn   = nlohmann::json::parse(smartstart_list);
    nlohmann::json value = jsn;

    for (auto &element: value) {
      try {
        std::vector<std::string> preferred_protocols;

        Entry entry = {element["DSK"],
                       element["Include"],
                       element["ProtocolControllerUnid"],
                       element["Unid"]};

        // PreferredProtocols is an optional field
        if (!element["PreferredProtocols"].is_null()) {
          for (auto &prot: element["PreferredProtocols"]) {
            preferred_protocols.push_back(prot.front());
          }

          entry.preferred_protocols = preferred_protocols;
        }

        // ManualInterventionRequired is an optional field. Assume false if not present.
        if (element["ManualInterventionRequired"].is_null()) {
          entry.manual_intervention_required = false;
        } else {
          entry.manual_intervention_required
            = element["ManualInterventionRequired"];
        }

        _smartstart_cache_temp.try_emplace(entry.dsk, entry);

        if (entry.include && entry.device_unid.empty()
            && (entry.protocol_controller_unid.empty()
                || entry.protocol_controller_unid
                     == this->_protocol_controller_unid)) {
          has_entries_awaiting_inclusion = true;
        }
      } catch (std::exception &err) {
        sl_log_warning(LOG_TAG,
                       "Failed to parse SmartStart entry, skipping entry: %s\n",
                       err.what());
      }
    }
  } catch (std::exception &err) {
    sl_log_warning(LOG_TAG,
                   "Failed to parse SmartStart list: %s\n",
                   err.what());
    return SL_STATUS_FAIL;
  }

  // We've got here with success parsing so update the real cache.
  _smartstart_cache = _smartstart_cache_temp;
  // Notify that we have entries for inclusion
  if (_notify_has_entries_awaiting_inclusion) {
    _notify_has_entries_awaiting_inclusion(has_entries_awaiting_inclusion);
  }
  return SL_STATUS_OK;
}

Management *Management::get_instance()
{
  if (_instance == nullptr) {
    _instance = new Management();
  }
  return _instance;
}

sl_status_t Management::init(const std::string &unid,
                             notification_function_t const &f)
{
  if (f) {
    this->_protocol_controller_unid              = unid;
    this->_notify_has_entries_awaiting_inclusion = f;
    process_start(&smartstart_management_process, NULL);
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

sl_status_t Management::teardown()
{
  process_exit(&smartstart_management_process);
  return SL_STATUS_OK;
}

} /* namespace smartstart */

PROCESS_THREAD(smartstart_management_process, ev, data)
{
  PROCESS_BEGIN()
  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
    } else if (ev == SMARTSTART_UPDATE_LIST_CACHE) {
    } else if (ev == PROCESS_EVENT_EXIT) {
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}