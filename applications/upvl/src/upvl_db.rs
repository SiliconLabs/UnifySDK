// License
// <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

// A file is automatically a module with the same name as the file.

//! UPVL Database Interface
//!
//! The first version of the database contains one table with the
//! provisions.
//!
//! We assume that the DSK is unique.
//!
//! We assume that there are so few entries in the database that
//! performance is not a problem.  This assumption is based on the
//! fact that the entire list of provisions must be published as one
//! MQTT message.

// For some nice hints about using rusqlite:
// https://tedspence.com/investigating-rust-with-sqlite-53d1f9a41112

use rusqlite::{params, Connection, Error};

use crate::upvl_json::{self, SmartStartEntry};
use upvl_log::{self};

const LOG_TAG: &str = "upvl_db";

/// Current Database Version.
/// At startup [db_upgrade] checks this against the "pragma_user_version",
/// and ensure migrating the database to the current version.
const CURRENT_DB_VERSION: i32 = 1;

/// Type that matches the rows in the database provision table.
// add the Debug trait to make the struct printable with :?
#[derive(Debug)]
struct UpvlDbEntry {
    dsk: String,
    payload: String,
}

/// Checks the database version and upgrades the data within the database if required
fn db_upgrade(db_conn: &Connection) {
    let version: i32 = db_conn
        .pragma_query_value(None, "user_version", |row| row.get(0))
        .unwrap();
    match version {
        0 => {
            // Version 0 upgrade, here we drop the table "provision" if it exists
            let my_result: Result<String, Error> = db_conn.query_row(
                "SELECT name FROM sqlite_master WHERE type='table' AND name='provision';",
                [],
                |row| row.get(0),
            );
            if my_result.is_ok() {
                println!("Dropping table");
                db_conn
                    .execute("DROP TABLE provision", [])
                    .expect("Failed to drop table provision");
            }
        }
        // Add version 1, 2, ... here
        _ => {}
    }
}

/// Open the database connection to the specified file.
/// Return the connection.
///
/// #Panics
/// This will **panic** if the database cannot be opened or the
/// provision table cannot be created
///
pub fn db_setup(db_file: String) -> rusqlite::Connection {
    upvl_log::log_info(LOG_TAG, format!("Using database file {}", db_file));
    // If UPVL cannot open the database, it exits with an error message.
    let db_conn = Connection::open(db_file).expect("Cannot open database");
    db_upgrade(&db_conn);
    db_conn
        .execute(
            "CREATE TABLE IF NOT EXISTS provision (
                 dsk               TEXT UNIQUE,
                 payload           TEXT
                 )",
            params![],
        )
        .expect("Cannot create provision table");
    // Set user_version in the database to `CURRENT_DB_VERSION`
    db_conn
        .pragma_update(None, "user_version", &CURRENT_DB_VERSION)
        .expect("Failed to set 'user_version' in database");
    db_conn
}

/// Close the database connection.
///
/// #Panics
/// This will **panic** if the database connection cannot be closed cleanly.
///
pub fn db_teardown(db_conn: Connection) {
    db_conn.close().expect("Database close failed");
}

/// Checks if an entry is valid
pub fn is_entry_valid(entry: &serde_json::Value) -> bool {
    entry["DSK"].is_string()
        && entry["Include"].is_boolean()
        && entry["ProtocolControllerUnid"].is_string()
        && entry["Unid"].is_string()
}

/// Update database for a provision in deserialized json format.
/// If a provision with this dsk already exists, update the
/// fields. If it does not exist, create a new provison.
pub fn db_upsert_entry(db_conn: &Connection, mut entry: SmartStartEntry) {
    // Insert the new DSK
    // If the dsk exists, and there are updates to some of the
    // fields, we need to replace those fields.

    // Check for existing object in the database with matching DSK
    if let Ok(db_payload) = db_conn.query_row(
        "SELECT dsk, payload from provision WHERE dsk = (:dsk)",
        &[(":dsk", &entry.DSK)],
        |row| {
            let result: Result<String, Error> = row.get(1);
            result
        },
    ) {
        // If entry found, merge the `entry.Payload` into it and overwrite `entry.Payload`
        if let Some(new_payload) = entry_merge(
            serde_json::from_str(db_payload.as_str()).ok(),
            Some(entry.Payload),
        ) {
            entry.Payload = new_payload;
        } else {
            upvl_log::log_warning(
                LOG_TAG,
                format!("Failed to merge SmartStart entries for DSK: {}", entry.DSK),
            );
            return;
        }
    }
    if !is_entry_valid(&entry.Payload) {
        upvl_log::log_warning(
            LOG_TAG,
            format!("SmartStart Entry invalid: {}", entry.Payload.to_string()),
        );
        return;
    }
    match db_conn.execute(
        "INSERT INTO provision (dsk, payload)
         VALUES (:dsk, :payload)
         ON CONFLICT(dsk) DO UPDATE SET (payload) = (:payload)",
        &[
            (":dsk", &entry.DSK),
            (":payload", &entry.Payload.to_string()),
        ],
    ) {
        Ok(_) => upvl_log::log_info(LOG_TAG, format!("Updated database for DSK {}.", entry.DSK)),
        Err(err) => upvl_log::log_error(
            LOG_TAG,
            format!("Error {} when updating table with DSK {}.", err, entry.DSK),
        ),
    }
}

