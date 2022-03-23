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

pub use zwave_controller_sys::zwave_controller_connection_info;
pub use zwave_controller_sys::zwave_controller_connection_info_t;
unify_tools::include_binding!(zwave_tx);

pub use zwave_tx::*;

pub mod zwave_tx_sys {
    pub use super::*;

    pub fn send_data(
        connection: zwave_controller_sys::zwave_controller_connection_info_t,
        data: &[u8],
        tx_options: zwave_tx_options_t,
        on_send_complete: on_zwave_tx_send_data_complete_t,
        user: *mut ::std::os::raw::c_void,
        session: *mut zwave_tx_session_id_t,
    ) -> sl_status_t {
        unsafe {
            zwave_tx_send_data(
                &connection as *const zwave_controller_sys::zwave_controller_connection_info_t,
                data.len() as u16,
                data.as_ptr(),
                &tx_options as *const zwave_tx_options_t,
                on_send_complete,
                user,
                session,
            )
        }
    }
}
