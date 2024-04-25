/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// System
#include <stdlib.h>

#include "zwave_command_class_sound_switch.h"
#include "zwave_command_classes_utils.h"
#include "ZW_classcmd.h"
#include "zwave_command_class_sound_switch_types.h"
#include "zwave_command_handler.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zwave_command_class_version_types.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Unify
#include "attribute_resolver.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "sl_log.h"

#define LOG_TAG "zwave_command_class_sound_switch"

// Used to get the tone string description
// If the description has a size > to this number, we will truncate it
// Specification says that payload data should not exceeded 64 bytes.
#define MAX_CHAR_SIZE 64

/**
 * @brief Full state of a sound Switch supporting node.
 */
typedef struct sound_switch_configuration {
  sound_switch_volume_t volume;
  sound_switch_tone_id_t tone;
} sound_switch_configuration_t;


static sl_status_t zwave_command_class_sound_switch_tones_number_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_SOUND_SWITCH_TONES_NUMBER_GET_FRAME *get_frame
    = (ZW_SOUND_SWITCH_TONES_NUMBER_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_SOUND_SWITCH;
  get_frame->cmd      = SOUND_SWITCH_TONES_NUMBER_GET;
  *frame_length       = sizeof(ZW_SOUND_SWITCH_TONES_NUMBER_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_sound_switch_handle_tones_number_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 2 + 1) {
    return SL_STATUS_FAIL ;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  uint8_t tone_number = frame_data[2];
  if (tone_number == 0) {
    sl_log_warning(LOG_TAG, "Zero Tones reported");
  }

  attribute_store_node_t tone_number_node = attribute_store_get_node_child_by_type(endpoint_node, 
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONES_NUMBER, 0);
  
  attribute_store_set_reported(tone_number_node, &tone_number, sizeof(tone_number));

  for(uint8_t i=1; i <= tone_number; i++) {
    attribute_store_node_t node_id = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_IDENTIFIER, tone_number_node);
    attribute_store_set_desired(node_id, &i, sizeof(i));
    sl_log_debug(LOG_TAG, "Set desired value %d of node %d", i, node_id);
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_sound_switch_tone_info_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "zwave_command_class_sound_switch_tone_info_get : %d", node);


  sound_switch_tone_id_t tone_id = 0;
  sl_status_t status = attribute_store_get_desired(node, &tone_id, sizeof(tone_id));
  sl_log_debug(LOG_TAG, "tone_id : %d", tone_id);

  // If we don't have a desired value look for the default tone ID if present
  // NOTE : if we don't have a node ID to look for we just abort here and don't send anything.
  // This will prevent a request to go off for no reason.
  if (status != SL_STATUS_OK) {

    attribute_store_node_t parent_node = attribute_store_get_node_parent(node);
    attribute_store_node_t tone_id_node = attribute_store_get_first_child_by_type(parent_node, 
      ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_TONE_IDENTIFIER);

    status = attribute_store_get_reported(tone_id_node, &tone_id, sizeof(tone_id));

    if (status != SL_STATUS_OK) {
      // We return SL_STATUS_NOT_SUPPORTED instead of SL_STATUS_FAIL to prevent an error
      // message in the logs. This case may happen and it's not a big deal.
      return SL_STATUS_NOT_SUPPORTED;
    }
  }

  ZW_SOUND_SWITCH_TONE_INFO_GET_FRAME *get_frame
    = (ZW_SOUND_SWITCH_TONE_INFO_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_SOUND_SWITCH;
  get_frame->cmd      = SOUND_SWITCH_TONE_INFO_GET;
  get_frame->toneIdentifier = tone_id;
  *frame_length       = sizeof(ZW_SOUND_SWITCH_TONE_INFO_GET_FRAME);

  return SL_STATUS_OK;
}


sl_status_t zwave_command_class_sound_switch_handle_tone_info_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{

  // Frame format : 
  // 0 Command Class = COMMAND_CLASS_SOUND_SWITCH
  // 1 Command = SOUND_SWITCH_TONE_INFO_REPORT      SIZE_FRAME_INFO
  // 2 Tone Identifier                              SIZE_TONE_ID
  // 3 Tone Duration 1                              SIZE_TONE_DURATION       
  // 4 Tone Duration 2
  // 5 Name Length                                  SIZE_TONE_NAME_SIZE
  // 6 Name 1
  // . ...
  // N Name N

  // Size frame info
  const uint8_t SIZE_FRAME_INFO = 2;
  const uint8_t SIZE_TONE_ID = 1;
  const uint8_t SIZE_TONE_DURATION = 2;
  const uint8_t SIZE_TONE_NAME_SIZE = 1;
  const uint8_t SIZE_MESSAGE_WITHOUT_NAME = SIZE_FRAME_INFO + SIZE_TONE_ID + SIZE_TONE_DURATION + SIZE_TONE_NAME_SIZE;

  // Addresses
  const uint8_t ADDRESS_TONE_IDENTIFIER = SIZE_FRAME_INFO; // 2
  const uint8_t ADDRESS_DURATION_1 = SIZE_FRAME_INFO + 1;  // 3
  const uint8_t ADDRESS_DURATION_2 = SIZE_FRAME_INFO + 2;  // 4
  const uint8_t ADDRESS_NAME_SIZE = SIZE_FRAME_INFO + SIZE_TONE_ID + SIZE_TONE_DURATION; // 5
  const uint8_t ADDRESS_NAME_STR_BASE = ADDRESS_NAME_SIZE + 1; // 6

  // Check frame size
  if (frame_length < SIZE_MESSAGE_WITHOUT_NAME) {
    return SL_STATUS_FAIL;
  }

  // Extract info from frame
  // Tone ID
  sound_switch_tone_id_t tone_identifier = frame_data[ADDRESS_TONE_IDENTIFIER];
  // Tone duration on 16 bits :
  // frame_data[ADDRESS_DURATION_1] frame_data[ADDRESS_DURATION_2]
  // 01                             05
  // Result 0105
  sound_switch_tone_duration_t tone_duration = (frame_data[ADDRESS_DURATION_1] << 8) | frame_data[ADDRESS_DURATION_2];  
  // Size of name array
  uint8_t tone_name_size = frame_data[ADDRESS_NAME_SIZE];

  if (frame_length < SIZE_MESSAGE_WITHOUT_NAME + tone_name_size) {
    return SL_STATUS_FAIL;
  }

  // Check if our name fits our buffer ; if not it is truncated
  if (tone_name_size > MAX_CHAR_SIZE) {
    tone_name_size = MAX_CHAR_SIZE;
  }
  char tone_name[MAX_CHAR_SIZE];
  for (int i = 0; i < tone_name_size; i++) {
    tone_name[i] = frame_data[ADDRESS_NAME_STR_BASE + i];
  }
  // Since Z-Wave sends string without NULL character we must add it
  tone_name[tone_name_size] = '\0';

  // Update attribute store
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);
  
  attribute_store_node_t tone_number_node = 
    attribute_store_get_node_child_by_type(endpoint_node, 
      ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONES_NUMBER, 0);

  // Find tone identifier
  attribute_store_node_t id_node = attribute_store_get_node_child_by_value(
    tone_number_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_IDENTIFIER,
    DESIRED_ATTRIBUTE,
    &tone_identifier,
    sizeof(tone_identifier), 0);

  attribute_store_set_reported(id_node, &tone_identifier, sizeof(tone_identifier));

  // Store attributes
  attribute_store_set_child_reported(
    id_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_DURATION_SECONDS,
    &tone_duration,
    sizeof(tone_duration));

  // We need to use tone_name_size+1 to take into account the \0 character needed by C
  // If it is not there the attribute cannot be written.
  attribute_store_set_child_reported(
    id_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_NAME,
    &tone_name,
    tone_name_size+1);

  return SL_STATUS_OK;
}


