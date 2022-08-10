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

use unify_middleware::Attribute;

/// Implementers of this trait interface will receive a call of
/// [ZwaveVersionChangedTrait::on_attribute_version] every time a change is
/// detected inside the attribute store of the defined type
/// [ZwaveVersionChangedTrait::ATTRIBUTE_STORE_TYPE] Secondly, the macro
/// '#[zwave_command_class](macro@zwave_rust_proc_macros::zwave_command_class)`
/// is required to successfully attach the trait interface into the attribute
/// store system.
///
/// >This component assumes the attribute store is initialized.
///
/// # Example
///
/// ```rust
/// # use zwave_rust_proc_macros::*;
/// # use unify_middleware::*;
/// # use unify_log_sys::*;
/// # static APP_NAME : &'static str = "test";
/// # use zpc_rust::rust_command_handlers::ZwaveVersionChangedTrait;
/// struct FooCommandClass;
///
/// #[zwave_command_class] // <-- zwave_command_class macro
/// impl ZwaveVersionChangedTrait for FooCommandClass {
///     const ATTRIBUTE_STORE_TYPE: u32 = 1234;
///
///     fn on_attribute_version(ep: Attribute, version: u8) {
///         println!("version of attribute {} changed!", ep);
///     }
/// }
/// ```
pub trait ZwaveVersionChangedTrait {
    /// attribute store type id of an attribute store attribute
    const ATTRIBUTE_STORE_TYPE: u32;
    /// will be called when version changes
    fn on_attribute_version(ep: Attribute, version: u8);
}
