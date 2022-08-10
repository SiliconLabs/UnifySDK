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

//! This module contains a list of traits a Command Class (CC) could implement to enable a certain piece of functionality.
//!
mod configuration_trait;
mod frame_trait;
mod version_changed_trait;

#[doc(inline)]
pub use configuration_trait::*;
#[doc(inline)]
pub use frame_trait::*;
#[doc(inline)]
pub use version_changed_trait::*;
