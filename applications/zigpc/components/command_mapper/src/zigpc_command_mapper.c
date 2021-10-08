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
#include <string.h>
#include <stdarg.h>

/* Shared components */
#include "sl_log.h"

/* ZigPC components */
#include "zigpc_common_zigbee.h"
#include <zigpc_datastore.h>
#include "zigpc_gateway.h"
#include "zcl_util.h"
#include "zigpc_group_mgmt.h"

#include "zigpc_command_mapper.h"
#include "zigpc_command_mapper_int.h"

static const char LOG_TAG[] = "zigpc_cmdmapper";

sl_status_t zigpc_command_mapper_populate_eui64(const char *unid,
                                                zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;

  if ((unid == NULL) || (eui64 == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    size_t unid_length = strlen(unid);
    status             = str_to_zigbee_eui64(unid, unid_length, eui64);
    if (status != SL_STATUS_OK) {
      status = SL_STATUS_INVALID_SIGNATURE;
      sl_log_warning(LOG_TAG, "Unable to convert UNID to EUI64: 0x%X", status);
    }
  }

  return status;
}

sl_status_t
  zigpc_command_mapper_cluster_support_check(const dotdot_unid_t unid,
                                             zigbee_endpoint_id_t endpoint_id,
                                             zcl_cluster_id_t cluster_id)
{
  zigbee_eui64_t eui64;
  sl_status_t status = zigpc_command_mapper_populate_eui64(unid, eui64);

  if (status == SL_STATUS_OK) {
    zigpc_cluster_data_t data;
    status = zigpc_datastore_read_cluster(eui64,
                                          endpoint_id,
                                          ZCL_CLUSTER_SERVER_SIDE,
                                          cluster_id,
                                          &data);
    if (status != SL_STATUS_OK) {
      status = SL_STATUS_NOT_FOUND;
    }
  }

  return status;
}

sl_status_t zigpc_command_mapper_send_unicast(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  const zigpc_zcl_frame_type_t frame_type,
  const zcl_cluster_id_t cluster_id,
  const zcl_command_id_t command_id,
  const size_t command_arg_count,
  const zigpc_zcl_frame_data_t *const command_arg_list)
{
  sl_status_t status = SL_STATUS_OK;
  zigbee_eui64_t eui64;
  zcl_frame_t zcl_frame;

  if (unid == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    status = zigpc_command_mapper_populate_eui64(unid, eui64);
  }

  if (status == SL_STATUS_OK) {
    status = zigpc_zcl_build_command_frame(&zcl_frame,
                                           frame_type,
                                           cluster_id,
                                           command_id,
                                           command_arg_count,
                                           command_arg_list);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to build ZCL Frame for cluster[0x%04X] "
                   "command[0x%02X]: 0x%X",
                   cluster_id,
                   command_id,
                   status);
    }
  }

  if (status == SL_STATUS_OK) {
    status = zigpc_gateway_send_zcl_command_frame(eui64,
                                                  endpoint,
                                                  cluster_id,
                                                  &zcl_frame);
    if (status != SL_STATUS_OK) {
      sl_log_error(
        LOG_TAG,
        "Failed to send ZCL Frame for cluster[0x%04X] command[0x%02X]: 0x%X ",
        cluster_id,
        command_id,
        status);
    }
  }

  if ((status == SL_STATUS_OK)
      && (cluster_id == zigpc_group_get_cluster_id())) {
    status = zigpc_command_mapper_handle_groups(eui64,
                                                endpoint,
                                                cluster_id,
                                                command_id,
                                                command_arg_count,
                                                command_arg_list);
  }

  sl_log_debug(LOG_TAG,
               "Status for cluster[0x%04X] command[0x%02X] handler for UNID: "
               "%s, endpoint: %d: 0x%X",
               cluster_id,
               command_id,
               unid,
               endpoint,
               status);

  return status;
}

sl_status_t zigpc_command_mapper_handle_groups(
  zigbee_eui64_t eui64,
  const dotdot_endpoint_id_t endpoint,
  const zcl_cluster_id_t cluster_id,
  const zcl_command_id_t command_id,
  const size_t command_arg_count,
  const zigpc_zcl_frame_data_t *const command_arg_list)
{
  sl_status_t status = SL_STATUS_OK;

  //ADD GROUP
  if ((cluster_id == zigpc_group_get_cluster_id())
      && (command_id == zigpc_group_add_get_id())) {
    if ((command_arg_count == 1) && (command_arg_list != NULL)
        && (command_arg_list[0].type == ZIGPC_ZCL_DATA_TYPE_UINT16)) {
      uint16_t group_id = *((uint16_t *)command_arg_list[0].data);
      status            = zigpc_group_add_desired(group_id, eui64, endpoint);
    } else if ((command_arg_count == 2) && (command_arg_list != NULL)
               && (command_arg_list[0].type == ZIGPC_ZCL_DATA_TYPE_UINT16)
               && (command_arg_list[1].type == ZIGPC_ZCL_DATA_TYPE_STRING)) {
      uint16_t group_id = *((uint16_t *)command_arg_list[0].data);
      char *group_name  = (char *)command_arg_list[1].data;

      status = zigpc_group_add_desired_with_name(group_id,
                                                 group_name,
                                                 eui64,
                                                 endpoint);
    } else {
      sl_log_warning(
        LOG_TAG,
        "Unable to handle add groups command - invalid argument data");
    }
  }
  //REMOVE GROUP
  else if ((cluster_id == zigpc_group_get_cluster_id())
           && (command_id == zigpc_group_remove_get_id())) {
    if ((command_arg_count == 1) && (command_arg_list != NULL)
        && (command_arg_list[0].type == ZIGPC_ZCL_DATA_TYPE_UINT16)) {
      uint16_t group_id = *((uint16_t *)command_arg_list[0].data);
      status
        = zigpc_group_remove(group_id, eui64, endpoint, ATTR_DIRECTION_DESIRED);
    } else {
      sl_log_warning(
        LOG_TAG,
        "Unable to handle remove groups command - invalid argument data");
    }
  }
  //REMOVE ALL GROUPS
  else if ((cluster_id == zigpc_group_get_cluster_id())
           && (command_id == zigpc_group_removeall_get_id())) {
    status = zigpc_group_remove_all(eui64, endpoint);
  } else {
    sl_log_warning(LOG_TAG,
                   "Unrecognized cluster/command id: cluster:%d, command%d",
                   cluster_id,
                   command_id);
  }

  return status;
}

