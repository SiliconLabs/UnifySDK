// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

//! Cache is an object that bookkeeps UNID, endpoint ID and it's attributes
//! (name and location). The storage is implemented with SQLite DB - so it is persistent.

use rusqlite::{params, Connection, Error, Statement};
use unify_config_sys::*;
use unify_log_sys::*;

pub const CONFIG_KEY_NAL_DB_FILE: &str = "nal.datastore_file";

declare_app_name!("cache");

#[derive(Default, Clone, PartialEq, Debug)]
pub struct NALConfig {
    db_file: String,
}

impl ConfigLoader for NALConfig {
    fn from_config() -> Result<Self, unify_config_sys::config_status_t>
    where
        Self: Sized,
    {
        let db_file = config_get_as_string(CONFIG_KEY_NAL_DB_FILE)?;
        Ok(NALConfig { db_file: db_file })
    }
}

#[derive(PartialEq, Eq, PartialOrd, Ord, Clone, Default, Debug)]
pub struct NodeEpKey {
    pub unid: String,
    pub ep_id: u16,
}

#[derive(Clone, PartialEq, Debug, Default)]
pub struct Attributes {
    pub name_desired: String,
    pub name_reported: String,
    pub location_desired: String,
    pub location_reported: String,
    pub is_proxied: bool,
}

/// Checks that the unid field of key struct (of type NodeEpKey)
/// is not an empty string. If it is empty - returns error.
macro_rules! CHECK_UNID_NOT_EMPTY {
    ($key:ident) => {
        if $key.unid.is_empty() {
            return Err("UNID can't be empty".to_string());
        }
    };
}

pub struct Cache {
    db_conn: rusqlite::Connection,
}

impl Cache {
    /// Creates a new Cache instance. Try to open a database file.
    /// If file not exists - try to create it.
    /// If there already present "nal_map" table and "idx_key" in DB - does nothing.
    /// If not - creates it.
    pub fn new(nal_config: NALConfig) -> Self {
        log_info!("Using database file {}", &nal_config.db_file);
        // if NAL cannot open the database, it exits with an error message
        let db_conn = Connection::open(&nal_config.db_file).expect("Cannot open database");
        // we need (unid, endpoint_id) index to perform the "on conflict (unid, endpoint_id) do update set"
        // operation, based on 2 keys (unid and endpoint_id)
        db_conn
            .execute(
                "CREATE TABLE IF NOT EXISTS nal_map (
                unid              TEXT NOT NULL,
                endpoint_id       INTEGER NOT NULL,
                name_desired      TEXT,
                name_reported     TEXT,
                location_desired  TEXT,
                location_reported TEXT,
                is_proxied        TEXT,
                UNIQUE(unid, endpoint_id),
                CHECK(unid <> '')
            );",
                params![],
            )
            .expect("Cannot create nal_map table");

