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

#define ATTRIBUTE_AUTO_GENERATED


// Generic includes
#include <assert.h>
#include <stdlib.h>

// Includes from other components
#include "attribute.hpp"
#include "attribute_resolver.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_helper.h"
#include "sl_log.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_command_classes_utils.h"
#include "zwave_command_handler.h"
#include "zwave_unid.h"

// Log tag
#define LOG_TAG "zwave_{{_name}}"

#include "zwave_{{_name}}_attribute_id.h"
#include "zwave_{{_name}}_handlers.h"

using namespace attribute_store;

///////////////////////////////////////////////////////////////////////////////
// Private functions, used to handle individual incoming commands.
///////////////////////////////////////////////////////////////////////////////

// Override functions

{{#cmd_type "_REPORT"}}
sl_status_t  __attribute__((weak)) zwave_{{_name}}_handle_report_command_override(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  return SL_STATUS_IDLE;
}
{{/cmd_type}}

{{#cmd_type "_GET"}}
sl_status_t  __attribute__((weak)) zwave_{{_name}}_override(
  attribute_store_node_t   _node, uint8_t *frame, uint16_t *frame_len)
{
  *frame_len = 0;
  return SL_STATUS_IDLE;
}
{{/cmd_type}}

{{#cmd_type "_SET"}}
sl_status_t __attribute__((weak)) zwave_{{_name}}_override(
  attribute_store_node_t    _node, uint8_t *frame, uint16_t *frame_len)
{
  *frame_len = 0;
  return SL_STATUS_IDLE;
}
{{/cmd_type}}

/**
 * Generators for {{_name}}_GET Commands
 */

{{#cmd_type "_GET"}}
static sl_status_t zwave_{{_name}}(
  attribute_store_node_t _node, uint8_t *frame, uint16_t *frame_len)
{
   sl_status_t rc = zwave_{{_name}}_override(_node, frame, frame_len);
   if( *frame_len > 0 ) {
     return rc;
  }  

  try {
    uint8_t offset=0;
    uint8_t size=0;
    uint8_t param_offsets[255] = {0}; // To avoid -Wno-maybe-uninitialized

    attribute node(_node);
    attribute parent = node.first_parent(ATTRIBUTE_ENDPOINT_ID);

    frame[offset++] = {{_cc_key}};
    frame[offset++] = {{_key}}; // {{_name}}
    {{#param}}
    //parameter {{_name}} {{_type}}
      {{#if has_attr}}
    node = parent.child_by_type(ATTRIBUTE_{{att.name}});
          {{#if (equals _length 0) }}
    //parameter is a mask inside a byte
    frame[offset] |= (node.reported<int32_t>()<< {{_shifter}}) & {{_fieldmask}};
          {{else}}
    //parameter is {{_length}} bytes long
    node.get(REPORTED_ATTRIBUTE, &frame[offset], &size );
          {{/if}}
      {{/if}}
    {{#if _size_key }}
      {{#if (equals _size_key "255") }}
    // Size key: {{_size_key}}, length is determined by message length
    offset += size;
      {{else}}
    frame[ param_offsets[{{_size_key}}] ]  |= (size & {{_size_mask}} );
    param_offsets[{{_key}}] = offset;  offset += size;
      {{/if}}
    {{else}}
    {{#if _length }}param_offsets[{{_key}}] = offset;  offset += {{_length}};{{/if}}
    {{/if}}
    {{#if (equals _last_bitfield 1)}}offset += 1;{{/if}}
    {{/param}}
    *frame_len = offset;
    return SL_STATUS_OK;
  } catch(const std::exception& e) {
    sl_log_debug(LOG_TAG, "%s: %s", __func__, e.what());
    return SL_STATUS_FAIL;
  }
}
{{/cmd_type}}

/**
 * Generators for {{_name}}_SET Commands
 */

{{#cmd_type "_SET"}}
static sl_status_t zwave_{{_name}}(
  attribute_store_node_t _node, uint8_t *frame, uint16_t *frame_len)
{
   sl_status_t rc = zwave_{{_name}}_override(_node, frame, frame_len);
   if( *frame_len > 0 ) {
     return rc;
  }  

  try {
    uint8_t offset = 0;
    uint8_t size = 0;
    uint8_t param_offsets[255] = {0}; // To avoid -Wno-maybe-uninitialized
    uint8_t param_key = 0;

    attribute node(_node);
    attribute parent = node.first_parent(ATTRIBUTE_ENDPOINT_ID);

    frame[offset++] = {{_cc_key}} ;
    frame[offset++] = {{_key}}; // {{_name}}
    {{#param}}
    // parameter name: {{_name}} type: {{_type}}
        {{#if has_attr}}
        {{#if att.parameter}}
    /// {{{this}}}
    frame[offset] |= (parent.parent().reported<int32_t>()<< {{_shifter}}) & {{_fieldmask}};
        {{else}}
    node = parent.child_by_type(ATTRIBUTE_{{att.name}});
          {{#if (equals _length 0) }}
    frame[offset] |= (node.desired_or_reported<int32_t>()<< {{_shifter}}) & {{_fieldmask}};
          {{else}}
    node.get(DESIRED_OR_REPORTED_ATTRIBUTE, &frame[offset], &size );
          {{/if}}
        {{/if}}
      {{/if}}
    {{#if _size_key }}
      {{#if (equals _size_key "255") }}
    // Size key: {{_size_key}}, length is determined by message length
    offset += size;
      {{else}}
    frame[ param_offsets[{{_size_key}}] ]  |= (size & {{_size_mask}} );
    param_offsets[{{_key}}] = offset; offset += size;
      {{/if}}
    {{else}}
    {{#if _length }}param_offsets[{{_key}}] = offset;  offset += {{_length}};{{/if}}
    {{/if}}
    {{#if (equals _last_bitfield 1)}}param_offsets[param_key++] = offset; offset += 1;{{/if}}
    {{/param}}
    *frame_len = offset;
    return SL_STATUS_OK;
  } catch(const std::exception& e) {
    sl_log_debug(LOG_TAG, "%s: %s", __func__, e.what());
    return SL_STATUS_FAIL;
  }
}
{{/cmd_type}}

/**
 * Handles incoming {{_name}}_REPORT Commands
 */

{{#cmd_type "_REPORT"}}
static sl_status_t zwave_{{_name}}_handle_report_command(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if( zwave_{{_name}}_handle_report_command_override(connection_info,frame,frame_length) == SL_STATUS_OK)  {
    return SL_STATUS_OK;
  }

  try {
    // Find out the unid of the sending node:
    unid_t supporting_node_unid;
    attribute node;

    zwave_unid_from_node_id(connection_info->remote.node_id,
                            supporting_node_unid);

    // Get the corresponding Attribute Store entry for this unid / endpoint:
    attribute parent
      = attribute_store_network_helper_get_endpoint_node(
        supporting_node_unid,
        connection_info->remote.endpoint_id);

    int offset = 2;
    int param_offsets[255] = {0}; // To avoid -Wno-maybe-uninitialized
    int size = 0;
    int value;

    {{#param}}
    // parameter name: {{_name}}, type: {{_type}}, length: {{_length}}, key: {{_key}},
    // fieldmask: {{_fieldmask}}, shifter: {{_shifter}}
      {{#if has_attr}}
    value = (frame[offset] & {{_fieldmask}}) >> {{_shifter}};
        {{#if att.parameter}}
    parent = parent.child_by_type_and_value<uint8_t>(ATTRIBUTE_{{att.name}}, value);
    if(!parent.is_valid()) {
      sl_log_debug(LOG_TAG, "Unable to locate {{att.name}} attribute with value %i.", value);
    }
        {{else}}
    node = parent.child_by_type(ATTRIBUTE_{{att.name}});
            {{#if _size_key }}
              {{#if (equals _size_key "255") }}
    // Size key: {{_size_key}}, length is determined by message length
    size = frame_length - offset;
              {{else}}
    size = frame[ param_offsets[{{_size_key}}] ]&{{_size_mask}};
              {{/if}}
            {{else}}
    size = {{_length}};
           {{/if}}
    if( !node.is_valid() ) {
      sl_log_debug(LOG_TAG,"Unable to locate {{att.name}} attribute (may not be a problem).");
    } else {
      //Check if there is more data
    {{#if (equals _length "-1") }}
    if( (offset + size) > frame_length) {
      std::vector<uint8_t> zeros(size);
      std::fill (zeros.begin(),zeros.begin()+size,0);
      node.set_reported(zeros);
    }
    {{else}}
    if( (offset + {{_length}}) > frame_length) {
      node.set_reported<int32_t>(0);
    }
    {{/if}}
    else { // If we sill have frame data
    {{#if (equals _length 0) }} node.set_reported<int32_t>(value);{{/if}}
    {{#if (equals _length "1") }} node.set_reported<int32_t>(value);{{/if}}
    {{#if (equals _length "2") }} value = (frame[offset] << 8) | (frame[offset+1]);
      node.set_reported<int32_t>(value);{{/if}}
    {{#if (equals _length "3") }} value = (frame[offset]) << 16 | (frame[offset+1] << 8) | (frame[offset+2]);
      node.set_reported<int32_t>(value);{{/if}}
    {{#if (equals _length "4") }} value = (frame[offset]) << 24 | (frame[offset+1] << 16) | (frame[offset+2] << 8) | (frame[offset+3]);
      node.set_reported<int32_t>(value);{{/if}}
    {{#if (equals _length "-1") }} node.set(REPORTED_ATTRIBUTE, &frame[offset], size );{{/if}}
    }
    node.clear_desired();
    }
        {{/if}}
      {{/if}}
    param_offsets[{{_key}}] = offset;{{#if (equals _last_bitfield 1)}} offset += 1;{{else}} offset += size;{{/if}}
    {{/param}}

    return SL_STATUS_OK;
  } catch(const std::exception& e) {
    sl_log_debug(LOG_TAG, "%s: %s", __func__, e.what());
    return SL_STATUS_FAIL;
  }
}
{{/cmd_type}}

sl_status_t zwave_{{_name}}_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 2) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[1]) {
    {{#cmd_type "_REPORT"}}
    case {{_key}}: //{{_name}}
      return zwave_{{_name}}_handle_report_command(
        connection_info,
        frame_data,
        frame_length);
      break;
    {{/cmd_type}}
    default:
      return SL_STATUS_NOT_SUPPORTED;
      break;
  }
}

/**
 * Function that detects a new {{_name}} and
 * creates the required Attributes for the {{_name}}
 * sub-component.
 */
static void {{_name}}_on_version_attribute_update(
  attribute_store_node_t _updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  zwave_cc_version_t version      = 0;
  attribute updated_node(_updated_node);
  attribute endpoint_node = updated_node.first_parent(ATTRIBUTE_ENDPOINT_ID);


  if (is_zwave_command_class_filtered_for_root_device(
        {{_key}}, _updated_node) == true) {
    return;
  }

  try {
  version = updated_node.reported<zwave_cc_version_t>();
  } catch(std::invalid_argument& e) {
    return;
  }

  assert(ATTRIBUTE_{{_name}}_VERSION == updated_node.type());

  {{#each attribute}}
  {{#if _parameter }}
  //Skipped {{_name}}
  {{else}}
    {{#if _parent}}
      // parent {{_parent}}
    {{else}}
  if( version >= {{_version}} )
  {
    attribute value_node = endpoint_node.child_by_type(ATTRIBUTE_{{_name}});
    if( !value_node.is_valid() ) {
      endpoint_node.add_node(ATTRIBUTE_{{_name}});
    }
  }
  {{/if}}
  {{/if}}
  {{/each}}
}


///////// Helper functions to create child attributes

{{#each attribute}}
{{#if _parameter }}
void zwave_{{ ../_name }}_add_{{_name}}( attribute_store_node_t __parent, uint8_t val ) {
  attribute parent(__parent);
  attribute ep = parent.first_parent( ATTRIBUTE_ENDPOINT_ID );
  attribute version = ep.child_by_type(ATTRIBUTE_{{../_name}}_VERSION);
  attribute node = parent.add_node( ATTRIBUTE_{{_name}} );

  node.set_reported<uint8_t>(val);

  {{#attr_children _name }}
  if( version.reported<uint8_t>() >= {{_version}} )
  {
    attribute value_node = node.child_by_type(ATTRIBUTE_{{_name}});
    if(! value_node.is_valid()) {
      node.add_node(ATTRIBUTE_{{_name}});
    }
  }
  {{/attr_children}}
}
{{/if}}
{{/each}}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////

sl_status_t zwave_{{_name}}_init()
{
  {{#each attribute}}
  {{#if _parameter}}
  // ATTRIBUTE_{{_name}} skipped because its a parameter
  {{else}}
  {{#if _get_func}}
  attribute_resolver_register_rule(ATTRIBUTE_{{_name}},
    {{#if _set_func}}zwave_{{_set_func}}{{else}}NULL{{/if}}, zwave_{{_get_func}}
  );
  {{/if}}
  {{/if}}
  {{/each}}

  attribute_store_register_callback_by_type(
    {{_name}}_on_version_attribute_update,ATTRIBUTE_{{_name}}_VERSION);

  // The support side of things: Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {0};
  handler.support_handler         = nullptr;
  handler.control_handler         = zwave_{{_name}}_handler;
  handler.command_class_name      = "{{human_readable_cc _help}}";
  handler.comments                = "Control Part is auto-generated.";
  // Not supported, so this does not really matter
  handler.minimal_scheme = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class                     = {{_key}};
  handler.version                           = {{_name}}_VERSION;
  handler.manual_security_validation        = false;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}