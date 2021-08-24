/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include <list>
#include <algorithm>
#include <string>
#include <cstring>
#include <stdio.h>

#include "sl_log.h"
#include "zcl_attribute_info.h"

#include "zcl_profile_list_int.h"

//TODO refactor this whole file

static const char *const LOG_TAG = "zcl_attribute_info";

static std::list<zcl_profile_t> profile_list;

bool operator==(const zcl_profile_t &a, const zcl_profile_t &b)
{
  return (a.cluster_id == b.cluster_id);
}

std::list<zcl_profile_t> &zigpc_zcl_profiles_get_profile_list(void)
{
  return profile_list;
}

size_t zigpc_zclprofiles_get_cluster_attribute_count(
  const zcl_cluster_id_t cluster_id)
{
  size_t attr_count = 0U;

  zcl_profile_t to_check;
  to_check.cluster_id = cluster_id;

  auto retrieved
    = std::find(profile_list.begin(), profile_list.end(), to_check);

  if (retrieved != profile_list.end()) {
    attr_count = retrieved->num_attributes;
  }

  return attr_count;
}

sl_status_t zigpc_zclprofiles_get_cluster_attribute_list(
  const zcl_cluster_id_t cluster_id, zcl_attribute_t *attribute_list)
{
  sl_status_t status = SL_STATUS_OK;

  zcl_profile_t to_check;
  to_check.cluster_id = cluster_id;

  auto retrieved
    = std::find(profile_list.begin(), profile_list.end(), to_check);

  if (retrieved != profile_list.end()) {
    memcpy(attribute_list,
           retrieved->attribute_list,
           retrieved->num_attributes * sizeof(zcl_attribute_t));
    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }
  return status;
}

sl_status_t attribute_val_to_string(uint8_t *data,
                                    enum e_zcl_attribute_type type,
                                    char *str,
                                    unsigned int strlen)
{
  sl_status_t status = SL_STATUS_OK;

  if ((data == NULL) || (str == NULL)) {
    type = ZCL_ATTR_TYPE_INVALID;
  }

  switch (type) {
    case ZCL_ATTR_TYPE_BOOL:
      bool bool_value;
      memcpy(&bool_value, data, 1);
      snprintf(str, strlen, "%u", bool_value);
      break;

    case ZCL_ATTR_TYPE_INT16:
      int16_t int_value;
      memcpy(&int_value, data, 2);
      snprintf(str, strlen, "%d", int_value);
      break;
    case ZCL_ATTR_TYPE_ENUM8:
    case ZCL_ATTR_TYPE_MAP8:
    case ZCL_ATTR_TYPE_UINT8:
      uint8_t uint8_value;
      memcpy(&uint8_value, data, 1);
      snprintf(str, strlen, "%u", uint8_value);
      break;
    case ZCL_ATTR_TYPE_UINT16:
      uint16_t uint_value;
      memcpy(&uint_value, data, 2);
      snprintf(str, strlen, "%u", uint_value);
      break;

    case ZCL_ATTR_TYPE_FLOAT:
      float f_value;
      memcpy(&f_value, data, 2);
      snprintf(str, strlen, "%f", f_value);
      break;

    default:
      status = SL_STATUS_FAIL;
      break;
  }

  return status;
}

#ifdef COMPILE_UNUSED_CODE

