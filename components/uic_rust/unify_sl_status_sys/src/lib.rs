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
// * exposes sl status values scraped from the sl_status.h file
// * exposes an function to get the textual representation of the status
include!(concat!(env!("OUT_DIR"), "/sl_status.rs"));

/* EXAMPLE:
pub const SL_STATUS_OK: u32 = 0x0000;
pub const SL_STATUS_FAIL: u32 = 0x0001;
pub const SL_STATUS_INVALID_STATE: u32 = 0x0002;
pub const SL_STATUS_NOT_READY: u32 = 0x0003;
pub const SL_STATUS_BUSY: u32 = 0x0004;
pub const SL_STATUS_IN_PROGRESS: u32 = 0x0005;
pub const SL_STATUS_ABORT: u32 = 0x0006;
pub const SL_STATUS_TIMEOUT: u32 = 0x0007;
....

pub fn sl_status_to_string(val: u32) -> &'static str {
*/