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

use alloc::format;
use core::fmt::Display;
use unify_middleware::{
    contiki::{ContikiPlatform, PlatformTrait},
    Attribute,
};

#[cfg_attr(test, mockall::automock)]
pub trait PollQueueTrait {
    /// removes the upcoming attribute from the poll queue.
    ///
    /// # Returns
    ///
    /// * `Some((attribute, interval))`  in the case there is an upcoming
    ///   attribute. interval is the value which the attribute was registered
    ///   with (in sec)
    /// * `None`                         when there are no items in the queue
    fn pop_next(&mut self) -> Option<(Attribute, u64)>;

    /// removes an item from the poll queue
    ///
    /// # Arguments
    ///
    /// * `attribute`   attribute to remove
    ///
    /// # Returns
    ///
    /// * `true`  if the attribute was present and removed
    /// * `false` the attribute is not present in the queue
    fn remove(&mut self, attribute: &Attribute) -> bool;

    /// get a borrow to the upcoming item in the queue
    ///
    /// # Returns
    ///
    /// * `Some((attribute, interval))`  in the case there is an upcoming
    ///   attribute. interval is the value which the attribute was registered
    ///   with (in sec)
    /// * `None`                         when there are no items in the queue
    fn upcoming(&self) -> Option<(Attribute, u64)>;

    /// add an new item to the poll queue. Interval is in seconds. Attributes
    /// will be updated in the queue when this function is called with different
    /// parameters. requeue if you want to readd an attribute in the system.
    ///
    /// # Arguments
    ///
    /// * `attribute`   attribute to queue
    /// * `interval`    how many seconds from now this attribute should reach
    ///   its deadline.
    /// * `now`         current timestamp in seconds
    ///
    /// # Returns
    ///
    /// * `true`        when the attribute was not in the queue or registered
    ///   with an different interval. the queue was updated successfully
    /// * `false`       when no action was performed. the attribute + interval
    ///   is already queue-ed
    fn queue(&mut self, attribute: Attribute, interval: u64, now: u64) -> bool;

    /// re-add an item with the interval given on previous insertion. timeout
    /// time= now + interval
    ///
    /// # Arguments
    ///
    /// * `attribute`   attribute to queue
    /// * `now`         current timestamp in seconds
    ///
    /// # Returns
    ///
    /// * `true`        when the attribute was not in the queue or registered
    ///   with an different interval. the queue was updated successfully
    /// * `false`       when the attribute
    fn requeue(&mut self, attribute: Attribute, now: u64) -> bool;

    /// see wether the given attribute is in the queue
    ///
    /// # Arguments
    ///
    /// * `attribute`
    ///
    /// # Returns
    ///
    /// * `true` if its present in the poll-queue
    /// * `false` if its not
    fn contains(&self, attribute: &Attribute) -> bool;

    /// Returns a slice to the internal queue list
    fn entries(&self) -> &[(Attribute, u64)];
}

impl Display for dyn PollQueueTrait {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if self.upcoming().is_none() {
            return write!(f, "queue is empty");
        }

        let attributes: Vec<(String, &u64)> = self
            .entries()
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
            format!("{:<20}{}", "items in queue:", self.entries().len())
        )?;

        if let Some((_, i)) = self.upcoming() {
            writeln!(
                f,
                "{:^width$}",
                format!(
                    "{:<20}{}s",
                    "upcoming poll in:",
                    safe_sub_as_string(i, ContikiPlatform::default().clock_seconds()),
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
                    safe_sub_as_string(*interval, ContikiPlatform::default().clock_seconds())
                ),
                width = width - 5
            )?;
        }
        writeln!(f, "{:=<width$}", width)
    }
}

fn safe_sub_as_string(a: u64, b: u64) -> String {
    match a.overflowing_sub(b) {
        (val, false) => val.to_string(),
        (wrapped, true) => format!("-{}", u64::MAX - wrapped + 1),
    }
}
