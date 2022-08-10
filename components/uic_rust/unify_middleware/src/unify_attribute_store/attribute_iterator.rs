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

use std::collections::VecDeque;

use crate::verify_attribute_ok;

use super::AttributeTrait;

/// Searches the attribute store tree breadth first.
///
/// # Arguments
/// * `handle`  -   handle to an attribute_store_node_t
///
/// # Returns
/// - Some(Attribute) if there is an next child attribute
/// - None if the end of the tree is reached.
///
/// If an ATTRIBUTE_STORE_INVALID_NODE is received from the C-API
/// None is returned.
#[derive(Debug, Clone, PartialEq)]
pub struct AttributeStoreIterator<T: AttributeTrait> {
    current_index: usize,
    queue: VecDeque<T>,
}

impl<T: AttributeTrait> AttributeStoreIterator<T> {
    pub fn new(attribute: T) -> Self {
        let mut queue = VecDeque::new();
        queue.push_back(attribute);

        Self {
            current_index: 0,
            queue,
        }
    }
}

#[mockall::automock]
impl<T> Iterator for AttributeStoreIterator<T>
where
    T: 'static + AttributeTrait,
{
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        let current = self.queue.pop_front()?;

        for x in 0..current.child_count() {
            self.queue.push_back(verify_attribute_ok(current.child(x))?);
        }

        if current.valid() {
            Some(current)
        } else {
            None
        }
    }
}

/// Searches the attribute store tree depth first.
///
/// # Arguments
/// * `handle`  -   handle to an attribute_store_node_t
///
/// # Returns
/// - Some(Attribute) if there is an next child attribute
/// - None if the end of the tree is reached.
///
/// If an ATTRIBUTE_STORE_INVALID_NODE is received from the C-API
/// None is returned.
#[derive(Debug, Clone, PartialEq)]
pub struct AttributeStoreIteratorDFS<T: AttributeTrait> {
    stack: Vec<T>,
}

impl<T: AttributeTrait> AttributeStoreIteratorDFS<T> {
    pub fn new(attribute: T) -> Self {
        let mut stack = Vec::new();
        stack.push(attribute);
        Self { stack }
    }
}

#[mockall::automock]
impl<T> Iterator for AttributeStoreIteratorDFS<T>
where
    T: 'static + AttributeTrait,
{
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        let current = self.stack.pop()?;
        let child_size = current.child_count();
        for x in 0..child_size {
            let child_node = verify_attribute_ok(current.child(x))?;
            self.stack.push(child_node);
        }

        if current.valid() {
            Some(current)
        } else {
            None
        }
    }
}
