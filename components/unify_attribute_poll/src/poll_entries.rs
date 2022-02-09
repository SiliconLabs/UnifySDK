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
use std::collections::HashMap;
use std::fmt::Display;
use unify_log_sys::*;
use unify_middleware::contiki::PlatformTrait;
use unify_middleware::Attribute;

declare_app_name!("poll_engine");

pub trait PollQueueTrait: Send + Sync + Display {
    /// returns the last entry which was popped off the queue. this does not
    /// include items that where removed via the remove call!
    fn is_flagged(&self, attribute: Attribute) -> bool;
    /// flag an attribute. no logic is attached to it. used to support external
    /// bookkeeping to provide means to flag an item that was polled.
    fn set_flag(&mut self, attribute: Attribute) -> bool;
    /// removes an item from the poll queue
    fn remove(&mut self, attribute: &Attribute) -> bool;
    /// get a borrow to the upcoming item in the queue
    fn upcoming(&self) -> Option<(Attribute, u64)>;
    /// add an new item to the poll queue. Interval is in seconds returns true
    /// when the attribute was not in the queue returns false when the attribute
    /// is already in the queue in the latter, the attribute is not added. Use
    /// requeue if you want to readd an attribute in the system
    fn queue(&mut self, attribute: Attribute, interval: u64) -> bool;
    /// re-add an item with the interval given on previous insertion. timeout
    /// time= now + interval
    fn requeue(&mut self, attribute: Attribute) -> bool;
    /// see wether the given attribute is in the queue
    fn contains(&self, attribute: &Attribute) -> bool;
}

/// This struct encapsulates a number of PollEntry's. They are stored ordered on
/// nearest deadline. Note that they are ordered in reverse order. e.g. the
/// nearest deadline is at the end of the list.
/// * Inserting and removing have both log(n) complexity.
/// * Lookup has a constant complexity

pub struct PollEntries {
    poll_queue: Vec<(Attribute, u64)>,
    interval_cache: HashMap<Attribute, u64>,
    platform: Box<dyn PlatformTrait>,
    flagged: Option<Attribute>,
}

impl PollEntries {
    pub fn new(platform: impl PlatformTrait + 'static) -> PollEntries {
        PollEntries {
            poll_queue: Vec::new(),
            interval_cache: HashMap::new(),
            platform: Box::new(platform),
            flagged: None,
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

    fn create_entry(&self, interval: u64, attribute: Attribute) -> (Attribute, u64) {
        let now = self.platform.clock_seconds();
        let deadline = now + interval;
        let entry = (attribute, deadline);
        entry
    }
}

impl PollQueueTrait for PollEntries {
    fn is_flagged(&self, attribute: Attribute) -> bool {
        self.flagged == Some(attribute)
    }

    fn set_flag(&mut self, attribute: Attribute) -> bool {
        if self.contains(&attribute) {
            self.flagged = Some(attribute);
            return true;
        }
        false
    }

    fn remove(&mut self, attribute: &Attribute) -> bool {
        assert!(self.poll_queue.len() == self.interval_cache.len());

        if let Some(index) = self.find_attribute_index(&attribute) {
            let rem = self.poll_queue.remove(index);
            log_debug!("removed {:?} from the poll entries", rem);

            self.interval_cache.remove(attribute);
            self.sort();

            if self.is_flagged(*attribute) {
                self.flagged = None;
            }

            return true;
        }
        false
    }

    fn upcoming(&self) -> Option<(Attribute, u64)> {
        self.poll_queue.last().cloned()
    }

    fn queue(&mut self, attribute: Attribute, interval: u64) -> bool {
        assert!(self.poll_queue.len() == self.interval_cache.len());

        let entry = self.create_entry(interval, attribute);
        match self.interval_cache.insert(attribute, interval) {
            Some(old) => {
                if old != interval {
                    let index = self
                        .find_attribute_index(&attribute)
                        .expect("interval cache and poll queue should be in sync");
                    self.poll_queue[index] = entry;
                    self.sort();
                } else {
                    return false;
                }
            }
            None => {
                self.poll_queue.push(entry);
                self.sort();
            }
        }
        return true;
    }

    fn requeue(&mut self, attribute: Attribute) -> bool {
        if let Some(interval) = self.interval_cache.get(&attribute).cloned() {
            if let Some(index) = self.find_attribute_index(&attribute) {
                log_debug!("re-queue {} timeout= {}s", attribute, interval);
                let entry = self.create_entry(interval, attribute);
                self.poll_queue[index] = entry;
                self.sort();
                return true;
            }
        }
        false
    }

    fn contains(&self, attribute: &Attribute) -> bool {
        assert!(self.poll_queue.len() == self.interval_cache.len());
        self.interval_cache.contains_key(attribute)
    }
}

impl Display for PollEntries {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if self.poll_queue.is_empty() {
            return write!(f, "queue is empty");
        }

        let attributes: Vec<(String, &u64)> = self
            .poll_queue
            .iter()
            .rev()
            .map(|(attribute, i)| (format!("{}", attribute), i))
            .collect();

        // determine column width
        let width = match attributes.iter().max_by(|x, y| x.0.len().cmp(&y.0.len())) {
            Some(entry) => entry.0.len() + 7,
            None => return std::fmt::Result::Err(std::fmt::Error),
        };

        writeln!(f, "\n{:=^width$}", "POLL-QUEUE")?;
        writeln!(
            f,
            "{:^width$}",
            format!("{:<20}{}", "items in queue:", self.poll_queue.len())
        )?;

        let flagged_str = match self.flagged {
            Some(a) => format!("[{}]", a.handle),
            None => "None".to_string(),
        };
        writeln!(
            f,
            "{:^width$}",
            format!("{:<20}{:?}", "flagged attribute:", flagged_str)
        )?;

        if let Some((_, i)) = self.upcoming() {
            writeln!(
                f,
                "{:^width$}",
                format!(
                    "{:<20}{}s",
                    "upcoming poll in:",
                    safe_sub_as_string(i, self.platform.clock_seconds())
                )
            )?;
        }

        writeln!(f, "{:<width$}", " ")?;
        for (attribute, interval) in attributes {
            writeln!(
                f,
                "{:.<width$}{:^5}",
                attribute,
                format!(
                    "{}s",
                    safe_sub_as_string(*interval, self.platform.clock_seconds())
                ),
                width = width - 5
            )?;
        }
        writeln!(f, "{:=<width$}")
    }
}

fn safe_sub_as_string(a: u64, b: u64) -> String {
    match a.overflowing_sub(b) {
        (val, false) => val.to_string(),
        (wrapped, true) => format!("-{}", u64::MAX - wrapped + 1),
    }
}

/// Testing poll entries
#[cfg(test)]
mod poll_entries_test {
    use super::*;
    use mockall::*;
    use unify_middleware::contiki::MockPlatformTrait;

