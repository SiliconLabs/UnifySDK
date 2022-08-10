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

use super::*;
use core::any::Any;
use futures::stream::LocalBoxStream;
use mockall::mock;
use serde::de::DeserializeOwned;
use serde::Serialize;

mock! {
    #[derive(Debug)]
    pub Attribute {}

    impl AttributeTrait for Attribute {
        fn invalid() -> Self;
        fn handle(&self) -> AttributeStoreHandle;
        fn valid(&self) -> bool;
        fn is_reported_set(&self) -> bool;
        fn is_desired_set(&self) -> bool;
        fn is_value_matched(&self) -> bool;
        fn set_reported_as_desired(&self) -> bool;
        fn set_desired_as_reported(&self) -> bool;
        fn start_transition(&self, duration: u32);
        fn stop_transition(&self);
        fn delete(self) -> Result<(), AttributeStoreError>;
        fn type_of(&self) -> AttributeTypeId;
        fn type_name(&self) -> String;
        fn child_count(&self) -> usize;
        fn child(&self, index: usize) -> Self;
        fn parent(&self) -> Self;
        fn get_first_parent_with_type(&self, type_id: AttributeTypeId) -> Option<Self>;
        fn desired_changed<T>(
            &self,
        ) -> LocalBoxStream<'static, Result<(T, AttributeEventType), AttributeStoreError>>
        where
            T: 'static + DeserializeOwned + Any + Unpin;
        fn reported_changed<T>(
            &self,
        ) -> LocalBoxStream<'static, Result<(T, AttributeEventType), AttributeStoreError>>
        where
            T: 'static + DeserializeOwned + Any + Unpin;
        fn get_desired_or_reported<T: 'static + DeserializeOwned + Any>(
            &self,
        ) -> Result<T, AttributeStoreError> {
            self.get_desired().or_else(|_| self.get_reported())
        }
        fn get_desired<T: 'static + DeserializeOwned + Any>(&self) -> Result<T, AttributeStoreError>;
        fn get_reported<T: 'static + DeserializeOwned + Any>(&self) -> Result<T, AttributeStoreError>;
        fn set_reported<T: Serialize + Any>(&self, value: Option<T>)
            -> Result<(), AttributeStoreError>;
        fn set_desired<T: Serialize + Any>(&self, value: Option<T>) -> Result<(), AttributeStoreError>;
        fn iter(&self) -> AttributeStoreIterator<Self>;
        fn iter_dfs(&self) -> AttributeStoreIteratorDFS<Self>;
        fn add<T: Serialize + Any>(
            &self,
            type_id: AttributeTypeId,
            reported: Option<T>,
            desired: Option<T>,
        ) -> Result<Self, AttributeStoreError>;
    }

    impl Clone for Attribute {
        fn clone(&self) -> Self;
    }
}
