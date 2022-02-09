// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
#![doc(html_no_source)]

use futures::stream::LocalBoxStream;
use unify_middleware::attribute_stream_ext::ForceUpdateAttributeStream;
use unify_middleware::{Attribute, AttributeEvent};

pub trait AttributeStreamExt {
    /// Creates an attribute store stream similar to the AttributeChanged
    /// Stream. It will stream all current attributes in the attribute store on
    /// startup as AttributeEventType::ATTRIBUTE_UPDATED events before
    /// continuing as a normal AttributeChangedStream.
    fn attribute_changed_replayed(
        predicate: impl 'static + Fn(&AttributeEvent) -> bool,
    ) -> LocalBoxStream<'static, AttributeEvent>;
}

impl AttributeStreamExt for Attribute {
    fn attribute_changed_replayed(
        predicate: impl 'static + Fn(&AttributeEvent) -> bool,
    ) -> LocalBoxStream<'static, AttributeEvent> {
        Box::pin(ForceUpdateAttributeStream::new(predicate))
    }
}