/// Merge two serde_json::value objects.
/// This will overwrite any items in `dest` that also exist in `src`,
/// items that doesn't exist in `dest` will be created
fn entry_merge(
    dest: Option<serde_json::Value>,
    src: Option<serde_json::Value>,
) -> Option<serde_json::Value> {
    if let Some(serde_json::Value::Object(mut target)) = dest {
        if let Some(serde_json::Value::Object(mut source)) = src {
            target.append(&mut source);
            return Some(serde_json::Value::Object(target));
        }
    }
    None
}

///Remove a provision by DSK.
///
///Silently ignore if the provision is not found, since the remove might be
///a duplicate.
///
///Returns the number of deleted rows, which should be either 1 or 0
/// (since the DSK is unique).
pub fn db_remove_entry(db_conn: &Connection, entry: &upvl_json::SmartStartEntry) -> usize {
    match db_conn.execute(
        "DELETE FROM provision WHERE DSK = (:dsk)",
        &[(":dsk", &entry.DSK)],
    ) {
        Ok(deletions) => {
            upvl_log::log_info(
                LOG_TAG,
                format!("Removed {} entries with DSK {}.", deletions, entry.DSK),
            );
            deletions
        }
        Err(err) => {
            upvl_log::log_error(
                LOG_TAG,
                format!("Error {} when removing DSK {}.", err, entry.DSK),
            );
            0
        }
    }
}

/// Print out everything from the data base in json format.
///
/// TODO: change this to return an Option, where None indicates that
/// the POC should not publish because there are no changes.
pub fn db_list_provisions(db_conn: &Connection) -> Vec<serde_json::Value> {
    // Create an empty List
    let mut sms_list: Vec<serde_json::Value> = Vec::new();

    let mut stmt = db_conn
        .prepare("SELECT dsk, payload FROM provision")
        .expect("Failed to find any provisions.");

    // Ask rusqlite for an iterator over the rows returned from stmt
    // The iterator takes a row and returns a Result<SmartStartEntry, _>.
    let pv_iter = stmt
        .query_map(params![], |row| {
            // The ? will propagate an error to
            // the outer context of the closure.
            let payload_str: String = row.get(1)?;
            Ok(upvl_json::SmartStartEntry {
                DSK: row.get(0)?,
                Payload: serde_json::from_str(&payload_str).unwrap(),
            })
        })
        .expect("Cannot create iterator");

    // Add the SmartStartEntries to the sms_list
    for pv in pv_iter {
        // The error that was propagated above is handled here
        match pv {
            Ok(db_entry) => {
                sms_list.push(db_entry.Payload);
            }
            Err(err) => {
                upvl_log::log_error(
                    LOG_TAG,
                    format!("Error in dabatase lookup {}, skipping entry in List", err),
                );
            }
        }
    }
    upvl_log::log_info(
        LOG_TAG,
        format!(
            "To publish: {}",
            serde_json::to_string(&sms_list).expect("Cannot serialize List")
        ),
    );
    // We should probably publish even if there are no entries, eg, if
    // the last received message was a Remove.
    sms_list
}