sl_status_t retrieve_cluster_name_length(const zcl_cluster_id_t cluster_id,
                                         unsigned int *cluster_name_len)
{
  sl_status_t status = SL_STATUS_OK;

  zcl_profile_t to_check;
  to_check.cluster_id = cluster_id;

  auto retrieved
    = std::find(profile_list.begin(), profile_list.end(), to_check);

  if ((retrieved != profile_list.end()) && (cluster_name_len != NULL)) {
    *cluster_name_len = strlen(retrieved->cluster_name) + 1;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t retrieve_cluster_name(const zcl_cluster_id_t cluster_id,
                                  char *cluster_name,
                                  unsigned int string_len)
{
  sl_status_t status = SL_STATUS_OK;

  zcl_profile_t to_check;
  to_check.cluster_id = cluster_id;

  auto retrieved
    = std::find(profile_list.begin(), profile_list.end(), to_check);

  if ((retrieved != profile_list.end()) && (cluster_name != NULL)) {
    snprintf(cluster_name, string_len, "%s", retrieved->cluster_name);
    sl_log_debug(LOG_TAG, "Found cluster name %s", cluster_name);
  } else {
    status = SL_STATUS_OK;
  }

  return status;
}

sl_status_t retrieve_cluster_id_from_name(const char *cluster_name,
                                          unsigned int strlen,
                                          zcl_cluster_id_t *cluster_id)
{
  sl_status_t status = SL_STATUS_OK;

  if (cluster_name == NULL || cluster_id == NULL) {
    return SL_STATUS_FAIL;
  }

  bool found = false;

  for (auto it = profile_list.begin();
       (it != profile_list.end()) && (found == false);
       it++) {
    int result = strncmp(it->cluster_name, cluster_name, strlen);

    if (result == 0) {
      found       = true;
      *cluster_id = it->cluster_id;
      status      = SL_STATUS_OK;
    }
  }

  if (!found) {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t
  retrieve_attribute_name_length(const zcl_cluster_id_t cluster_id,
                                 const zcl_attribute_id_t attribute_id,
                                 unsigned int *length)
{
  sl_status_t status = SL_STATUS_FAIL;

  zcl_profile_t to_check;
  to_check.cluster_id = cluster_id;

  auto retrieved
    = std::find(profile_list.begin(), profile_list.end(), to_check);

  if ((retrieved != profile_list.end()) && (length != NULL)) {
    unsigned int i;
    for (i = 0; (i < retrieved->num_attributes) && (status != SL_STATUS_OK);
         i++) {
      unsigned next_id = retrieved->attribute_list[i].attribute_id;

      if (next_id == attribute_id) {
        char name[32];
        strncpy(name, retrieved->attribute_list[i].attribute_name, 32);

        *length = strlen(name) + 1;
        status  = SL_STATUS_OK;

        sl_log_debug(LOG_TAG,
                     "Found length for CLUSTER %d ATTR %d, NAME %s",
                     cluster_id,
                     attribute_id,
                     name);
      }
    }
  }

  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Failted to find length for CLUSTER %d ATTR %d",
                   cluster_id,
                   attribute_id);
  }

  return status;
}

sl_status_t retrieve_attribute_name(const zcl_cluster_id_t cluster_id,
                                    const zcl_attribute_id_t attribute_id,
                                    char *attribute_name,
                                    unsigned int str_length)
{
  sl_status_t status = SL_STATUS_FAIL;

  zcl_profile_t to_check;
  to_check.cluster_id = cluster_id;

  auto retrieved
    = std::find(profile_list.begin(), profile_list.end(), to_check);

  if ((retrieved != profile_list.end()) && (attribute_name != NULL)) {
    unsigned int i;
    for (i = 0; (i < retrieved->num_attributes) && (status != SL_STATUS_OK);
         i++) {
      unsigned next_id = retrieved->attribute_list[i].attribute_id;

      if (next_id == attribute_id) {
        snprintf(attribute_name,
                 str_length,
                 "%s",
                 retrieved->attribute_list[i].attribute_name);

        status = SL_STATUS_OK;

        sl_log_debug(LOG_TAG,
                     "Found for attr %d name %s",
                     retrieved->attribute_list[i].attribute_id,
                     retrieved->attribute_list[i].attribute_name);
      }
    }
  }

  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Failed to find info for CLUSTER %d ATTR %d",
                   cluster_id,
                   attribute_id);
  }

  return status;
}

sl_status_t retrieve_attribute_type(const zcl_cluster_id_t cluster_id,
                                    const zcl_attribute_id_t attribute_id,
                                    enum e_zcl_attribute_type *type)
{
  sl_status_t status = SL_STATUS_FAIL;

