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

use crate::rust_command_handlers::{
    zwave_command_class_definitions::ConnectionInfoTrait,
    zwave_command_class_definitions::ZwaveControllerEncapsulationScheme,
};
use crate::zwave_command_classes_sys::zwave_command_class_get_endpoint_node;
use crate::zwave_controller_sys::zwave_controller_connection_info;
use crate::zwave_controller_sys::zwave_controller_endpoint_t;
use unify_log_sys::*;
use unify_middleware::AttributeTrait;
use unify_middleware::{attribute_store_or_return_with, Attribute};
declare_app_name!("connection_info");

impl ConnectionInfoTrait for zwave_controller_connection_info {
    fn get_local(&self) -> zwave_controller_endpoint_t {
        self.local
    }

    fn get_remote(&self) -> zwave_controller_endpoint_t {
        self.remote
    }

    fn get_encapsulation(&self) -> ZwaveControllerEncapsulationScheme {
        self.encapsulation.into()
    }

    fn get_endpoint(&self) -> Attribute {
        unsafe {
            let ep_node = zwave_command_class_get_endpoint_node(
                self as *const zwave_controller_connection_info,
            );

            attribute_store_or_return_with!(Attribute::invalid()).from_handle(ep_node)
        }
    }
}
