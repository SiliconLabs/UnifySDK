// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

#![deny(missing_docs)]
//! Handle JSON conversion to/from internal types
//!
//!This module defines the domain-specific data types, so that we can
//!use serde_json to read/generate the JSON strings in the MQTT
//!payload.

// Not using this one yet, but it is needed for error handling if a
// message is malformed.
//use serde_json::Result;

// Support pretty-printing of the structs, too
use std::fmt;

/// A type to represent a provision for one device.
///
///This type is used internally in the Update, Remove, and List types.
///
/// See the spec for documentation of the fields.
// We may only need clone for tests?
#[derive(std::clone::Clone, PartialEq, Debug, Default)]
#[allow(non_snake_case)]
pub struct SmartStartEntry {
    pub DSK: String,
    pub Payload: serde_json::Value,
}

impl From<serde_json::Value> for SmartStartEntry {
    fn from(item: serde_json::Value) -> Self {
        let dsk = item["DSK"].as_str().unwrap().to_string();
        SmartStartEntry {
            DSK: dsk,
            Payload: item,
        }
    }
}

/// Convert from String to SmartStartEntry
impl From<String> for SmartStartEntry {
    fn from(item: String) -> Self {
        let tmp: serde_json::Value = serde_json::from_str(item.as_str()).unwrap();
        Self::from(tmp)
    }
}

/// Convert SmartStartEntry into serde_json::Value
impl From<SmartStartEntry> for serde_json::Value {
    fn from(entry: SmartStartEntry) -> Self {
        entry.Payload
    }
}

/// Convert SmartStartEntry into String
impl From<SmartStartEntry> for String {
    fn from(entry: SmartStartEntry) -> Self {
        entry.Payload.to_string()
    }
}

/// Pretty print the Provision type (more readable and compact than json)
impl fmt::Display for SmartStartEntry {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.Payload)
    }
}

#[cfg(test)]
mod test {
    use super::SmartStartEntry;

    #[test]
    /// Test deserialization of SmartStartEntry (i.e. String with Json to Object)
    fn test_smartstartentry_deserialization() {
        let entry_str = r#"{
            "DSK": "24859-64107-46202-12845-60475-62452-54892-59867",
            "Include": false,
            "ProtocolControllerUnid": "",
            "Unid": "8184198715775"
        }"#;
        let tmp: serde_json::Value = serde_json::from_str(entry_str).unwrap();
        let entry: SmartStartEntry = SmartStartEntry::from(tmp.clone());
        let entry_expected = SmartStartEntry {
            DSK: "24859-64107-46202-12845-60475-62452-54892-59867".to_string(),
            Payload: tmp,
        };
        assert_eq!(entry_expected, entry);
    }

    #[test]
    /// Test serialization of SmartStartEntry (i.e. Object to String with Json)
    fn test_smartstartentry_serialization() {
        let entry_expected_str = r#"{
            "DSK": "24859-64107-46202-12845-60475-62452-54892-59867",
            "Include": false,
            "ProtocolControllerUnid": "",
            "Unid": "8184198715775"
        }"#;
        let entry_obj = SmartStartEntry {
            DSK: String::from("24859-64107-46202-12845-60475-62452-54892-59867"),
            Payload: serde_json::from_str(entry_expected_str).unwrap(),
        };
        let entry: String = entry_obj.into();
        // Remove all empty spaces and newlines from expected.
        let entry_expected = String::from(entry_expected_str)
            .replace(" ", "")
            .replace("\n", "");
        assert_eq!(entry_expected, entry);
    }
}