        db_conn
            .execute(
                "CREATE TABLE IF NOT EXISTS nal_delayed_messages (
                unid    TEXT NOT NULL,
                topic   TEXT NOT NULL,
                payload TEXT NOT NULL,
                CHECK(unid <> ''),
                CHECK(topic <> '')
            );",
                params![],
            )
            .expect("Cannot create nal_delayed_messages table");

        db_conn
            .execute("DELETE from nal_delayed_messages;", params![])
            .expect("Cannot clean up nal_delayed_messages table");

        let cache = Cache { db_conn: db_conn };

        cache
    }

    /// Gets a single value from DB row.
    fn db_get_single_row_val(&mut self, key: &NodeEpKey, column_name: &str) -> String {
        let query_str = format!(
            "SELECT {} from nal_map WHERE unid = '{}' AND endpoint_id = {}",
            column_name, &key.unid, key.ep_id
        );

        if let Ok(db_payload) = self.db_conn.query_row(query_str.as_str(), [], |row| {
            let result: Result<String, Error> = row.get(0);
            result
        }) {
            return String::from(db_payload.as_str());
        }

        return String::from("");
    }

    /// Checks if node presents in cache.
    pub fn node_presents(&mut self, key: &NodeEpKey) -> bool {
        // unid column in DB can't be empty
        // if db_get_single_row_val(..., "unid") function returned empty string - no node with such key exists in database
        let unid = self.db_get_single_row_val(key, "unid");
        return !(unid == String::from(""));
    }

    /// Gets a value for the given key.
    /// If node with such key doesn't present in DB - returns an error.
    pub fn node_get(
        &mut self,
        key: &NodeEpKey,
        value: &mut Attributes,
    ) -> std::result::Result<(), String> {
        CHECK_UNID_NOT_EMPTY!(key);

        if self.node_presents(key) {
            value.name_desired = self.db_get_single_row_val(key, "name_desired");
            value.name_reported = self.db_get_single_row_val(key, "name_reported");
            value.location_desired = self.db_get_single_row_val(key, "location_desired");
            value.location_reported = self.db_get_single_row_val(key, "location_reported");
            value.is_proxied =
                if self.db_get_single_row_val(key, "is_proxied") == String::from("true") {
                    true
                } else {
                    false
                };
        } else {
            return Err(format!(
                "Can't find node '{}', ep ID '{}'",
                key.unid, key.ep_id
            ));
        }

        Ok(())
    }

    /// Updates a value for the given key.
    /// If node with such key doesn't present in DB - creates it.
    pub fn node_set(
        &mut self,
        key: &NodeEpKey,
        attributes_new: &Attributes,
    ) -> std::result::Result<(), String> {
        CHECK_UNID_NOT_EMPTY!(key);

        let query_str = format!(
            "INSERT INTO nal_map (unid, endpoint_id, name_desired, name_reported, location_desired, location_reported, is_proxied) VALUES
             ('{unid}', {ep_id}, '{name_desired}', '{name_reported}', '{location_desired}', '{location_reported}', '{is_proxied}')
             on conflict (unid, endpoint_id) do update set
             name_desired      = '{name_desired}',
             name_reported     = '{name_reported}',
             location_desired  = '{location_desired}',
             location_reported = '{location_reported}',
             is_proxied        = '{is_proxied}'
             ;",
            unid              = &key.unid,
            ep_id             = &key.ep_id,
            name_desired      = &attributes_new.name_desired,
            name_reported     = &attributes_new.name_reported,
            location_desired  = &attributes_new.location_desired,
            location_reported = &attributes_new.location_reported,
            is_proxied = if attributes_new.is_proxied {"true"} else {"false"}
        );

        match self.db_conn.execute(&query_str, []) {
            Ok(_) => log_info!(
                "Updated database for node key ({}, {}).",
                &key.unid,
                &key.ep_id
            ),
            Err(err) => log_error!(
                "Error {} when updating table with node key ({}, {}).",
                err,
                &key.unid,
                &key.ep_id
            ),
        }

        Ok(())
    }

    /// Removes a node from the cache.
    /// Here is no necessity to check if node presents in cache.
    pub fn node_remove(&mut self, key: &NodeEpKey) -> std::result::Result<(), String> {
        let query_str = format!(
            "DELETE from nal_map where unid = '{unid}' AND endpoint_id = {ep_id}",
            unid = &key.unid,
            ep_id = &key.ep_id,
        );

        match self.db_conn.execute(&query_str, []) {
            Ok(_) => log_info!("Removed node with key ({}, {}).", &key.unid, &key.ep_id),
            Err(err) => log_error!(
                "Error {} when removing node with key ({}, {}).",
                err,
                &key.unid,
                &key.ep_id
            ),
        }

        Ok(())
    }

    /// Returns a vector of all nodes that present in database
    pub fn get_all_nodes(&mut self) -> Result<Vec<NodeEpKey>, Error> {
        let query_str = format!("SELECT unid, endpoint_id FROM nal_map");
        let mut stmt: Statement = self.db_conn.prepare(query_str.as_str())?;
        let mut rows = stmt.query([])?;

        let mut ret_keys = Vec::new();
        while let Some(row) = rows.next()? {
            let ep_id_i32: i32 = row.get(1)?;

            let key = NodeEpKey {
                unid: row.get(0)?,
                ep_id: ep_id_i32 as u16,
            };

            log_debug!("got such node: '{}, {}'", key.unid, key.ep_id);

            ret_keys.push(key.clone());
        }

        Ok(ret_keys)
    }

    /// Returns a vector of node keys with requested UNID that presence in DB.
    pub fn get_all_nodes_match_name(&mut self, unid: &str) -> Result<Vec<NodeEpKey>, Error> {
        let query_str = format!(
            "SELECT unid, endpoint_id FROM nal_map WHERE unid = '{}'",
            unid
        );
        let mut stmt: Statement = self.db_conn.prepare(query_str.as_str())?;
        let mut rows = stmt.query([])?;

        let mut ret_keys = Vec::new();
        while let Some(row) = rows.next()? {
            let ep_id_i32: i32 = row.get(1)?;

            let key = NodeEpKey {
                unid: row.get(0)?,
                ep_id: ep_id_i32 as u16,
            };
            ret_keys.push(key.clone());
        }

        Ok(ret_keys)
    }

    /// Adds a delayed message row to the nal_delayed_messages table
    pub fn add_delayed_message(
        &mut self,
        unid: &str,
        topic: &str,
        payload: &str,
    ) -> std::result::Result<(), String> {
        let query_str = format!(
            "INSERT INTO nal_delayed_messages (unid, topic, payload) VALUES ('{unid}', '{topic}', '{payload}');",
            unid    = &unid,
            topic   = &topic,
            payload = &payload
        );

        match self.db_conn.execute(&query_str, []) {
            Ok(_) => return Ok(()),
            Err(_) => return Err(format!("Can't execute such query: '{}'", query_str)),
        }
    }

    /// Gets all delayed messages data (topic, payload) with requested UNID.
    /// Deletes all these messages trom the table.
    pub fn get_all_delayed_messages(&mut self, unid: &str) -> Result<Vec<(String, String)>, Error> {
        let query_str = format!(
            "SELECT topic, payload FROM nal_delayed_messages WHERE unid = '{}'",
            unid
        );
        let mut stmt: Statement = self.db_conn.prepare(query_str.as_str())?;
        let mut rows = stmt.query([])?;

        let mut ret_msgs = Vec::new();
        while let Some(row) = rows.next()? {
            let topic: String = row.get(0)?;
            let payload: String = row.get(1)?;
            ret_msgs.push((topic.clone(), payload.clone()));
        }

        // the messages we got will be processed - so now we have to delete them
        let delete_query_str = format!("DELETE from nal_delayed_messages where unid = '{}'", unid);
        self.db_conn.execute(&delete_query_str, [])?;

        Ok(ret_msgs)
    }
}

