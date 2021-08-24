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

#ifndef ZWAVE_{{_name}}_HANDLERS_H
#define ZWAVE_{{_name}}_HANDLERS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "attribute_store.h"
#include "sl_status.h"

/**
 * @defgroup {{_name}}_handler Command handlers for {{_name}}
 * @ingroup command_classes
 * @brief Functions for controlling {{_name}}
 * 
 * This module implement functions for generating and parsing the Z-Wave frames
 * for controlling {{_name}}. The module builds frames by reading data from the
 * attribute according to the tree data model shown below. When data is received 
 * is will also update the attribute store with data again according the the model 
 * described below.
 * 
 * In addition the module will react on the on_version update events from the
 * attribute store to and insert required attributes into to attribute store. A 
 * set of helper functions  for inserting attribute for more complex data is also 
 * provided. The complex data are generally data dependign on some sort of index.
 *
 * 
@dot
digraph G {
  rankdir=LR;
  splines="ortho";
  {{#each attribute}}
  ep [ label="endpoint", shape="box"]
  {{_name}} [ label="{{_name}}\nV{{_version}}\nget:{{_get_func}}", shape="ellipse" ]
  {{/each}}

  {{#each attribute}}
  {{#if _parent}}
  {{_parent}} -> {{_name}} 
  {{else}}
  ep -> {{_name}}
  {{/if}}
  {{/each}}
}
@enddot

@{
*/


/**
 * @brief Version of {{_name}} which this handlers is genrated for
 * 
 */
#define {{_name}}_VERSION {{_version}}

{{#each attribute}}
{{#if _parameter }}
/**
 * @brief Add {{_name}} and its mandatory children to a parent
 * 
 * This function will add an attribute of type {{_name}} under the given parent
 * attribute. The value of the new attribute set to index, and the manetory
 * children are also created under the new attribute, considering the commad
 * class version.
 * 
 * @param parent attribute store node to add children to.
 * @param index Index of the new attribute subtree to created
 */
void zwave_{{ ../_name }}_add_{{_name}}( attribute_store_node_t parent, uint8_t index );
{{/if}}
{{/each}}

/**
 * @brief Initialisation of {{_name}}
 * 
 * This function registers the command handler with the attribute resolver 
 * and the command handler framework.
 * 
 */
sl_status_t zwave_{{_name}}_init();


{{#cmd_type "_REPORT"}}
/**
 * @brief Override function for {{_name}}
 * 
 * This if this function is implemented as a weak symbol. This provided
 * such an external component can modifiy the behaviour of the generated 
 * code.
 * 
 * @return If not SL_STATUS_OK the auto generated hander is executed after this.
 */
sl_status_t  zwave_{{_name}}_handle_report_command_override(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame,
  uint16_t frame_length) ;
{{/cmd_type}}

{{#cmd_type "_GET"}}
/**
 * @brief Override function for {{_name}}
 * 
 * This if this function is implemented as a weak symbol. This provided
 * such an external component can modifiy the behaviour of the generated 
 * code.
 * 
 * @return If not SL_STATUS_OK the auto generated hander is executed after this.
 */

sl_status_t zwave_{{_name}}_override(
  attribute_store_node_t  _node, uint8_t *frame, uint16_t *frame_len);
{{/cmd_type}}

{{#cmd_type "_SET"}}
/**
 * @brief Override function for {{_name}}
 * 
 * This if this function is implemented as a weak symbol. This provided
 * such an external component can modifiy the behaviour of the generated 
 * code.
 * 
 * @return If not SL_STATUS_OK the auto generated hander is executed after this.
 */

sl_status_t zwave_{{_name}}_override(
  attribute_store_node_t _node, uint8_t *frame, uint16_t *frame_len);
{{/cmd_type}}


#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_HANDLERS_{{_name}}_H
/** @} end zwave_handlers.h */