/// Print out all the provisions.
///
/// #Panics
/// This will **panic** if the SELECT fails or if iterator creation fails.
///
pub fn db_debug_print_provisions(db_conn: &Connection) {
    let mut stmt = db_conn
        .prepare("SELECT dsk, payload FROM provision")
        .expect("Failed to find provisions.");
    let pv_iter = stmt
        .query_map(
            params![],
            // The closure will pass its errors
            // to the outer context with '?'.
            // The error is then handled in the
            // 'match' below.
            |row| {
                Ok(UpvlDbEntry {
                    dsk: row.get(0)?,
                    payload: row.get(1)?,
                })
            },
        )
        .expect("Cannot create iterator");
    for pv in pv_iter {
        // :? to format something that is not usually printable
        match pv {
            Ok(pv) => {
                upvl_log::log_info(LOG_TAG, format!("Found provision {:?}", pv));
            }
            Err(err) => {
                upvl_log::log_error(LOG_TAG, format!("Invalid provision {:?}", err));
            }
        }
    }
}

#[cfg(test)]
mod test {
    use super::{db_list_provisions, db_remove_entry, db_setup, db_upgrade, db_upsert_entry};
    use crate::upvl_json::SmartStartEntry;
    use rusqlite::{params, Connection};
    use serde_json::json;

    /// Helper for testing db_version check
    /// Sets the `db_version` as user_version and creates a table "provision"
    fn help_test_db_upgrade(db_conn: &Connection, db_version: i32) {
        db_conn
            .pragma_update(None, "user_version", &db_version)
            .expect("Should work");
        db_conn
            .execute("CREATE TABLE provision(id)", [])
            .expect("Should work");
        db_upgrade(db_conn);
    }
    #[test]
    fn test_db_upgrade_0() {
        // Test that db_upgrade drops the table "provision" if "user_version" is 0
        let db_conn = Connection::open(":memory:").expect("Cannot open database");
        help_test_db_upgrade(&db_conn, 0);
        db_upgrade(&db_conn);
        let mut stmt = db_conn
            .prepare("SELECT name FROM sqlite_master WHERE type='table' AND name='provision'")
            .expect("Should work");
        assert_eq!(false, stmt.exists([]).expect("Should work"));
    }

