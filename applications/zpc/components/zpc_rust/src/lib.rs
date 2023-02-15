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
#![doc(html_no_source)]

//! This rust module is part of the ZPC binary. It is build as a standalone
//! static library and linked into the ZPC by the cmake build system. This means
//! that this crate **cannot** provide any linkage to any native libraries. This
//! is intended! This also implies that unit-test **cannot call C functions**.
//! Do not try to make it work by adding link arguments or build.rs link
//! definitions!! Calls towards native functions should be decoupled from the
//! rust code with an trait interface, which can be mocked instead in unit-test
//! scenarios. The C bindings are accessible within the whole crate because we
//! are behind on abstracting them away.
//!
//! # Decoupling C bindings
//!
//! dont do this:
//! ```rust, ignore
//! extern "C" {
//!     fn foo_function();
//! }
//!
//! struct Bar;
//! impl Bar{
//!     fn very_complex_logic(&self){
//!         ..
//!         foo_function();
//!     }
//!
//! }
//! ```
//!
//! Instead, abstract away the bindings by putting it behind a trait interface.
//!
//! ```rust, no_run
//! extern "C" {
//!   fn foo_function();
//! }
//!
//! #[cfg_attr(test, mockall::automock)] // -> generates a mock object of the trait interface when build for test
//! trait FooTrait {
//!   fn foo(&self);
//! }
//!
//! struct ConcreteFoo;
//! impl FooTrait for ConcreteFoo {
//!   fn foo(&self) {
//!       unsafe {
//!           foo_function();
//!       }
//!   }
//! }
//!
//! struct Bar;
//! impl Bar {
//!  /// pass trait as argument. we are not calling the C binding directly, but its now behind the FooTrait.
//!  /// This way you can very easy test the Bar object by just passing a `MockFooTrait`
//!  fn very_complex_logic(&self, foo_object: &dyn FooTrait){
//!         foo_object.foo();
//!     }
//! }
//! ```

unify_tools::include_binding!(
    concat!(env!("OUT_DIR"), "/zwave_controller.rs"),
    zwave_controller_sys
);
use zwave_controller_sys::zwave_controller_connection_info_t;
use zwave_controller_sys::zwave_controller_encapsulation_scheme_t;
unify_tools::include_binding!(
    concat!(env!("OUT_DIR"), "/zwave_command_handler.rs"),
    zwave_command_handler_sys
);
unify_tools::include_binding!(concat!(env!("OUT_DIR"), "/zwave_tx.rs"), zwave_tx_sys);
unify_tools::include_binding!(
    concat!(env!("OUT_DIR"), "/zwave_command_classes.rs"),
    zwave_command_classes_sys
);

pub mod rust_command_handlers;
mod zpc_attribute_store;
mod zwave_poll_manager;

use rust_command_class_frame_types::*;
use rust_command_handlers::*;
use zwave_command_class_firmware_update;

zwave_rust_proc_macros::extern_c_hook!();
zwave_rust_proc_macros_legacy::initialize_zwave_command_classes!();
