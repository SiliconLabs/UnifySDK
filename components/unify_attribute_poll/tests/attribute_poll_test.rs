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

use unify_attribute_poll::attribute_poll_trait::AttributePollTrait;
use unify_attribute_poll::{AttributePoll, PollEngineConfig};
use unify_middleware::{Attribute, AttributeStoreTrait};

#[test]
fn test_something() {
    let attribute_poll = AttributePoll::default();
    attribute_poll.initialize(PollEngineConfig {
        backoff: 30,
        default_interval: 60,
    });
    let root = Attribute::root().unwrap();
    let child = root.add(1, Some(1234u32), Some(42u32)).unwrap();
    attribute_poll.register(child.clone(), 100);
}