sl_status_t zigpc_cmdmapper_redirect_to_unicast(
  zigbee_group_id_t group_id,
  zigpc_zcl_frame_type_t frame_type,
  zcl_cluster_id_t cluster_id,
  zcl_command_id_t command_id,
  size_t command_arg_count,
  const zigpc_zcl_frame_data_t *const command_arg_list)
{
  sl_status_t status        = SL_STATUS_OK;
  const size_t member_count = zigpc_groupmgmt_get_member_count(group_id);

  if (member_count == 0U) {
    status = SL_STATUS_EMPTY;
  } else {
    zigpc_group_member_t member_list[member_count];
    status
      = zigpc_groupmgmt_get_member_list(group_id, member_list, member_count);
    if (status != SL_STATUS_OK) {
      sl_log_debug(LOG_TAG,
                   "Unable to retrieve member list for groupID[0x%04X]: 0x%X",
                   group_id,
                   status);
    } else {
      char eui64_str[ZIGBEE_EUI64_HEX_STR_LENGTH];
      for (size_t i = 0U; i < member_count; i++) {
        status = zigbee_eui64_to_str(member_list[i].eui64,
                                     eui64_str,
                                     ZIGBEE_EUI64_HEX_STR_LENGTH);

        if (status != SL_STATUS_OK) {
          sl_log_debug(LOG_TAG,
                       "Failed to convert EUI64 to string: 0x%X",
                       status);
        } else {
          status = zigpc_command_mapper_send_unicast(eui64_str,
                                                     member_list[i].endpoint_id,
                                                     frame_type,
                                                     cluster_id,
                                                     command_id,
                                                     command_arg_count,
                                                     command_arg_list);

          sl_log_debug(LOG_TAG,
                       "Status for redirecting groupID[0x%04X] message to "
                       "EUI64[%s] Endpoint[%u]: 0x%X",
                       group_id,
                       eui64_str,
                       member_list[i].endpoint_id,
                       status);
        }
      }
    }
  }

  return status;
}

sl_status_t zigpc_command_mapper_send_multicast(
  const zigbee_group_id_t group_id,
  const zigpc_zcl_frame_type_t frame_type,
  const zcl_cluster_id_t cluster_id,
  const zcl_command_id_t command_id,
  const size_t command_arg_count,
  const zigpc_zcl_frame_data_t *const command_arg_list)
{
  sl_status_t status = SL_STATUS_OK;
  zcl_frame_t zcl_frame;
  size_t member_count = 0U;

  // Validate if Multicast ID is valid from group manager
  if (group_id < 0x0001 || (group_id > 0xfff7)) {
    status = SL_STATUS_INVALID_RANGE;
  } else {
    member_count = zigpc_groupmgmt_get_member_count(group_id);
    if (member_count == 0U) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  }

  if ((status == SL_STATUS_OK)
      && (cluster_id == zigpc_group_get_cluster_id())) {
    // Check for multicast group commands that should be redirected as unicasts

    status = zigpc_cmdmapper_redirect_to_unicast(group_id,
                                                 frame_type,
                                                 cluster_id,
                                                 command_id,
                                                 command_arg_count,
                                                 command_arg_list);
  } else if (status == SL_STATUS_OK) {
    status = zigpc_zcl_build_command_frame(&zcl_frame,
                                           frame_type,
                                           cluster_id,
                                           command_id,
                                           command_arg_count,
                                           command_arg_list);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to build ZCL Frame for cluster[0x%04X] "
                   "command[0x%02X]: 0x%X",
                   cluster_id,
                   command_id,
                   status);
    } else {
      status = zigpc_gateway_send_zcl_frame_multicast(group_id,
                                                      cluster_id,
                                                      &zcl_frame);
      if (status != SL_STATUS_OK) {
        sl_log_error(LOG_TAG,
                     "Failed to send multicast ZCL Frame for cluster[0x%04X] "
                     "command[0x%02X]: 0x%X ",
                     cluster_id,
                     command_id,
                     status);
      }
    }
  }

  sl_log_debug(LOG_TAG,
               "Status for handling cluster[0x%04X] command[0x%02X] handler to"
               " groupID[0x%04X]: 0x%X",
               cluster_id,
               command_id,
               group_id,
               status);

  return status;
}

sl_status_t zigpc_command_mapper_setup(void)
{
  sl_status_t status = zigpc_command_mapper_register_dotdot_mqtt_handlers();

  if (status == SL_STATUS_OK) {
    status = zigpc_command_mapper_mqtt_bygroup_handlers_init();
  }

  return status;
}

int zigpc_command_mapper_shutdown(void)
{
  return 0;
}
