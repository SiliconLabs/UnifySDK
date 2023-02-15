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
use crate::{Attribute, AttributeStore, AttributeStoreError, AttributeStoreTrait};
/// private helper macro to easily switch between an concrete and mock
/// attributestore. It creates an function with the following signature:
/// ```rust, ignore
///   pub fn attribute_store_instance() -> Result<&'static AttributeStoreType, AttributeStoreError>;
/// ```
///
/// # Arguments
///
/// * `instance_type` attribute store type.
/// * `attribute_type` which attribute types it will use
/// * `instantiation`   how to instantiate the `instance_type`
///  
macro_rules! setup_attribute_store_instance {
    ($instance_type: ty, $attribute_type:ty, $instantiation:expr) => {
        pub type AttributeStoreType =  dyn AttributeStoreTrait<$attribute_type>;

        pub fn attribute_store_instance() -> Result<
            &'static AttributeStoreType,
            AttributeStoreError,
        > {
            use once_cell::sync::Lazy;
            static ATTRIBUTE_STORE: Lazy<Result<$instance_type, AttributeStoreError>> =
                Lazy::new(|| $instantiation);
            match &*ATTRIBUTE_STORE {
                Ok(store) => Ok(store as &AttributeStoreType),
                Err(e) => Err(e.clone()),
            }
        }
    };
}

#[cfg(not(test))]
setup_attribute_store_instance!(AttributeStore, Attribute, AttributeStore::new());
#[cfg(test)]
setup_attribute_store_instance!(
    crate::MockAttributeStoreTrait<crate::attribute_mock::MockAttribute>,
    crate::attribute_mock::MockAttribute,
    Ok(crate::MockAttributeStoreTrait::<
        crate::attribute_mock::MockAttribute,
    >::new())
);

/// This macro returns a reference to the attribute store instance. If the
/// attribute store is not instantiated it will return with the given expression
/// which is passed as argument. e.g.
/// ```rust
/// # use unify_log_sys::*;
/// # declare_app_name!("test");
/// # use unify_middleware::attribute_store_or_return_with;
/// fn foo() -> bool {
/// let attribute_store = attribute_store_or_return_with!(false);
/// return true;
/// }
/// ```
/// will exit the current function with `false`, otherwise it will assign a
/// borrow to the attribute store to `attribute_store`.
///
/// this expression can be anything:
/// ```rust
/// # use unify_log_sys::*;
/// # declare_app_name!("test");
/// # use unify_middleware::attribute_store_or_return_with;
/// fn arbitrary_function() -> String {
///     let attribute_store = attribute_store_or_return_with!(String::new());
///     "foo_bar".to_string()
/// }
/// ```
/// will return an `String` object in the case the attribute store is not
/// initialized.
///
/// >this function also accepts no arguments.
#[macro_export]
macro_rules! attribute_store_or_return_with {
    ($exit:expr) => {
        match $crate::unify_attribute_store::attribute_store_instance() {
            Ok(store) => store,
            Err(_) => {
                unify_log_sys::log_error!("attribute store not initialized");
                return $exit;
            }
        }
    };

    () => {
        attribute_store_or_return_with!(())
    };
}
