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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sl_status.h"

#include "zigpc_common_zigbee.h"
#include "zigpc_common_unid.h"

zigbee_eui64_uint_t zigbee_eui64_to_uint(const zigbee_eui64_t eui64)
{
  zigbee_eui64_uint_t dest = 0U;
  zigbee_eui64_uint_t tmp;

  if (eui64 == NULL) {
    dest = 0xFFFFFFFFFFFFFFFF;
  } else {
    dest = 0U;
    for (size_t i = 0U; i < ZIGBEE_EUI64_SIZE; i++) {
      tmp          = eui64[i];
      size_t shift = ((ZIGBEE_EUI64_SIZE - i - 1) * 8U);
      tmp <<= shift;
      dest |= tmp;
    }
  }

  return dest;
}

sl_status_t zigbee_uint_to_eui64(zigbee_eui64_uint_t src, zigbee_eui64_t dest)
{
  sl_status_t status = SL_STATUS_OK;

  if (dest == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    for (size_t i = 0U; i < ZIGBEE_EUI64_SIZE; i++) {
      size_t shift = ((ZIGBEE_EUI64_SIZE - i - 1) * 8U);
      dest[i]      = (src >> shift) & 0xFF;
    }
  }

  return status;
}

void zigbee_eui64_copy_switch_endian(zigbee_eui64_t dst,
                                     const zigbee_eui64_t src)
{
  for (uint8_t i = 0; i < ZIGBEE_EUI64_SIZE; i++) {
    dst[ZIGBEE_EUI64_SIZE - i - 1] = src[i];
  }
}

void zigpc_common_switch_endian(uint8_t *source, uint8_t *dest, size_t size)
{
    unsigned int i;
    for(i = 0; i < size; i++)
    {
        dest[i] = source[size - i - 1];
    }
}

