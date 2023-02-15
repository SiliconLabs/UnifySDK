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

use crate::zwave_controller_sys::zwave_controller_endpoint_t;
use unify_middleware::Attribute;

use super::ZwaveControllerEncapsulationScheme;

/// The [ConnectionInfoTrait] is a collection of getters for extracting
/// information out of a connection information data type. This allows for
/// example extracting an endpoint easily for the command class implementors
/// with [ConnectionInfoTrait::get_endpoint].
#[cfg_attr(test, mockall::automock)]
pub trait ConnectionInfoTrait {
    fn get_local(&self) -> zwave_controller_endpoint_t;
    fn get_remote(&self) -> zwave_controller_endpoint_t;
    fn get_encapsulation(&self) -> ZwaveControllerEncapsulationScheme;
    fn get_endpoint(&self) -> Attribute;
}
