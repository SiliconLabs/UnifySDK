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
use crate::unify_attribute_store::attribute_trait::AttributeTrait;
use crate::{
    attribute_store_or_return_with,
    contiki::{contiki_spawn, contiki_spawn_on_timeout},
    Attribute, AttributeChangedStream, AttributeEvent, AttributeStoreType, AttributeTypeId,
    AttributeValueState,
};
use futures::stream::LocalBoxStream;
use futures::StreamExt;
use std::time::Duration;

/// Await a change from the attribute store for a given predicate.
///
/// # Arguments
///
/// * `predicate`   -   Define the criteria for when to trigger an
///   attribute_change.
///
/// # Returns
///
/// A future that returns tuple with the change attribute + eventtype of that
/// change The future returns an error in case there was an internal error or
/// the attribute change callbacks could not be registered.
///
/// # Examples
///
/// ```rust, no_run
///# async fn wrapper_func() {
///#     use unify_middleware::contiki::*;
///#     use unify_middleware::unify_attribute_store::*;
///#     use futures::stream::StreamExt;
/// let added_predicate = |change: &AttributeEvent<Attribute>| {
///     change.event_type == AttributeEventType::ATTRIBUTE_CREATED
/// };
/// let mut added_stream = attribute_changed(added_predicate);
/// if let Some(attr) = added_stream.next().await {
///     println!("attribute {:?} added!", attr);
/// }
/// # use unify_log_sys::*;
/// # declare_app_name!("test");
/// # use unify_middleware::attribute_store_or_return_with;
/// # let attribute_store = attribute_store_or_return_with!();
/// let root_changed =
///     |change: &AttributeEvent<Attribute>| change.attribute == attribute_store.root();
/// let mut root_change_stream = attribute_changed(root_changed);
/// if let Some(_) = root_change_stream.next().await {
///     println!("change of root!");
/// }
///# }
/// ```
pub fn attribute_changed(
    predicate: impl Fn(&AttributeEvent<Attribute>) -> bool + 'static,
) -> LocalBoxStream<'static, AttributeEvent<Attribute>> {
    fn conversion(event: AttributeEvent<Attribute>) -> AttributeEvent<Attribute> {
        event
    }

    Box::pin(AttributeChangedStream::new(Box::new(predicate), conversion))
}

/// Calls the given callback after a certain duration. on a callback dispatch,
/// an reference to the attribute store is passed as convenience This function
/// is very similar to [contiki_spawn_on_timeout], it sets up additional boiler
/// plate for convenience.
/// >When the attribute store is not initialized, the callback will be omitted!
pub fn attribute_on_timeout<F>(timeout: Duration, callback: F)
where
    F: 'static + FnOnce(&AttributeStoreType),
{
    use unify_log_sys::*;
    declare_app_name!("attribute_timeout");
    let task = async move {
        callback(attribute_store_or_return_with!());
    };
    contiki_spawn_on_timeout(timeout, task)
}

// private helper macro to setup an attribute changed task. DRY!
macro_rules! attribute_changed_task {
    ($cb: ident, $pred: expr) => {
        let task = async move {
            let mut attribute_changes = attribute_changed($pred).fuse();

            loop {
                let event = attribute_changes.select_next_some().await;
                $cb(&event);
            }
        };

        contiki_spawn(task);
    };
}

/// Generic callback called when an attribute inside the attribute store
/// changed. This function is intended for functional programming. i.e. in
/// state-less scenario's, the callbacks are self contained and dont have
/// borrows outside of its scope. In the other scenarios use [attribute_changed]
/// instead.
///
/// # Arguments
///
/// * `callback`    function or capture that takes an `&Attribute` as argument
pub fn attribute_callback<F>(callback: F)
where
    F: 'static + Fn(&AttributeEvent<Attribute>),
{
    attribute_changed_task!(callback, |_| true);
}

/// callback called when an attribute for a given type inside the attribute
/// store changed.
///
/// # Arguments
///
/// * `callback`    function or capture that takes an `&Attribute` as argument
pub fn attribute_callback_by_type<F>(typ: AttributeTypeId, callback: F)
where
    F: 'static + Fn(&AttributeEvent<Attribute>),
{
    attribute_changed_task!(callback, move |event| event.attribute.type_of() == typ);
}

/// callback called when an attribute for a given type and value state inside
/// the attribute store changed.
///
/// # Arguments
///
/// * `callback`    function or capture that takes an `&Attribute` as argument
pub fn attribute_callback_by_type_and_state<F>(
    attribute_type: AttributeTypeId,
    value_state: AttributeValueState,
    callback: F,
) where
    F: 'static + Fn(&AttributeEvent<Attribute>),
{
    attribute_changed_task!(callback, move |event| event.attribute.type_of()
        == attribute_type
        && event.value_state == value_state);
}