    #[test]
    fn one_item_in_queue() {
        let mut platform_mock = MockPlatformTrait::new();
        platform_mock
            .expect_clock_seconds()
            .with()
            .return_const(0u64);

        let mut entries = PollEntries::new(platform_mock);
        assert_eq!(entries.upcoming(), None);

        let attribute1 = Attribute::new(1, 1).unwrap();

        assert!(entries.queue(attribute1, 22));
        assert_eq!(entries.upcoming(), Some((attribute1, 22)));
    }

    #[test]
    fn cannot_readd_via_queue() {
        let mut platform_mock = MockPlatformTrait::new();
        platform_mock
            .expect_clock_seconds()
            .with()
            .return_const(0u64);

        let mut entries = PollEntries::new(platform_mock);
        let attribute1 = Attribute::new(1, 1).unwrap();

        assert!(entries.queue(attribute1, 22));
        assert!(!entries.queue(attribute1, 22));
    }

    #[test]
    fn dequeue_multiple() {
        let mut seq = Sequence::new();

        let mut platform_mock = MockPlatformTrait::new();
        platform_mock
            .expect_clock_seconds()
            .with()
            .times(3)
            .in_sequence(&mut seq)
            .return_const(0u64);

        let mut entries = PollEntries::new(platform_mock);
        assert!(entries.queue(Attribute::new(2, 2).unwrap(), 4));
        assert!(entries.queue(Attribute::new(0, 0).unwrap(), 2));
        assert!(entries.queue(Attribute::new(1, 1).unwrap(), 3));

        assert_eq!(entries.upcoming(), Some((Attribute::new(0, 0).unwrap(), 2)));
        assert!(entries.remove(&Attribute::new(0, 0).unwrap()));

        assert_eq!(entries.upcoming(), Some((Attribute::new(1, 1).unwrap(), 3)));
        assert!(entries.remove(&Attribute::new(1, 1).unwrap()));

        assert_eq!(entries.upcoming(), Some((Attribute::new(2, 2).unwrap(), 4)));
    }

