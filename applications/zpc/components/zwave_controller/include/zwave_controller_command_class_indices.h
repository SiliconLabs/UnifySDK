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

#ifndef ZWAVE_CONTROLLER_COMMAND_CLASS_INDICES_H
#define ZWAVE_CONTROLLER_COMMAND_CLASS_INDICES_H

// First Command Class identifier for which the Command Class uses 2 bytes.
#define EXTENDED_COMMAND_CLASS_IDENTIFIER_START 0xF0
#define COMMAND_CLASS_CONTROL_MARK              0xEF
#define COMMAND_CLASS_FIRST_APPLICATION_CC      COMMAND_CLASS_BASIC

// Generic command classes defines for parsing incoming frames
#define COMMAND_CLASS_INDEX 0
#define COMMAND_INDEX       1

#define ZWAVE_NOP_COMMAND_CLASS         0x00
#define ZWAVE_PROTOCOL_COMMAND_CLASS    0x01
#define ZWAVE_NODE_INFO_REQUEST_COMMAND 0x02

#endif  // ZWAVE_CONTROLLER_COMMAND_CLASS_INDICES_H