  zcl_profile_t to_check;
  to_check.cluster_id = cluster_id;

  auto retrieved
    = std::find(profile_list.begin(), profile_list.end(), to_check);

  if ((retrieved != profile_list.end()) && (type != NULL)) {
    unsigned int i;
    for (i = 0; (i < retrieved->num_attributes) && (status != SL_STATUS_OK);
         i++) {
      unsigned next_id = retrieved->attribute_list[i].attribute_id;

      if (next_id == attribute_id) {
        *type = retrieved->attribute_list[i].type;

        status = SL_STATUS_OK;

        sl_log_debug(LOG_TAG,
                     "Found for attr %d name %s",
                     retrieved->attribute_list[i].attribute_id,
                     retrieved->attribute_list[i].attribute_name);
      }
    }
  }

  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Failed to find info for CLUSTER %d ATTR %d",
                   cluster_id,
                   attribute_id);
  }

  return status;
}

sl_status_t retrieve_attribute_id_from_name(const zcl_cluster_id_t cluster_id,
                                            const char *attribute_name,
                                            unsigned int str_length,
                                            zcl_attribute_id_t *attribute_id)
{
  sl_status_t status = SL_STATUS_OK;
  size_t num_attributes;

  std::string attribute_str(attribute_name);

  num_attributes = zigpc_zclprofiles_get_cluster_attribute_count(cluster_id);

  if ((status == SL_STATUS_OK) && (num_attributes > 0)) {
    zcl_attribute_t attribute_list[num_attributes];

    status = zigpc_zclprofiles_get_cluster_attribute_list(cluster_id,
                                                          attribute_list);

    bool found_name = false;
    unsigned int i;
    for (i = 0; (i < num_attributes) && (!found_name); i++) {
      std::string next_name(attribute_list[i].attribute_name);
      int result = attribute_str.compare(next_name);

      found_name = (result == 0);

      if (found_name) {
        *attribute_id = attribute_list[i].attribute_id;
        status        = SL_STATUS_OK;
      }
    }

    if (!found_name) {
      status = SL_STATUS_FAIL;
    }
  }

  return status;
}

//TODO unify this with zcl_types in zcl_util
sl_status_t attribute_string_to_val(const char *value_cstr,
                                    unsigned int strlen,
                                    enum e_zcl_attribute_type type,
                                    uint8_t *data,
                                    size_t data_size)
{
  sl_status_t status = SL_STATUS_OK;

  std::string value_base;

  if ((data == NULL) || (value_cstr == NULL)) {
    type = ZCL_ATTR_TYPE_INVALID;
  } else {
    value_base = value_cstr;
  }

  uint8_t buffer[data_size];

  switch (type) {
    case ZCL_ATTR_TYPE_BOOL:
      bool bool_value;
      bool_value = (value_base != "0") && (value_base != "false");

      memcpy(buffer, &bool_value, data_size);

      break;

    case ZCL_ATTR_TYPE_INT16:
      int16_t int_value;
      int_value = std::stoi(value_base);
      memcpy(buffer, &int_value, data_size);

      break;

    case ZCL_ATTR_TYPE_ENUM8:
    case ZCL_ATTR_TYPE_MAP8:
    case ZCL_ATTR_TYPE_UINT8:
      uint8_t uint8_value;
      uint8_value = std::stoul(value_base);
      memcpy(buffer, &uint8_value, data_size);

      break;

    case ZCL_ATTR_TYPE_UINT16:
      uint16_t uint16_value;
      uint16_value = std::stoul(value_base);
      memcpy(buffer, &uint16_value, data_size);

      break;

    case ZCL_ATTR_TYPE_FLOAT:
      float f_value;
      f_value = std::stof(value_base);
      memcpy(buffer, &f_value, data_size);

      break;

    default:
      status = SL_STATUS_FAIL;
      break;
  }

  if (status == SL_STATUS_OK) {
    zigpc_common_switch_endian(buffer, data, data_size);
  }

  return status;
}
#endif  // COMPILE_UNUSED_CODE