    #[test]
    fn requeue() {
        let mut seq = Sequence::new();

        let mut platform_mock = MockPlatformTrait::new();
        platform_mock
            .expect_clock_seconds()
            .with()
            .times(3)
            .in_sequence(&mut seq)
            .return_const(0u64);
        platform_mock
            .expect_clock_seconds()
            .with()
            .times(1)
            .in_sequence(&mut seq)
            .return_const(2u64);

        let mut entries = PollEntries::new(platform_mock);
        assert!(entries.queue(Attribute::new(2, 2).unwrap(), 4));
        assert!(entries.queue(Attribute::new(0, 0).unwrap(), 2));
        assert!(entries.queue(Attribute::new(1, 1).unwrap(), 3));
        assert!(entries.requeue(Attribute::new(0, 0).unwrap()));
        assert_eq!(entries.upcoming(), Some((Attribute::new(1, 1).unwrap(), 3)));

        assert!(!entries.requeue(Attribute::new(33, 0).unwrap()));
    }

    #[test]
    fn remove_test() {
        let mut platform_mock = MockPlatformTrait::new();
        platform_mock
            .expect_clock_seconds()
            .with()
            .return_const(0u64);

        let mut entries = PollEntries::new(platform_mock);
        assert!(entries.queue(Attribute::new(2, 2).unwrap(), 2));
        assert!(entries.queue(Attribute::new(0, 0).unwrap(), 0));
        assert!(entries.queue(Attribute::new(1, 1).unwrap(), 1));

        assert!(entries.remove(&Attribute::new(0, 0).unwrap()));
        assert_eq!(entries.upcoming(), Some((Attribute::new(1, 1).unwrap(), 1)));

        assert!(entries.remove(&Attribute::new(2, 2).unwrap()));
        assert_eq!(entries.upcoming(), Some((Attribute::new(1, 1).unwrap(), 1)));

        assert!(entries.remove(&Attribute::new(1, 1).unwrap()));
        assert_eq!(entries.upcoming(), None);
    }

    #[test]
    fn test_find_of_queue() {
        let mut platform_mock = MockPlatformTrait::new();
        platform_mock
            .expect_clock_seconds()
            .with()
            .return_const(0u64);

        let mut entries = PollEntries::new(platform_mock);
        assert!(entries.queue(Attribute::new(86, 1).unwrap(), 7200));
        assert!(entries.queue(Attribute::new(88, 1).unwrap(), 7200));
        assert!(entries.queue(Attribute::new(90, 1).unwrap(), 7200));
        assert!(!entries.queue(Attribute::new(90, 1).unwrap(), 7200));
        assert!(!entries.queue(Attribute::new(90, 1).unwrap(), 7200));
    }

    #[test]
    fn test_restart() {
        let mut platform_mock = MockPlatformTrait::new();
        platform_mock
            .expect_clock_seconds()
            .with()
            .return_const(0u64);

        let mut entries = PollEntries::new(platform_mock);
        assert!(entries.queue(Attribute::new(186, 34562).unwrap(), 7200));
        assert!(entries.queue(Attribute::new(184, 34562).unwrap(), 7200));
        assert!(entries.queue(Attribute::new(182, 34562).unwrap(), 7200));
        assert!(entries.queue(Attribute::new(86, 34562).unwrap(), 7200));
        assert!(entries.queue(Attribute::new(88, 34562).unwrap(), 7200));
        assert!(entries.queue(Attribute::new(90, 34562).unwrap(), 7200));
        assert!(entries.queue(Attribute::new(143, 9474).unwrap(), 7200));

        assert!(entries.requeue(Attribute::new(186, 34562).unwrap()));
        assert!(entries.requeue(Attribute::new(184, 34562).unwrap()));
        assert!(entries.requeue(Attribute::new(182, 34562).unwrap()));

        assert!(!entries.queue(Attribute::new(186, 34562).unwrap(), 7200));
        assert!(!entries.queue(Attribute::new(184, 34562).unwrap(), 7200));
    }

    #[test]
    fn test_overflowing() {
        let mut seq = Sequence::new();

        let mut platform_mock = MockPlatformTrait::new();
        platform_mock
            .expect_clock_seconds()
            .times(1)
            .in_sequence(&mut seq)
            .with()
            .return_const(0u64);
        platform_mock
            .expect_clock_seconds()
            .times(2)
            .in_sequence(&mut seq)
            .with()
            .return_const(20u64);

        let mut entries = PollEntries::new(platform_mock);
        assert!(entries.queue(Attribute::new(88, 34562).unwrap(), 10));
        println!("{}", entries);
    }
}
