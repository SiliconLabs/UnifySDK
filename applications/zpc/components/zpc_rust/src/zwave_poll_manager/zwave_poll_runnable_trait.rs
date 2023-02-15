use super::zwave_poll_config::AttributePollMap;
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
////////////////////////////////////////////////////////////////////

/// This trait abstracts away the running of a poll register requiring a
/// attribute poll map
#[cfg_attr(test, mockall::automock)]
pub trait PollRunnableTrait {
    /// this function takes self. meaning self is moved and does not exist
    /// anymore after the function run is called. Secondly, this call is none
    /// blocking
    ///
    /// # Arguments
    ///
    /// * `poll_map`    map containing poll timeout given different attribute
    /// type_id's
    fn run(self, poll_map: AttributePollMap);
}
