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

//! Cache is an object that bookkeeps UNID, endpoint_ID and it's attributes
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

/// # Description
///
/// Checks that the unid field of key struct (of type NodeEpKey) is not an empty string.
/// If it is empty - returns error.
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
    /// # Description
    ///
    /// Creates a new Cache instance. Try to open a database file.
    /// If file not exists - try to create it.
    /// If tables in database not present - creates them.
    ///
    /// # Arguments
    ///
    /// * _nal_config_ - path to database file is given from here
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
                priority INTEGER NOT NULL,
                CHECK(unid <> ''),
                CHECK(topic <> '')
            );",
                params![],
            )
            .expect("Cannot create nal_delayed_messages table");

        db_conn
            .execute(
                "CREATE TABLE IF NOT EXISTS unids (
                unid TEXT NOT NULL,
                endpoint_id_list_received INTEGER NOT NULL,
                CHECK(unid <> '')
            );",
                params![],
            )
            .expect("Cannot create unids table");

        db_conn
            .execute("DELETE from nal_delayed_messages;", params![])
            .expect("Cannot clean up nal_delayed_messages table");

        let cache = Cache { db_conn: db_conn };

        cache
    }

    // "nal_map" table API

    /// # Description
    ///
    /// Gets a single value from a row from `nal_map` table.
    ///
    /// # Arguments
    ///
    /// * _key_ - UNID and endpoint_id to search a node are given from here
    /// * _column_name_ - the name of column in a table to get the data from
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

    /// # Description
    ///
    /// Checks if node presents in database.
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

    /// # Description
    ///
    /// Updates attributes of node with provided key.
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
            Ok(_) => Ok(()),
            Err(err) => Err(format!(
                "Failed to update 'nal_map' table with node with key ({}, {}), error: '{}'.",
                &key.unid, &key.ep_id, err
            )),
        }
    }

    /// # Description
    ///
    /// Removes a node from the database.
    /// Here is no necessity to check if node presents in database.
    pub fn node_remove(&mut self, key: &NodeEpKey) -> std::result::Result<(), String> {
        let query_str = format!(
            "DELETE from nal_map where unid = '{unid}' AND endpoint_id = {ep_id}",
            unid = &key.unid,
            ep_id = &key.ep_id,
        );

        match self.db_conn.execute(&query_str, []) {
            Ok(_) => Ok(()),
            Err(err) => Err(format!(
                "Failed to remove from 'nal_map' table  node with key ({}, {}), error: '{}'.",
                &key.unid, &key.ep_id, err
            )),
        }
    }

    /// # Description
    ///
    /// Returns a vector of all nodes that present in database.
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

            ret_keys.push(key.clone());
        }

        Ok(ret_keys)
    }

    /// # Description
    ///
    /// Returns a vector of tuples (UNID, endpoint_id).
    /// The vector contains all nodes that present in database with provided UNID.
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

    // "nal_delayed_messages" table API

    /// # Description
    ///
    /// Adds a delayed message to `nal_delayed_messages` table.
    pub fn add_delayed_message(
        &mut self,
        unid: &str,
        topic: &str,
        payload: &str,
        priority: u32,
    ) -> std::result::Result<(), String> {
        let query_str = format!(
            "INSERT INTO nal_delayed_messages (unid, topic, payload, priority) VALUES ('{unid}', '{topic}', '{payload}', {priority});",
            unid     = &unid,
            topic    = &topic,
            payload  = &payload,
            priority = priority
        );

        match self.db_conn.execute(&query_str, []) {
            Ok(_) => return Ok(()),
            Err(err) => {
                return Err(format!(
                    "Can't execute such query: '{}', error: '{}'",
                    query_str, err
                ))
            }
        }
    }

    /// # Description
    ///
    /// Returns a vector of tuples (topic, payload).
    /// Tuples are contains delayed messages with provided UNID, sorted by `priority` value in ascendent order.
    pub fn get_all_delayed_messages(&mut self, unid: &str) -> Result<Vec<(String, String)>, Error> {
        let query_str = format!("SELECT topic, payload FROM nal_delayed_messages WHERE unid = '{}' ORDER BY priority ASC", unid);
        let mut stmt: Statement = self.db_conn.prepare(query_str.as_str())?;
        let mut rows = stmt.query([])?;

        let mut ret_msgs = Vec::new();
        while let Some(row) = rows.next()? {
            let topic: String = row.get(0)?;
            let payload: String = row.get(1)?;
            ret_msgs.push((topic.clone(), payload.clone()));
        }

        Ok(ret_msgs)
    }

    /// # Description
    ///
    /// Deletes all delayed messages with provided UNID from `delayed_messages` table.
    pub fn delete_all_delayed_messages(&mut self, unid: &str) -> Result<(), String> {
        let delete_query_str = format!("DELETE FROM nal_delayed_messages WHERE unid = '{}';", unid);

        match self.db_conn.execute(&delete_query_str, []) {
            Ok(_) => return Ok(()),
            Err(err) => {
                return Err(format!(
                    "Can't delete delayed messages for UNID '{}', error: '{}'",
                    unid, err
                ))
            }
        }
    }

    // "unids" table API

    /// # Description
    ///
    /// Checks if a State topic has been received for specified UNID
    /// (if it was - the row with such `unid` value should present in `unids` table).
    pub fn is_unid_registered(&mut self, unid: &str) -> bool {
        let query_str = format!("SELECT unid from unids");
        let mut stmt: Statement = self.db_conn.prepare(query_str.as_str()).unwrap();
        let mut rows = stmt.query([]).unwrap();

        while let Some(row) = rows.next().unwrap() {
            let unid_got: String = row.get(0).unwrap();

            if String::from(unid) == unid_got {
                return true;
            }
        }

        return false;
    }

    /// # Description
    ///
    /// Checks if an EndpointIdList topic has been received for specified UNID
    /// (if it was - the row with such `unid` value should present in `unids` table and it's `endpoint_id_list_received` value should be `1`).
    pub fn is_epidlist_received(&mut self, unid: &str) -> bool {
        let query_str = format!(
            "SELECT endpoint_id_list_received FROM unids WHERE unid = '{}';",
            unid
        );
        let mut stmt: Statement = self.db_conn.prepare(query_str.as_str()).unwrap();
        let mut rows = stmt.query([]).unwrap();

        let row = rows.next().unwrap();

        // this is_none() check prevents error that should never happen in production usage, but it occurs
        // in unit-tests - so don't pay attention
        if row.is_none() {
            return false;
        }

        let value: i32 = row.unwrap().get(0).unwrap();

        if value == 1 {
            return true;
        }

        return false;
    }

    /// # Description
    ///
    /// Register UNID, received with a State topic.
    /// Adds a row to `unids` table with `endpoint_id_list_received` set to 0.
    /// This function should be called from the State topic handler only.
    pub fn register_unid(&mut self, unid: &str) -> std::result::Result<(), String> {
        let query_str = format!(
            "INSERT INTO unids (unid, endpoint_id_list_received) VALUES ('{}', 0);",
            unid
        );

        match self.db_conn.execute(&query_str, []) {
            Ok(_) => Ok(()),
            Err(err) => Err(format!(
                "Failed to add to 'unids' table UNID '{}', error: '{}'.",
                unid, err
            )),
        }
    }

    /// # Description
    ///
    /// Register receiving EndpointIdList for specified UNID.
    /// Sets `endpoint_id_list_received` value of row with specified UNID to `1`.
    /// This function should be called from the EndpointIdList topic handler only.
    pub fn register_received_epidlist(&mut self, unid: &str) -> std::result::Result<(), String> {
        let query_str = format!(
            "UPDATE unids SET endpoint_id_list_received = 1 WHERE unid = '{}';",
            unid
        );

        match self.db_conn.execute(&query_str, []) {
            Ok(_) => Ok(()),
            Err(err) => Err(format!(
                "Failed to update 'unids' table with UNID '{}', error: '{}'.",
                unid, err
            )),
        }
    }

    /// # Description
    ///
    /// Removes a row with specified UNID from the `unids` table.
    pub fn remove_unid(&mut self, unid: &str) -> std::result::Result<(), String> {
        let delete_query_str = format!("DELETE from unids where unid = '{}'", unid);

        match self.db_conn.execute(&delete_query_str, []) {
            Ok(_) => Ok(()),
            Err(err) => Err(format!(
                "Failed to delete a row with UNID '{}' from 'unids' table, error: '{}'.",
                unid, err
            )),
        }
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
                db_file: String::from(":memory:"),
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

            self.db_conn
                .execute("DELETE from unids;", params![])
                .expect("Cannot clean up unids table");
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

    #[test]
    #[serial]
    fn test_delayed_messages() {
        let mut cache = Cache::new(NALConfig::new_for_unit_tests());
        cache.clean_up_db_for_unit_tests();

        let unid_1 = "test_node_1";
        let unid_2 = "test_node_2";
        let topic_1_1 = "ucl/by-unid/test_node_1/ep1/NameAndLocation/Commands/WriteAttributes";
        let topic_1_2 = "ucl/by-unid/test_node_1/ep2/NameAndLocation/Commands/WriteAttributes";
        let topic_2_1 = "ucl/by-unid/test_node_2/ep1/NameAndLocation/Commands/WriteAttributes";
        let topic_2_2 = "ucl/by-unid/test_node_2/ep2/NameAndLocation/Commands/WriteAttributes";
        let msg_1_1 = "payload_1_1";
        let msg_1_2 = "payload_1_2";
        let msg_2_1 = "payload_2_1";
        let msg_2_2 = "payload_2_2";

        assert_eq!(
            cache.add_delayed_message(unid_1, topic_1_1, msg_1_1, 1),
            Ok(())
        );
        assert_eq!(
            cache.add_delayed_message(unid_1, topic_1_2, msg_1_2, 0),
            Ok(())
        );
        assert_eq!(
            cache.add_delayed_message(unid_2, topic_2_1, msg_2_1, 0),
            Ok(())
        );
        assert_eq!(
            cache.add_delayed_message(unid_2, topic_2_2, msg_2_2, 1),
            Ok(())
        );

        // test insert messages
        {
            let vec_1_real: Vec<(String, String)> = cache.get_all_delayed_messages(unid_1).unwrap();
            let vec_2_real: Vec<(String, String)> = cache.get_all_delayed_messages(unid_2).unwrap();

            let vec_1_expected: Vec<(String, String)> = vec![
                (String::from(topic_1_2), String::from(msg_1_2)),
                (String::from(topic_1_1), String::from(msg_1_1)),
            ];
            let vec_2_expected: Vec<(String, String)> = vec![
                (String::from(topic_2_1), String::from(msg_2_1)),
                (String::from(topic_2_2), String::from(msg_2_2)),
            ];

            assert_eq!(vec_1_real, vec_1_expected);
            assert_eq!(vec_2_real, vec_2_expected);
        }

        // test deletion of messages
        {
            assert_eq!(cache.delete_all_delayed_messages(unid_1), Ok(()));

            let vec_1_real: Vec<(String, String)> = cache.get_all_delayed_messages(unid_1).unwrap();
            let vec_2_real: Vec<(String, String)> = cache.get_all_delayed_messages(unid_2).unwrap();

            let vec_1_expected: Vec<(String, String)> = vec![];
            let vec_2_expected: Vec<(String, String)> = vec![
                (String::from(topic_2_1), String::from(msg_2_1)),
                (String::from(topic_2_2), String::from(msg_2_2)),
            ];

            assert_eq!(vec_1_real, vec_1_expected);
            assert_eq!(vec_2_real, vec_2_expected);
        }
    }

    #[test]
    #[serial]
    fn test_unids() {
        let mut cache = Cache::new(NALConfig::new_for_unit_tests());
        cache.clean_up_db_for_unit_tests();

        let unid_1 = "test_node_1";
        let unid_2 = "test_node_2";

        assert_eq!(cache.register_unid(unid_1), Ok(()));
        assert_eq!(cache.register_unid(unid_2), Ok(()));

        assert_eq!(cache.is_unid_registered(unid_1), true);
        assert_eq!(cache.is_unid_registered(unid_2), true);

        assert_eq!(cache.remove_unid(unid_2), Ok(()));

        assert_eq!(cache.is_unid_registered(unid_1), true);
        assert_eq!(cache.is_unid_registered(unid_2), false);
    }
}