sl_status_t zigbee_eui64_to_str(const zigbee_eui64_t eui64,
                                char *str_buf,
                                const size_t str_length)
{
  sl_status_t status = SL_STATUS_OK;
  char *str_buf_ptr;

  if ((eui64 == NULL) || (str_buf == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  } else if (str_length < ZIGBEE_EUI64_HEX_STR_LENGTH) {
    status = SL_STATUS_WOULD_OVERFLOW;
  } else {
    str_buf_ptr = str_buf;

    for (size_t i = 0; i < ZIGBEE_EUI64_SIZE; i++) {
      snprintf(str_buf_ptr, 3, "%02X", eui64[i]);
      str_buf_ptr += 2;
    }
  }

  return status;
}

sl_status_t str_to_zigbee_eui64(const char *str_buf,
                                const size_t str_length,
                                zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;
  char string_buf[ZIGBEE_EUI64_HEX_STR_LENGTH];
  const size_t string_eui64_size = (ZIGBEE_EUI64_HEX_STR_LENGTH - 1);

  if (str_buf == NULL || eui64 == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else if (str_length == 0) {
    status = SL_STATUS_EMPTY;
  } else {
    if (str_length < string_eui64_size) {
      const size_t padding = string_eui64_size - str_length;
      for (size_t k = 0; k < padding; k++) {
        string_buf[k] = '0';
      }

      strncpy(&string_buf[padding], str_buf, str_length);
    } else {
      size_t index = str_length - string_eui64_size;
      memcpy(string_buf, &str_buf[index], string_eui64_size);
      string_buf[string_eui64_size] = '\0';
    }

    for (size_t i = 0; (i < string_eui64_size) && (status == SL_STATUS_OK);
         i        = i + 2) {
      char segment[3]     = {string_buf[i], string_buf[i + 1], '\0'};
      uint16_t next_value = strtol(segment, NULL, 16);
      if (next_value <= 0xFF) {
        size_t eui_index = i / 2;
        eui64[eui_index] = next_value;
      } else {
        status = SL_STATUS_FAIL;
      }
    }
  }

  return status;
}

sl_status_t zigpc_common_eui64_to_unid(const zigbee_eui64_t eui64,
                                       char *dest_unid,
                                       const size_t dest_unid_length)
{
  sl_status_t status = SL_STATUS_OK;
  char eui64_str[ZIGBEE_EUI64_HEX_STR_LENGTH];
  size_t prefix_length = strlen(ZIGPC_UNID_PREFIX);

  if ((eui64 == NULL) || (dest_unid == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  } else if ((ZIGBEE_EUI64_HEX_STR_LENGTH + prefix_length) > dest_unid_length) {
    status = SL_STATUS_WOULD_OVERFLOW;
  } else {
    status = zigbee_eui64_to_str(eui64, eui64_str, ZIGBEE_EUI64_HEX_STR_LENGTH);
  }
  if (status == SL_STATUS_OK) {
    snprintf(dest_unid, dest_unid_length, "%s%s", ZIGPC_UNID_PREFIX, eui64_str);
  }

  return status;
}

bool is_valid_zcl_command(const zcl_command_t command)
{
  bool is_valid = false;

  bool is_valid_standard_cluster
    = (command.cluster_id >= ZCL_MIN_CLUSTER_ID)
      && (command.cluster_id <= ZCL_MAX_CLUSTER_ID);

  bool is_valid_mfctr_cluster = (command.cluster_id >= ZCL_MIN_MFCTR_ID)
                                && (command.cluster_id <= ZCL_MAX_MFCTR_ID);

  bool is_valid_command = (command.command_id >= ZCL_MIN_COMMAND_ID)
                          && (command.command_id <= ZCL_MAX_COMMAND_ID);

  is_valid
    = (is_valid_standard_cluster || is_valid_mfctr_cluster) && is_valid_command;

  return is_valid;
}

bool is_valid_zcl_attribute(const zcl_attribute_t attribute)
{
  bool is_valid         = false;
  bool is_valid_cluster = ((attribute.cluster_id >= ZCL_MIN_CLUSTER_ID)
                           && (attribute.cluster_id <= ZCL_MAX_CLUSTER_ID))
                          || ((attribute.cluster_id >= ZCL_MIN_MFCTR_ID)
                              && (attribute.cluster_id <= ZCL_MAX_MFCTR_ID));

  bool is_valid_standard_attr
    = (attribute.attribute_id >= ZCL_MIN_STD_ATTR_ID)
      && (attribute.attribute_id <= ZCL_MAX_STD_ATTR_ID);

  bool is_valid_global_attr
    = (attribute.attribute_id >= ZCL_MIN_GLOB_ATTR_ID)
      && (attribute.attribute_id <= ZCL_MAX_GLOB_ATTR_ID);

  bool is_valid_type = attribute.type != ZCL_ATTR_TYPE_INVALID;

  bool has_valid_payload = (attribute.desired_payload[0] != '\0')
                           && (attribute.reported_payload[0] != '\0');

  is_valid = (is_valid_standard_attr || is_valid_global_attr)
             && is_valid_cluster && is_valid_type && has_valid_payload;

  return is_valid;
}

bool is_valid_zcl_cluster(const zcl_cluster_type_t cluster)
{
  bool is_valid = ((cluster.cluster_id >= ZCL_MIN_CLUSTER_ID)
                   && (cluster.cluster_id <= ZCL_MAX_CLUSTER_ID))
                  || ((cluster.cluster_id >= ZCL_MIN_MFCTR_ID)
                      && (cluster.cluster_id <= ZCL_MAX_MFCTR_ID));

  return is_valid;
}

bool is_valid_zigbee_node(const zigbee_node_t node)
{
  //To be determined - what is acceptable in a 'valid' node
  //In the meantime, always valid
  bool is_valid = true;
  return is_valid;
}

bool is_valid_zigbee_endpoint(const zigbee_endpoint_t endpoint)
{
  bool is_valid = true;

  return is_valid;
}

sl_status_t copy_command(const zcl_command_t source, zcl_command_t *dest)
{
  sl_status_t status;

  if (is_valid_zcl_command(source) && (dest != NULL)) {
    dest->cluster_id = source.cluster_id;
    dest->command_id = source.command_id;
    status           = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t copy_attribute_list(const zcl_attribute_t *const source,
                                zcl_attribute_t *dest,
                                unsigned int list_size)
{
  sl_status_t status = SL_STATUS_OK;

  if ((source == NULL) || (dest == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    for (size_t i = 0; (i < list_size) && (status == SL_STATUS_OK); i++) {
      if (is_valid_zcl_attribute(source[i])) {
        dest[i].cluster_id   = source[i].cluster_id;
        dest[i].attribute_id = source[i].attribute_id;
        dest[i].type         = source[i].type;

        strncpy(dest[i].attribute_name,
                source[i].attribute_name,
                ZCL_DEFAULT_STR_LENGTH - 1);
        strncpy(dest[i].desired_payload,
                source[i].desired_payload,
                ZCL_DEFAULT_STR_LENGTH - 1);
        strncpy(dest[i].reported_payload,
                source[i].reported_payload,
                ZCL_DEFAULT_STR_LENGTH - 1);

        status = SL_STATUS_OK;
      } else {
        status = SL_STATUS_FAIL;
      }
    }
  }

  return status;
}

sl_status_t copy_cluster_list(const zcl_cluster_type_t *const source,
                              zcl_cluster_type_t *dest,
                              unsigned int list_size)
{
  sl_status_t status = SL_STATUS_OK;

  if ((source == NULL) || (dest == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    for (size_t i = 0; i < list_size; i++) {
      dest[i].cluster_id = source[i].cluster_id;
      dest[i].role       = source[i].role;
    }
  }

  return status;
}

sl_status_t copy_endpoint(const zigbee_endpoint_t source,
                          zigbee_endpoint_t *dest)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (is_valid_zigbee_endpoint(source) && (dest != NULL)) {
    memcpy(dest->cluster_list,
           source.cluster_list,
           source.cluster_count * sizeof(zcl_cluster_type_t));

    dest->cluster_count = source.cluster_count;
    dest->endpoint_id   = source.endpoint_id;

    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t copy_endpoint_list(const zigbee_endpoint_t *const source,
                               zigbee_endpoint_t *dest,
                               unsigned int list_size)
{
  sl_status_t status = SL_STATUS_OK;

  if ((source == NULL) || (dest == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    unsigned int i;
    for (i = 0; (i < list_size) && (status == SL_STATUS_OK); i++) {
      dest[i].endpoint_id   = source[i].endpoint_id;
      dest[i].cluster_count = source[i].cluster_count;

      status = copy_cluster_list(source[i].cluster_list,
                                 dest[i].cluster_list,
                                 source[i].cluster_count);
    }
  }

  return status;
}

sl_status_t copy_node(const zigbee_node_t source, zigbee_node_t *dest)
{
  sl_status_t status = SL_STATUS_OK;

  if (dest == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else if (!is_valid_zigbee_node(source)) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    status = copy_endpoint_list(source.endpoint_list,
                                dest->endpoint_list,
                                source.endpoint_count);
  }

  if (status == SL_STATUS_OK) {
    dest->endpoint_count = source.endpoint_count;
    dest->is_active      = source.is_active;
    memcpy(dest->eui64, source.eui64, ZIGBEE_EUI64_SIZE);
  }

  return status;
}
