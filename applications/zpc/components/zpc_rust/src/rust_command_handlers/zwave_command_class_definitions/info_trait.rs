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

pub type CommandClassVersion = u8;
pub type CommandClassType = u16;

/// The [ZwaveCommandClassInfo] is used for auto generating interfaces for quick
/// implementations of CCs. This trait should normally not be used for manual
/// implementation.
pub trait ZwaveCommandClassInfo {
    const VERSION: CommandClassVersion;
    const COMMAND_CLASS_TYPE_ID: CommandClassType;
}
