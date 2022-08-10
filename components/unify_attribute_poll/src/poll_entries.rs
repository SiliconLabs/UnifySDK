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
use crate::poll_queue_trait::PollQueueTrait;
use alloc::collections::BTreeMap;
use unify_middleware::Attribute;

/// This struct encapsulates a number of PollEntry's. They are stored ordered on
/// nearest deadline. Note that they are ordered in reverse order. e.g. the
/// nearest deadline is at the end of the list.
/// * Lookup, inserting, removing have both log(n) complexity.
pub struct PollEntries {
    poll_queue: Vec<(Attribute, u64)>,
    interval_cache: BTreeMap<Attribute, u64>,
}

impl PollEntries {
    pub fn new() -> PollEntries {
        PollEntries {
            poll_queue: Vec::new(),
            interval_cache: BTreeMap::new(),
        }
    }

    fn sort(&mut self) {
        self.poll_queue
            .sort_unstable_by(|a, b| a.1.cmp(&b.1).reverse());
    }

    fn find_attribute_index(&self, attribute: &Attribute) -> Option<usize> {
        self.poll_queue
            .iter()
            .position(|entry| entry.0 == *attribute)
    }

    fn update_sorted_unchecked(&mut self, attribute: Attribute, interval: u64, now: u64) {
        let index = self
            .find_attribute_index(&attribute)
            .expect("interval cache and poll queue should be in sync");
        self.poll_queue[index] = (attribute, interval + now);
        self.sort();
    }
}

impl PollQueueTrait for PollEntries {
    fn pop_next(&mut self) -> Option<(Attribute, u64)> {
        let entry = self.poll_queue.pop()?;
        let interval = self
            .interval_cache
            .remove(&entry.0)
            .expect("interval_cache and poll-queue should always be in sync");

        Some((entry.0, interval))
    }

    fn remove(&mut self, attribute: &Attribute) -> bool {
        if let Some(index) = self.find_attribute_index(&attribute) {
            self.poll_queue.remove(index);
            self.interval_cache.remove(attribute);
            self.sort();

            return true;
        }
        false
    }

    fn upcoming(&self) -> Option<(Attribute, u64)> {
        self.poll_queue.last().cloned()
    }

    fn queue(&mut self, attribute: Attribute, interval: u64, now: u64) -> bool {
        assert!(
            self.poll_queue.len() == self.interval_cache.len(),
            "expected interval_cache and poll_queue to never be out of sync"
        );

        match self.interval_cache.insert(attribute, interval) {
            Some(old) => {
                let updated = old != interval;
                if updated {
                    self.update_sorted_unchecked(attribute, interval, now);
                }
                return updated;
            }
            None => {
                self.poll_queue.push((attribute, interval + now));
                self.sort();
                return true;
            }
        }
    }

    fn requeue(&mut self, attribute: Attribute, now: u64) -> bool {
        assert!(
            self.poll_queue.len() == self.interval_cache.len(),
            "expected interval_cache and poll_queue to never be out of sync"
        );

        if let Some(interval) = self.interval_cache.get(&attribute).cloned() {
            self.update_sorted_unchecked(attribute, interval, now);
            return true;
        }
        false
    }

    fn contains(&self, attribute: &Attribute) -> bool {
        assert!(
            self.poll_queue.len() == self.interval_cache.len(),
            "expected interval_cache and poll_queue to never be out of sync"
        );

        self.interval_cache.contains_key(attribute)
    }

    fn entries(&self) -> &[(Attribute, u64)] {
        &self.poll_queue
    }
}

/// Testing poll entries
#[cfg(test)]
mod poll_entries_test {
    use super::*;
    use unify_middleware::{AttributeStore, AttributeStoreTrait};

    #[test]
    fn one_item_in_queue() {
        let attribute_store = AttributeStore::new().unwrap();
        let mut entries = PollEntries::new();
        assert_eq!(entries.upcoming(), None);

        let attribute1 = attribute_store.from_handle_and_type(1, 1);

        assert!(entries.queue(attribute1, 22, 0));
        assert_eq!(entries.upcoming(), Some((attribute1, 22)));
        assert_eq!(entries.pop_next(), Some((attribute1, 22)));
        assert_eq!(entries.pop_next(), None);
    }

    #[test]
    fn cannot_readd_via_queue() {
        let attribute_store = AttributeStore::new().unwrap();
        let mut entries = PollEntries::new();
        let attribute1 = attribute_store.from_handle_and_type(1, 1);

        assert!(entries.queue(attribute1, 22, 0));
        assert!(!entries.queue(attribute1, 22, 0));
    }

