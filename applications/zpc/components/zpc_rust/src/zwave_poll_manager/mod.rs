///////////////////////////////////////////////////////////////////////////////
// # License
// <b>Copyright 2022  Silicon Laboratories Inc. www.silabs.com</b>
///////////////////////////////////////////////////////////////////////////////
// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
//
///////////////////////////////////////////////////////////////////////////////

//! This module reads yaml configuration file that contains attribute types and
//! its polling interval values in `seconds` as follow: Keys for polling
//! intervals: ... polling_interval_zwave -> represents the interval for Z-Wave
//! node ... polling_interval_zwave_v1 -> represents the interval for Z-Wave
//! Plus v1 node ... polling_interval_zwave_v2 -> represents the interval for
//! Z-Wave Plus v2 node
//!
//! Example registering polling interval for classic Z-Wave and Zwave Plus v1
//! device classes:
//! ```yaml
//!   - attribute_type: ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE
//!     polling_interval_zwave: 100
//!     polling_interval_zwave_v1: 100
//!```
//! Example registering polling interval for all device classes:
//!``` yaml
//!   - attribute_type: ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE
//!     polling_interval_zwave: 100
//!     polling_interval_zwave_v1: 100
//!     polling_interval_zwave_v2: 30
//!```
//!
//! [PollRegister](zwave_poll_register::PollRegister) will watch for included nodes in the attribute store. When a
//! new node is included or gets updated. It will schedule a poll interval for
//! that node. Given the user configured a poll interval for that node
//! matching the zwave plus info version number.
mod zwave_poll_config;
mod zwave_poll_register;
mod zwave_poll_runnable_trait;
