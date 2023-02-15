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
use unify_middleware::{AttributeStore, AttributeStoreTrait, AttributeTrait, AttributeTypeId};

#[test]
fn test_something() {
    let attribute_store = AttributeStore::new().unwrap();
    let attribute_poll = AttributePoll::default();

    const POLL_MARK_TYPE: AttributeTypeId = 0x18;
    attribute_poll.initialize(PollEngineConfig {
        backoff: 30,
        default_interval: 60,
        poll_mark_attribute_type: POLL_MARK_TYPE,
    });
    let root = attribute_store.root();
    let child = root.add(1, Some(1234u32), Some(42u32)).unwrap();
    attribute_poll.register(child.clone(), 100);
}