    #[test]
    fn dequeue_multiple() {
        let attribute_store = AttributeStore::new().unwrap();
        let mut entries = PollEntries::new();
        assert!(entries.queue(attribute_store.from_handle_and_type(2, 2), 4, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(0, 0), 2, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(1, 1), 3, 0));

        assert_eq!(
            entries.upcoming(),
            Some((attribute_store.from_handle_and_type(0, 0), 2))
        );
        assert!(entries.remove(&attribute_store.from_handle_and_type(0, 0)));

        assert_eq!(
            entries.upcoming(),
            Some((attribute_store.from_handle_and_type(1, 1), 3))
        );
        assert!(entries.remove(&attribute_store.from_handle_and_type(1, 1)));

        assert_eq!(
            entries.upcoming(),
            Some((attribute_store.from_handle_and_type(2, 2), 4))
        );
        assert_eq!(
            entries.pop_next(),
            Some((attribute_store.from_handle_and_type(2, 2), 4))
        );
    }

    #[test]
    fn requeue() {
        let attribute_store = AttributeStore::new().unwrap();
        let mut entries: Box<dyn PollQueueTrait> = Box::new(PollEntries::new());
        let mut now = 0;
        assert!(entries.queue(attribute_store.from_handle_and_type(2, 2), 4, now));
        assert!(entries.queue(attribute_store.from_handle_and_type(0, 0), 2, now));
        assert!(entries.queue(attribute_store.from_handle_and_type(1, 1), 3, now));
        now += 2;
        assert!(entries.requeue(attribute_store.from_handle_and_type(0, 0), now));

        assert_eq!(
            entries.upcoming(),
            Some((attribute_store.from_handle_and_type(1, 1), 3))
        );
        assert!(!entries.requeue(attribute_store.from_handle_and_type(33, 0), 2));
        assert_eq!(
            entries.upcoming(),
            Some((attribute_store.from_handle_and_type(1, 1), 3))
        );
    }

    #[test]
    fn remove_test() {
        let attribute_store = AttributeStore::new().unwrap();
        let mut entries = PollEntries::new();
        assert!(entries.queue(attribute_store.from_handle_and_type(2, 2), 2, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(0, 0), 0, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(1, 1), 1, 0));

        assert!(entries.remove(&attribute_store.from_handle_and_type(0, 0)));
        assert_eq!(
            entries.upcoming(),
            Some((attribute_store.from_handle_and_type(1, 1), 1))
        );

        assert!(entries.remove(&attribute_store.from_handle_and_type(2, 2)));
        assert_eq!(
            entries.upcoming(),
            Some((attribute_store.from_handle_and_type(1, 1), 1))
        );

        assert!(entries.remove(&attribute_store.from_handle_and_type(1, 1)));
        assert_eq!(entries.upcoming(), None);
    }

    #[test]
    fn test_find_of_queue() {
        let attribute_store = AttributeStore::new().unwrap();
        let mut entries = PollEntries::new();
        assert!(entries.queue(attribute_store.from_handle_and_type(86, 1), 7200, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(88, 1), 7200, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(90, 1), 7200, 0));
        assert!(!entries.queue(attribute_store.from_handle_and_type(90, 1), 7200, 0));
        assert!(!entries.queue(attribute_store.from_handle_and_type(90, 1), 7200, 0));
    }

    #[test]
    fn test_restart() {
        let attribute_store = AttributeStore::new().unwrap();
        let mut entries = PollEntries::new();
        assert!(entries.queue(attribute_store.from_handle_and_type(186, 34562), 7200, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(184, 34562), 7200, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(182, 34562), 7200, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(86, 34562), 7200, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(88, 34562), 7200, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(90, 34562), 7200, 0));
        assert!(entries.queue(attribute_store.from_handle_and_type(143, 9474), 7200, 0));

        assert!(entries.requeue(attribute_store.from_handle_and_type(186, 34562), 1));
        assert!(entries.requeue(attribute_store.from_handle_and_type(184, 34562), 2));
        assert!(entries.requeue(attribute_store.from_handle_and_type(182, 34562), 3));
        assert!(!entries.queue(attribute_store.from_handle_and_type(186, 34562), 7200, 100));
        assert!(!entries.queue(attribute_store.from_handle_and_type(184, 34562), 7200, 2000));
    }

    #[test]
    fn test_overflowing() {
        let attribute_store = AttributeStore::new().unwrap();
        let mut entries = PollEntries::new();
        assert!(entries.queue(attribute_store.from_handle_and_type(88, 34562), 10, 0));
        println!("{}", &entries as &dyn PollQueueTrait);
    }
}