static sl_status_t zwave_command_class_sound_switch_configuration_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_SOUND_SWITCH_CONFIGURATION_GET_FRAME *get_frame
    = (ZW_SOUND_SWITCH_CONFIGURATION_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_SOUND_SWITCH;
  get_frame->cmd      = SOUND_SWITCH_CONFIGURATION_GET;
  *frame_length       = sizeof(ZW_SOUND_SWITCH_CONFIGURATION_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_sound_switch_handle_configuration_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{

  if (frame_length < 4) {
    return SL_STATUS_FAIL;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  sound_switch_volume_t volume = frame_data[2];
  if (volume > 100) {
    sl_log_warning(LOG_TAG, "Node reported volume higher than 100");
    volume = 100;
  }

  attribute_store_set_child_reported(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME,
    &volume,
    sizeof(volume));

  sound_switch_tone_id_t tone = frame_data[3];

  attribute_store_set_child_reported(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_TONE_IDENTIFIER,
    &tone,
    sizeof(tone));

  return SL_STATUS_OK;
}


static void get_configuration(attribute_store_node_t state_node,
                              sound_switch_configuration_t *configuration)
{
  attribute_store_node_t volume_node
    = attribute_store_get_first_child_by_type(state_node,
                                              ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME);

  sl_status_t status = attribute_store_get_desired_else_reported(volume_node, 
                              &configuration->volume,
                              sizeof(configuration->volume));
  if (status != SL_STATUS_OK) {
    configuration->volume = 0;
    sl_log_warning(LOG_TAG, "Can't get CONFIGURED_DEFAULT_VOLUME from attribute store. Value set to 0.");
  }

  attribute_store_node_t tone_node 
    = attribute_store_get_first_child_by_type(state_node,
                                              ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_TONE_IDENTIFIER);

  status = attribute_store_get_desired_else_reported(tone_node,
                             &configuration->tone,
                             sizeof(configuration->tone));

  if (status != SL_STATUS_OK) {
    configuration->tone = 0;
    sl_log_warning(LOG_TAG, "Can't get CONFIGURED_DEFAULT_TONE_IDENTIFIER from attribute store. Value set to 0.");
  }
}

static sl_status_t zwave_command_class_sound_switch_configuration_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sound_switch_configuration_t configuration = {};

  const attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  get_configuration(endpoint_id_node, &configuration);

  ZW_SOUND_SWITCH_CONFIGURATION_SET_FRAME *set_frame
    = (ZW_SOUND_SWITCH_CONFIGURATION_SET_FRAME *)frame;
  set_frame->cmdClass              = COMMAND_CLASS_SOUND_SWITCH;
  set_frame->cmd                   = SOUND_SWITCH_CONFIGURATION_SET;
  set_frame->volume                = configuration.volume;
  set_frame->defaultToneIdentifier = configuration.tone;
  *frame_length = sizeof(ZW_SOUND_SWITCH_CONFIGURATION_SET_FRAME);
  return SL_STATUS_OK;
} 

static void zwave_monitor_dotdot_on_off_on_time_update(
  attribute_store_node_t updated_node, attribute_store_change_t change) {

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t play_node = attribute_store_get_first_child_by_type(endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY);

  // We are not in a sound switch environemnt, we can don't care about this update
  if (!attribute_store_node_exists(play_node)) {
    return;
  }

  // We don't care if it's any other change type than updated
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  uint16_t on_time = 0;
  sl_status_t status = attribute_store_get_desired(updated_node, &on_time, sizeof(on_time));

  // We can be in this state for many reasons (e.g. desired attribute undefined)
  // In that case we do nothing and move on.
  if (status != SL_STATUS_OK) {
    return;
  }

  // Check overflow since our play attribute is only a uint8_t
  if (on_time > 255) {
    on_time = 255;
  }

  sound_switch_tone_id_t tone_id = (sound_switch_tone_id_t)on_time;
  // Copy attribute to the play node
  attribute_store_set_desired(play_node, &tone_id, sizeof(sound_switch_tone_id_t));
  // Undefine temp buffer
  attribute_store_undefine_desired(updated_node);
}

static void zwave_command_class_sound_switch_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_SOUND_SWITCH, updated_node) == true) {
    return;
  }

  zwave_cc_version_t version = 0;
  attribute_store_get_reported(updated_node, &version, sizeof(version));

  if (version == 0) {
    return;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // Attribute initialisation order is important here since it determines in 
  // which order the GET commands will be sent
  const attribute_store_type_t attributes[]
    = {
       ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONES_NUMBER,
       ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME,
       ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_TONE_IDENTIFIER,
       ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY};

  attribute_store_add_if_missing(endpoint_node,
                                 attributes,
                                 COUNT_OF(attributes));
}

