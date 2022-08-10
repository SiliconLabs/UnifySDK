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

use crate::{zwave_command_classes_sys::sl_status_t, rust_command_handlers::zwave_command_class_definitions::ConnectionInfoTrait};
use unify_sl_status_sys::SL_STATUS_NOT_SUPPORTED;
/// The [FrameControlSupportTrait] exposes trait for implementing logic handling
/// support and control of frames. This trait should be used for implementing
/// the logic on specific frames.
pub trait FrameControlSupportTrait {
    #[allow(unused_variables)]
    fn on_support(&self, info: &dyn ConnectionInfoTrait) -> Result<(), sl_status_t> {
        Err(SL_STATUS_NOT_SUPPORTED)
    }

    #[allow(unused_variables)]
    fn on_control(&self, info: &dyn ConnectionInfoTrait) -> Result<(), sl_status_t> {
        Err(SL_STATUS_NOT_SUPPORTED)
    }
}
