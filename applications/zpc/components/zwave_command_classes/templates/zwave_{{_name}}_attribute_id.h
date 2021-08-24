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

#ifndef ZWAVE_{{_name}}_ATTRIBUTE_ID_H
#define ZWAVE_{{_name}}_ATTRIBUTE_ID_H


{{#each attribute}}
DEFINE_ATTRIBUTE(ATTRIBUTE_{{_name}}, (({{../_key}}<< 8) | {{_key}}))
{{/each}}


#endif  //ZWAVE_{{_name}}_ATTRIBUTE_ID_H