#[cfg(test)]

mod tests {
    use super::*;
    // The default Rust behavior is to run unit tests in parallel.
    // Here we have a shared resource (database, open in memory) and tests should run
    // sequentially to avoid data races (also, we may use synchronization primitives, but this is overhead).
    use serial_test::serial;

    impl NALConfig {
        /// this function should be used in unit tests ONLY
        pub fn new_for_unit_tests() -> Self {
            NALConfig {
                db_file: String::from(":memory"),
            }
        }
    }

    impl Cache {
        /// this function should be used in unit tests ONLY
        pub fn clean_up_db_for_unit_tests(&mut self) {
            self.db_conn
                .execute("DELETE from nal_map;", params![])
                .expect("Cannot clean up nal_map table");

            self.db_conn
                .execute("DELETE from nal_delayed_messages;", params![])
                .expect("Cannot clean up nal_delayed_messages table");
        }
    }

    #[test]
    #[serial]
    fn test_node_add() {
        let mut cache = Cache::new(NALConfig::new_for_unit_tests());
        cache.clean_up_db_for_unit_tests();

        let key = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };
        let attributes_desired = Attributes {
            name_desired: String::from("test_name_desired"),
            name_reported: String::from("test_name_reported"),
            location_desired: String::from("test_location_desired"),
            location_reported: String::from("test_location_reported"),
            is_proxied: false,
        };

        assert_eq!(cache.node_set(&key, &attributes_desired), Ok(()));

        let mut attributes_reported = Attributes::default();
        assert_eq!(cache.node_get(&key, &mut attributes_reported), Ok(()));

