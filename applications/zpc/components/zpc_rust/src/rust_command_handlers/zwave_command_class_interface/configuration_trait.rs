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

use crate::{
    rust_command_handlers::zwave_command_class_definitions::ZwaveControllerEncapsulationScheme,
    zwave_controller_sys::sl_status_t,
};

use super::FrameControlSupportTrait;

/// This trait is mandatory to be implemented for a given command class. It will
/// setup and register your command class into the system. Note that this trait
/// *requires to be accompanied by the
/// [#\[zwave_command_class\]](macro@zwave_rust_proc_macros::zwave_command_class)!!*
///
/// ```rust, ignore
/// #[zwave_command_class] // <---- dont forget macro
/// impl CommandClassConfigurationTrait for FooFrame {
/// ...
/// }
/// ```
pub trait CommandClassConfigurationTrait: FrameControlSupportTrait {
    /// Is only used for the support handle logic. The scheme indicates the
    /// minimum level of security the node will accept to support a command
    /// class.
    const MINIMAL_SCHEME: ZwaveControllerEncapsulationScheme;
    /// Name of the Command Class
    const COMMAND_CLASS_NAME: &'static str;
    /// Adds the possibility to make a comment for a CC in case of special
    /// quirks or similar. These comments are used in logging
    const COMMENTS: &'static str;
    /// enables the possibility to do non-secure operations in a secure network
    /// inclusion for special cases.
    const MANUAL_SECURITY_VALIDATION: bool;
    /// boolean that describes wether a `on_support()` function of the
    /// [FrameControlSupportTrait] was implemented for one of the command
    /// classes associated frames. This information is needed at compile time so
    /// the correct code can be generated.
    const INCLUDE_SUPPORT: bool = false;
    /// boolean that describes wether a `on_control()` function of the
    /// [FrameControlSupportTrait] was implemented for one of the command
    /// classes associated frames. This information is needed at compile time so
    /// the correct code can be generated.
    const INCLUDE_CONTROL: bool = false;

    /// # Return
    ///
    ///  SL_STATUS_OK on success, any other error code for an error.
    fn on_init() -> sl_status_t;
}
