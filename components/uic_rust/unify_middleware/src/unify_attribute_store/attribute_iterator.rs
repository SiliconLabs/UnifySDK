// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

use std::collections::VecDeque;

use crate::{AttributeStoreExtTrait, AttributeStoreTrait};

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
pub struct UicAttributeStoreIterator<T: AttributeStoreExtTrait> {
    current_index: usize,
    queue: VecDeque<T>,
}

impl<T: AttributeStoreExtTrait> UicAttributeStoreIterator<T> {
    pub fn new(attribute: T) -> Self {
        let mut queue = VecDeque::new();
        queue.push_back(attribute);

        Self {
            current_index: 0,
            queue,
        }
    }
}

impl<T: AttributeStoreExtTrait + AttributeStoreTrait<Attribute = T>> Iterator
    for UicAttributeStoreIterator<T>
{
    type Item = T::Attribute;

    fn next(&mut self) -> Option<Self::Item> {
        let current = self.queue.pop_front()?;

        for x in 0..current.child_count() {
            self.queue.push_back(current.child(x).ok()?);
        }

        if current.exists() {
            return Some(current);
        } else {
            return None;
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
pub struct UicAttributeStoreIteratorDFS<T: AttributeStoreExtTrait> {
    stack: Vec<T>,
}

impl<T: AttributeStoreExtTrait> UicAttributeStoreIteratorDFS<T> {
    pub fn new(attribute: T) -> Self {
        let mut stack = Vec::new();
        stack.push(attribute);
        Self { stack }
    }
}

impl<T: AttributeStoreExtTrait + AttributeStoreTrait<Attribute = T>> Iterator
    for UicAttributeStoreIteratorDFS<T>
{
    type Item = T::Attribute;

    fn next(&mut self) -> Option<Self::Item> {
        let current = self.stack.pop()?;
        let child_size = current.child_count();
        for x in 0..child_size {
            let child_node = current.child(x).ok()?;
            self.stack.push(child_node);
        }

        if current.exists() {
            return Some(current);
        } else {
            return None;
        }
    }
}