        assert_eq!(attributes_reported, attributes_desired);
    }

    #[test]
    #[serial]
    fn test_node_add_with_empty_unid() {
        let mut cache = Cache::new(NALConfig::new_for_unit_tests());
        cache.clean_up_db_for_unit_tests();

        let key = NodeEpKey {
            unid: String::from(""),
            ep_id: 0,
        };
        let attributes = Attributes::default();

        assert_eq!(
            cache.node_set(&key, &attributes),
            Err(String::from("UNID can't be empty"))
        );
    }

    #[test]
    #[serial]
    fn test_node_remove() {
        let mut cache = Cache::new(NALConfig::new_for_unit_tests());
        cache.clean_up_db_for_unit_tests();

        let key = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };
        let attributes_desired = Attributes {
            name_desired: String::from("test_name_desired"),
            name_reported: String::from("test_name_reported"),
            location_desired: String::from("test_location_desired"),
            location_reported: String::from("test_location_reported"),
            is_proxied: false,
        };

        assert_eq!(cache.node_set(&key, &attributes_desired), Ok(()));

        assert_eq!(cache.node_remove(&key), Ok(()));

        let mut attributes_reported = Attributes::default();
        let res = cache.node_get(&key, &mut attributes_reported);

        assert_eq!(
            res,
            Err(format!(
                "Can't find node '{}', ep ID '{}'",
                key.unid, key.ep_id
            ))
        );
    }

    #[test]
    #[serial]
    fn test_node_set() {
        let mut cache = Cache::new(NALConfig::new_for_unit_tests());
        cache.clean_up_db_for_unit_tests();

        let key = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };
        let attributes_1 = Attributes {
            name_desired: String::from("test_name_desired"),
            name_reported: String::from("test_name_reported"),
            location_desired: String::from("test_location_desired"),
            location_reported: String::from("test_location_reported"),
            is_proxied: false,
        };
        let attributes_2 = Attributes {
            name_desired: String::from("new_name_desired"),
            name_reported: String::from("new_name_reported"),
            location_desired: String::from("new_location_desired"),
            location_reported: String::from("new_location_reported"),
            is_proxied: false,
        };

        assert_eq!(cache.node_set(&key, &attributes_1), Ok(()));

        assert_eq!(cache.node_set(&key, &attributes_2), Ok(()));

        let mut attributes_reported = Attributes::default();
        assert_eq!(cache.node_get(&key, &mut attributes_reported), Ok(()));

        assert_eq!(attributes_reported, attributes_2);
    }

    #[test]
    #[serial]
    fn test_node_presents() {
        let mut cache = Cache::new(NALConfig::new_for_unit_tests());
        cache.clean_up_db_for_unit_tests();

        let key_exist = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };
        let key_not_exist = NodeEpKey {
            unid: String::from("test_node_2"),
            ep_id: 0,
        };
        let attributes = Attributes {
            name_desired: String::from("test_name_desired"),
            name_reported: String::from("test_name_reported"),
            location_desired: String::from("test_location_desired"),
            location_reported: String::from("test_location_reported"),
            is_proxied: false,
        };

        assert_eq!(cache.node_set(&key_exist, &attributes), Ok(()));

        assert_eq!(cache.node_presents(&key_exist), true);
        assert_eq!(cache.node_presents(&key_not_exist), false);
    }

    #[test]
    #[serial]
    fn test_node_get() {
        let mut cache = Cache::new(NALConfig::new_for_unit_tests());
        cache.clean_up_db_for_unit_tests();

        let key_1 = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };
        let attributes_1 = Attributes {
            name_desired: String::from("test_name_desired_1"),
            name_reported: String::from("test_name_reported_1"),
            location_desired: String::from("test_location_desired_1"),
            location_reported: String::from("test_location_reported_1"),
            is_proxied: false,
        };
        let key_2 = NodeEpKey {
            unid: String::from("test_node_2"),
            ep_id: 0,
        };
        let attributes_2 = Attributes {
            name_desired: String::from("test_name_desired_2"),
            name_reported: String::from("test_name_reported_2"),
            location_desired: String::from("test_location_desired_2"),
            location_reported: String::from("test_location_reported_2"),
            is_proxied: false,
        };

        assert_eq!(cache.node_set(&key_1, &attributes_1), Ok(()));
        assert_eq!(cache.node_set(&key_2, &attributes_2), Ok(()));

        let mut attributes_reported = Attributes::default();
        assert_eq!(cache.node_get(&key_1, &mut attributes_reported), Ok(()));

        assert_eq!(attributes_reported, attributes_1);
    }

    #[test]
    #[serial]
    fn test_get_all_nodes_match_name() {
        let mut cache = Cache::new(NALConfig::new_for_unit_tests());
        cache.clean_up_db_for_unit_tests();

        let key_1_0 = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };
        let key_1_1 = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 1,
        };
        let key_1_2 = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 2,
        };
        let key_2 = NodeEpKey {
            unid: String::from("test_node_2"),
            ep_id: 0,
        };
        let key_3 = NodeEpKey {
            unid: String::from("test_node_3"),
            ep_id: 0,
        };
        let attributes = Attributes {
            name_desired: String::from("test_name_desired_1"),
            name_reported: String::from("test_name_reported_1"),
            location_desired: String::from("test_location_desired_1"),
            location_reported: String::from("test_location_reported_1"),
            is_proxied: false,
        };

        assert_eq!(cache.node_set(&key_1_0, &attributes), Ok(()));
        assert_eq!(cache.node_set(&key_1_1, &attributes), Ok(()));
        assert_eq!(cache.node_set(&key_1_2, &attributes), Ok(()));
        assert_eq!(cache.node_set(&key_2, &attributes), Ok(()));
        assert_eq!(cache.node_set(&key_3, &attributes), Ok(()));

        assert_eq!(
            cache.get_all_nodes_match_name("test_node_1").unwrap(),
            vec![key_1_0, key_1_1, key_1_2]
        );
    }
}
