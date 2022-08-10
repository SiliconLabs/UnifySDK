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

use core::fmt;
use serde::{de, Deserialize, Deserializer};
use std::collections::BTreeMap;
use std::path::Path;
use unify_log_sys::*;
use unify_middleware::attribute_store_or_return_with;

declare_app_name!("zwave_poll_manager");

type AttributeType = u32;
type PollInterval = u32;
#[derive(Deserialize, Debug, Clone, Copy)]
struct PollConfig {
    #[serde(deserialize_with = "deserialize_attribute_type")]
    pub attribute_type: AttributeType,
    pub polling_interval_zwave: Option<u32>,
    pub polling_interval_zwave_v1: Option<u32>,
    pub polling_interval_zwave_v2: Option<u32>,
}

impl PollConfig {
    fn to_poll_map(&self, poll_map: &mut AttributePollMap) {
        if let Some(v0) = self.polling_interval_zwave.and_then(none_if_zero) {
            poll_map.0.insert(self.attribute_type, v0);
        }
        if let Some(v1) = self.polling_interval_zwave_v1.and_then(none_if_zero) {
            poll_map.1.insert(self.attribute_type, v1);
        }
        if let Some(v2) = self.polling_interval_zwave_v2.and_then(none_if_zero) {
            poll_map.2.insert(self.attribute_type, v2);
        }
    }
}

// Returns `None` if the value is 0
fn none_if_zero(val: u32) -> Option<u32> {
    if val == 0 {
        None
    } else {
        Some(val)
    }
}

#[derive(Debug, Clone, Default, PartialEq)]
pub struct AttributePollMap(
    pub BTreeMap<AttributeType, PollInterval>, // attribute_type, polling_interval_zwave
    pub BTreeMap<AttributeType, PollInterval>, // attribute_type, polling_interval_zwave_v1
    pub BTreeMap<AttributeType, PollInterval>, // attribute_type, polling_interval_zwave_v2
);

/// Deserialize attribute_type from either String representation or u32 to a u32.
fn deserialize_attribute_type<'de, D>(deserializer: D) -> Result<u32, D::Error>
where
    D: Deserializer<'de>,
{
    struct AttributeTypeVisitor;
    impl<'de> de::Visitor<'de> for AttributeTypeVisitor {
        type Value = u32;
        fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
            formatter.write_str("An attribute type as String or u32")
        }

        // Support writing type as String in Config file, use
        fn visit_str<E>(self, s: &str) -> Result<Self::Value, E>
        where
            E: de::Error,
        {
            let attribute_store = attribute_store_or_return_with!(Err(de::Error::custom(
                "attribute_store not initialized"
            )));
            if let Ok(res) = attribute_store.attribute_type_from_name(s) {
                Ok(res)
            } else {
                Err(de::Error::invalid_value(de::Unexpected::Str(s), &self))
            }
        }

        // Support writing type as u32 in Config file
        fn visit_u64<E>(self, v: u64) -> Result<Self::Value, E>
        where
            E: de::Error,
        {
            v.try_into().map_err(|_| {
                serde::de::Error::invalid_value(
                    serde::de::Unexpected::Unsigned(v),
                    &" values should not exceed u32",
                )
            })
        }
    }
    deserializer.deserialize_any(AttributeTypeVisitor)
}

/// Reads a whole yaml file and tries to parse it into a [AttributePollMap]
///
/// # Arguments
///
/// * `poll_list_file`  any object that implemented `AsRef<Path>` that point to
///   a valid .yaml file on disk
///
/// # Returns
///
/// * `Ok(AttributePollMap)`    if the yaml file was successfully parsed.
/// * `Err(e)`                  std::io::Error if the file could not be opened.
///   serde_yaml::Error if there where parsing problems
pub fn read_config_yaml_to_map<P: AsRef<Path>>(
    poll_list_file: &P,
) -> Result<AttributePollMap, Box<dyn std::error::Error>> {
    let data = std::fs::read(poll_list_file)?;
    buffer_to_poll_map(&data)
}

/// deserializes the raw yaml data into an [AttributePollMap].
///
/// # Arguments
///
/// * `data`    raw byte slice (encoded utf-8) containing valid yaml data
///
/// # Returns
///
/// * `Ok(AttributePollMap)`    de-serialization successful
/// * `Err(serde_yaml::Error)`  error deserializing
fn buffer_to_poll_map(data: &[u8]) -> Result<AttributePollMap, Box<dyn std::error::Error>> {
    let mut poll_map = AttributePollMap::default();
    serde_yaml::from_slice::<Vec<PollConfig>>(data)?
        .iter()
        .for_each(|p| p.to_poll_map(&mut poll_map));
    Ok(poll_map)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_yaml_parsing() {
        let yaml = b"---
        -
          attrgfdf: Test
        ";
        assert!(buffer_to_poll_map(yaml).is_err());

        let yaml = b"---\n-attribute_type: TEST\n";
        assert!(buffer_to_poll_map(yaml).is_err());

        let yaml = format!(
            "
        - attribute_type: {}
          polling_interval_zwave: 3600
          polling_interval_zwave_v1: 3600",
            u32::MAX as u64 + 1,
        );
        assert!(buffer_to_poll_map(yaml.as_bytes()).is_err());

        let yaml = b"---
        -
          attribute_type: 0x123
          polling_interval_zwave: 3600
          polling_interval_zwave_v1: 3600";
        assert!(buffer_to_poll_map(yaml).is_ok());

        let yaml = b"
        - attribute_type: 123
          polling_interval_zwave: 3600
          polling_interval_zwave_v1: 3600";
        assert!(buffer_to_poll_map(yaml).is_ok());
    }

    #[test]
    fn test_yaml_config_mapping() {
        let yaml = b"
        - attribute_type: 123
          polling_interval_zwave: 12
          polling_interval_zwave_v2: 0
          polling_interval_zwave_v1: 3600
        - attribute_type: 55
          polling_interval_zwave_v2: 1
          polling_interval_zwave_v1: 44";

        let map = buffer_to_poll_map(yaml).unwrap();
        assert_eq!(map.0.len(), 1);
        assert_eq!(map.1.len(), 2);

        // setting value to 0 does not import it into the config
        assert_eq!(map.2.len(), 1);

        assert_eq!(map.0[&123], 12);
        assert_eq!(map.1[&123], 3600);
        assert_eq!(map.1[&55], 44);
        assert_eq!(map.2[&55], 1);
    }
}