    #[test]
    fn test_db_upgrade_1() {
        // Test that db_upgrade doesn't drop the table "provision" if "user_version" is 1
        let db_conn = Connection::open(":memory:").expect("Cannot open database");
        help_test_db_upgrade(&db_conn, 1);
        db_upgrade(&db_conn);
        let mut stmt = db_conn
            .prepare("SELECT name FROM sqlite_master WHERE type='table' AND name='provision'")
            .expect("Should work");
        assert_eq!(true, stmt.exists([]).expect("Should work"));
    }
    #[test]
    // Don't run this test case until the TODO is fixed
    /// Test that we can create the database and the provision table.
    fn test_setup_succeeds() {
        let db_name = ":memory:";

        // Currently, setup does not handle errors gracefully, so this
        // is enough to test that it works.
        db_setup(db_name.to_string());
    }
    // This may not be a good way to create a test database.  If we
    // want to move the db_conn into this module we have to change it.
    // But it is the simplest way to generate an empty database.
    fn help_test_setup() -> Connection {
        let db_conn = Connection::open_in_memory().expect("Cannot open db");
        db_conn
            .execute(
                "CREATE TABLE provision (
                  dsk               TEXT UNIQUE,
                  payload           TEXT
                  )",
                params![],
            )
            .expect("Cannot create table");
        db_conn
    }
    fn help_test_insert(db_conn: &Connection, dsk: String) -> SmartStartEntry {
        let provision = SmartStartEntry {
            DSK: dsk.clone(),
            Payload: json!({
                 "DSK": dsk,
                 "Include":false,
                 "ProtocolControllerUnid": "",
                 "Unid":""}),
        };
        db_upsert_entry(db_conn, provision.clone());
        provision
    }
    #[test]
    /// Insert a dsk in an empty db and remove it
    fn test_remove() {
        let db_conn: Connection = help_test_setup();
        let dsk1 = "42".to_string();
        let dsk1_rem = dsk1.clone();
        help_test_insert(&db_conn, dsk1);
        let remove_fail = SmartStartEntry {
            DSK: "666".to_string(),
            ..Default::default()
        };
        assert_eq!(db_remove_entry(&db_conn, &remove_fail), 0);
        let remove_sms = SmartStartEntry {
            DSK: dsk1_rem,
            ..Default::default()
        };
        assert_eq!(db_remove_entry(&db_conn, &remove_sms), 1);
    }
    #[test]
    /// Generate List from
    /// - empty db
    fn test_list_empty() {
        let db_conn = help_test_setup();
        // empty List
        let list = db_list_provisions(&db_conn);
        let exp_list: Vec<serde_json::Value> = Vec::new();
        assert_eq!(
            serde_json::to_string(&list).unwrap(),
            serde_json::to_string(&exp_list).unwrap(),
            "The List found in the database with no elements \
                          is different from the expected List"
        );
    }
    #[test]
    /// Generate List from
    /// - one row
    fn test_list_one() {
        let db_conn = help_test_setup();
        let pv42 = help_test_insert(&db_conn, "42".to_string());
        let list = db_list_provisions(&db_conn);
        let exp_list: Vec<serde_json::Value> = vec![pv42.into()];
        assert_eq!(
            serde_json::to_string(&list).unwrap(),
            serde_json::to_string(&exp_list).unwrap(),
            "The List found in the database with one element \
                            is different from the expected List"
        );
    }
    #[test]
    fn test_invalid_entries() {
        let db_conn = help_test_setup();
        let dsk = "1234".to_string();
        let provision = SmartStartEntry {
            DSK: dsk.clone(),
            Payload: json!({
                 "DSK": dsk,
                 "ProtocolControllerUnid": "",
                 "Unid":""}),
        };
        db_upsert_entry(&db_conn, provision);
        assert_eq!(
            "[]",
            serde_json::to_string(&db_list_provisions(&db_conn)).unwrap()
        );
        let provision = SmartStartEntry {
            DSK: dsk.clone(),
            Payload: json!({
                 "DSK": dsk,
                 "Include":false,
                 "Unid":""}),
        };
        db_upsert_entry(&db_conn, provision);
        assert_eq!(
            "[]",
            serde_json::to_string(&db_list_provisions(&db_conn)).unwrap()
        );
        let provision = SmartStartEntry {
            DSK: dsk.clone(),
            Payload: json!({
                 "DSK": dsk,
                 "Include":false,
                 "ProtocolControllerUnid": ""}),
        };
        db_upsert_entry(&db_conn, provision);
        assert_eq!(
            "[]",
            serde_json::to_string(&db_list_provisions(&db_conn)).unwrap()
        );
    }
    #[test]
    /// Generate List from
    /// - a changed row
    /// - more rows
    fn test_list_changed() {
        let db_conn = help_test_setup();
        // We want to use pv42 to modify the DB, so make it mutable.
        let mut pv42 = help_test_insert(&db_conn, "42".to_string());
        let list = db_list_provisions(&db_conn);
        let exp_list: Vec<serde_json::Value> = vec![pv42.clone().into()];
        assert_eq!(
            serde_json::to_string(&list).unwrap(),
            serde_json::to_string(&exp_list).unwrap()
        );
        // Update entry and list again
        pv42.Payload = json!({
            "DSK": "42",
            "Unid": "I am unique",
            "Include":true,
            "ProtocolControllerUnid": "DEADBEEF-2"
        });
        db_upsert_entry(&db_conn, pv42.clone());
        let list = db_list_provisions(&db_conn);
        let value = serde_json::json!([{"DSK":"42",
                                         "Include":true,
                                         "ProtocolControllerUnid":"DEADBEEF-2",
                                         "Unid":"I am unique"}]
        );
        assert_eq!(
            serde_json::to_string(&list).unwrap(),
            serde_json::to_string(&value).unwrap(),
            "The List found in the database with a modified element\
                            is different from the expected List."
        );
        // Update entry and list again
        let mut pv42_update = pv42.clone();
        pv42_update.Payload = json!({
            "DSK": "42",
            "Include":false,
        });
        pv42.Payload["Include"] = serde_json::Value::Bool(false);
        db_upsert_entry(&db_conn, pv42_update);
        let value = serde_json::json!([{"DSK":"42",
                                          "Include":false,
                                          "ProtocolControllerUnid":"DEADBEEF-2",
                                          "Unid":"I am unique"}]);
        let list = db_list_provisions(&db_conn);
        assert_eq!(
            serde_json::to_string(&list).unwrap(),
            serde_json::to_string(&value).unwrap(),
            "The List found in the database with a modified element\
                            is different from the expected List."
        );
        // Add an element and list again
        let pv24 = help_test_insert(&db_conn, "24".to_string());
        let list = db_list_provisions(&db_conn);
        let exp_list: Vec<serde_json::Value> = [pv42.into(), pv24.into()].to_vec();
        assert_eq!(
            serde_json::to_string(&list).unwrap(),
            serde_json::to_string(&exp_list).unwrap(),
            "The List found in the database with two elements\
                            is different from the expected List."
        );
    }
}
