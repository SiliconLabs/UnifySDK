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

//! C interface for the Attribute Poll Engine wrapping (inlcude/attribute_poll.h)

use unify_middleware::attribute_store_or_return_with;

use super::*;
use crate::attribute_poll_trait::AttributePollTrait;
use unify_log_sys::*;

declare_app_name!("poll_engine");

#[no_mangle]
pub unsafe extern "C" fn attribute_poll_register(
    handle: attribute_store_node_t,
    interval: IntervalType,
) -> sl_status_t {
    let attribute_store = attribute_store_or_return_with!(SL_STATUS_FAIL);
    AttributePoll::default().register(attribute_store.from_handle(handle), interval);
    SL_STATUS_OK
}

#[no_mangle]
pub unsafe extern "C" fn attribute_poll_deregister(handle: attribute_store_node_t) -> sl_status_t {
    let attribute_store = attribute_store_or_return_with!(SL_STATUS_FAIL);
    AttributePoll::default().deregister(attribute_store.from_handle(handle));
    SL_STATUS_OK
}

#[no_mangle]
pub unsafe extern "C" fn attribute_poll_schedule(handle: attribute_store_node_t) -> sl_status_t {
    let attribute_store = attribute_store_or_return_with!(SL_STATUS_FAIL);
    AttributePoll::default().schedule_now(attribute_store.from_handle(handle));
    SL_STATUS_OK
}

#[no_mangle]
pub extern "C" fn attribute_poll_init() -> sl_status_t {
    // Fixme:Both backoff and default interval config should be parameters of
    // the init function instead of reading from config. (will be fixed in UIC-1526).
    if let Ok(backoff) = unify_config_sys::config_get_as_int("zpc.poll.backoff") {
        if let Ok(default_interval) =
            unify_config_sys::config_get_as_int("zpc.poll.default_interval")
        {
            AttributePoll::default().initialize(PollEngineConfig {
                backoff: backoff as u32,
                default_interval: default_interval as u32,
            });
            return SL_STATUS_OK;
        }
    }
    SL_STATUS_FAIL
}

#[no_mangle]
pub unsafe extern "C" fn attribute_poll_disable() -> sl_status_t {
    AttributePoll::default().pause();
    SL_STATUS_OK
}

#[no_mangle]
pub unsafe extern "C" fn attribute_poll_enable() -> sl_status_t {
    AttributePoll::default().resume();
    SL_STATUS_OK
}