static sl_status_t zwave_command_class_sound_switch_tone_play_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Check version
  attribute_store_node_t endpoint_node = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node = attribute_store_get_node_child_by_type(endpoint_node, ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_VERSION, 0);

  zwave_cc_version_t current_supported_version = 0;
  attribute_store_get_desired_else_reported(version_node, &current_supported_version, sizeof(current_supported_version));

  // Attributes for the set command
  // 0 : Stop tone | 255 : Default tone
  sound_switch_tone_id_t play = 0;

  // Get value of play ; if play == 0 we will send a terminate sound command
  attribute_store_get_desired_else_reported(node, &play, sizeof(play));

  if (current_supported_version == 2) {
    // Default volume 
    sound_switch_volume_t volume = 255;
    // Check volume if we doesn't try to stop the 
    if (play != 0) {
      attribute_store_node_t volume_node = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME);

      attribute_store_get_desired_else_reported(volume_node, &volume, sizeof(volume));
    }

    ZW_SOUND_SWITCH_TONE_PLAY_SET_V2_FRAME *set_frame
      = (ZW_SOUND_SWITCH_TONE_PLAY_SET_V2_FRAME *)frame;
    set_frame->cmdClass       = COMMAND_CLASS_SOUND_SWITCH;
    set_frame->cmd            = SOUND_SWITCH_TONE_PLAY_SET;
    set_frame->toneIdentifier = play;
    set_frame->playCommandToneVolume = volume;
    *frame_length             = sizeof(ZW_SOUND_SWITCH_TONE_PLAY_SET_V2_FRAME);
  } else {
    ZW_SOUND_SWITCH_TONE_PLAY_SET_FRAME *set_frame
      = (ZW_SOUND_SWITCH_TONE_PLAY_SET_FRAME *)frame;
    set_frame->cmdClass       = COMMAND_CLASS_SOUND_SWITCH;
    set_frame->cmd            = SOUND_SWITCH_TONE_PLAY_SET;
    set_frame->toneIdentifier = play;
    *frame_length             = sizeof(ZW_SOUND_SWITCH_TONE_PLAY_SET_FRAME);
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_sound_switch_tone_play_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_SOUND_SWITCH_TONE_PLAY_GET_FRAME *get_frame
    = (ZW_SOUND_SWITCH_TONE_PLAY_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_SOUND_SWITCH;
  get_frame->cmd      = SOUND_SWITCH_TONE_PLAY_GET;
  *frame_length       = sizeof(ZW_SOUND_SWITCH_TONE_PLAY_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_sound_switch_handle_tone_play_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 2 + 1) {
    return SL_STATUS_FAIL;
  }

  // Update attribute store
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);


  sound_switch_tone_id_t tone_identifier = frame_data[2];
  sound_switch_volume_t volume = 0;

  // Volume is optional and only done in the version 2 of the class
  if (frame_length == 2 + 1 + 1) {
    volume = frame_data[3];

    attribute_store_set_child_reported(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME,
      &volume,
      sizeof(volume));
  }

  // Write the current tone identifier to the tone play attribute.
  // Note that the device will publish a report with the tone_id = 0 when
  // the sound stops.
  attribute_store_set_child_reported(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY,
    &tone_identifier,
    sizeof(tone_identifier));

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_sound_switch_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case SOUND_SWITCH_TONES_NUMBER_REPORT:
      return zwave_command_class_sound_switch_handle_tones_number_report(
        connection_info,
        frame_data,
        frame_length);

    case SOUND_SWITCH_TONE_INFO_REPORT:
      return zwave_command_class_sound_switch_handle_tone_info_report(
        connection_info,
        frame_data,
        frame_length);

    case SOUND_SWITCH_CONFIGURATION_REPORT:
      return zwave_command_class_sound_switch_handle_configuration_report(
        connection_info,
        frame_data,
        frame_length);

    case SOUND_SWITCH_TONE_PLAY_REPORT:
      return zwave_command_class_sound_switch_handle_tone_play_report(
        connection_info,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

sl_status_t zwave_command_class_sound_switch_init()
{
  attribute_store_register_callback_by_type(
    &zwave_command_class_sound_switch_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_VERSION);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONES_NUMBER,
    NULL,
    &zwave_command_class_sound_switch_tones_number_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_IDENTIFIER,
    NULL,
    &zwave_command_class_sound_switch_tone_info_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME,
    &zwave_command_class_sound_switch_configuration_set,
    &zwave_command_class_sound_switch_configuration_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_TONE_IDENTIFIER,
    &zwave_command_class_sound_switch_configuration_set,
    &zwave_command_class_sound_switch_configuration_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY,
    &zwave_command_class_sound_switch_tone_play_set,
    &zwave_command_class_sound_switch_tone_play_get);

  // Workaround to allow to send a specific tone ID though this attribute.
  //
  // The OnOff UCL cluster only support boolean states and we need a uint8_t to be able to 
  // send specific toneID though the Play Set command.
  //  
  // There is 3 way to do this :
  //  - Add an attribute to an existing UCL cluster
  //  - Add a whole new cluster for sound switch
  //  - Map this functionality to an unused property. 
  //
  // The latest has been choosen to avoid changes on clusters. This approach might change
  // in future release, don't take this as a reference for now.
  //
  // With this workaround we can :
  //  - Play the default tone (255) with the ZCL OnOff attribute (1)
  //  - Stop the tone (0) with the ZCL OnOff attribute (0)
  //  - Play specific tone (1-254) with the ZCL OnTime attribute (1-254) that will be "copied" into our ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY
  //    - It can be controlled by the command WriteAttribute of the OnOff cluster though MQTT : ucl/by-unid/+/ep0/OnOff/Commands/WriteAttributes {"OnTime":2}
  //
  // zwave_monitor_dotdot_on_off_on_time_update will copy the contents of DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME into 
  // ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY and then undefine itself.
  //
  // We do that here and not in UAM because we need to undefined the DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME value
  // after using it since it is used only to bring the value to ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY.
  attribute_store_register_callback_by_type_and_state(
    &zwave_monitor_dotdot_on_off_on_time_update,
    DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME,
    DESIRED_ATTRIBUTE);

  zwave_command_handler_t handler = {0};
  handler.support_handler         = NULL;
  handler.control_handler = &zwave_command_class_sound_switch_control_handler;
  handler.minimal_scheme  = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_SOUND_SWITCH;
  handler.version                    = SOUND_SWITCH_VERSION_V2;
  handler.command_class_name         = "Sound Switch";
  handler.comments                   = "You can control the specific tone play with the ON_OFF_ON_TIME ZCL attribute. See code or python CTT script for details.";
  
  return zwave_command_handler_register_handler(handler);
}
