// License
// <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

//!We use the mosquitto_client crate for the connection to the broker.
//!
//!This module contains the main event loop of the UPVL client.
//!
//!It also provides a bit more wrapping and customization
//!specifically to the UPVL Client features.
//!
//!Other crates with MQTT client implementations are available,
//!including some implemented in Rust,
//!but their interfaces may be different.
//!
//!Other crates:
//! - <https://github.com/eclipse/paho.mqtt.rust> (wrapper for the C
//! client, claims support for v5 from version 0.7.0, SSL)
//!
//! - <https://crates.io/crates/actix-mqtt-client> (active, at least)

//! - <https://docs.rs/crate/rumqtt/0.31.0>

//! - <https://github.com/inre/rust-mq> (Rust client, does not look
//! active)

//! - <https://github.com/fluffysquirrels/mqtt-async-client-rs> (Rust
//! client, so far 3.1.1, relatively new, MIT License)

// Import the Rust mosquitto interface
extern crate mosquitto_client as mosq;

// Use some "standard libraries"

// Support pretty-printing of the structs, too
use std::fmt;
use std::str;

// The test thread uses time and thread.
// To run the demo, comment out the #[cfg(test)]
#[cfg(test)]
use std::{thread, time};

// Import the other locally defined modules and types

// The data store module
use crate::upvl_db;

// The json parser/generator module.
// The json structures are used in the interface between mqtt and data-store.
use crate::upvl_json;
use std::path::Path;
// Constants

/// Logging helper
/// TODO: fancier logging
// Constants should be upper case, otherwise, the compiler gives a warning.
const SUBS_PREFIX: &str = "Subscriber: ";
/// Logging helper
const PUB_PREFIX: &str = "Publisher: ";
/// The default upvl_mqtt log tag
const LOG_TAG: &str = "upvl_mqtt";

// DATA TYPES

/// A data-type for the data to send to the mqtt-client Callbacks object.
/// The data is available as the first argument to the callback.
//pub struct UpvlMqttSessionData <'a,'b> {
pub struct UpvlMqttSessionData<'a, 'b> {
    /// Number of published messages
    pub pub_count: i32,
    /// received messages count
    pub rec_count: i32,
    /// number of received messages dropped because of errors
    pub rec_drops: i32,
    /// The topic matchers returned when subscribing to the topics.
    /// These do not exist before there is a connection, so they are
    /// wrapped in an Option.
    /// TODO: it might be simpler to get rid of these and do our own
    /// matching.
    pub subscribers: Option<UpvlSubscribers<'b>>,
    /// A reference to the data store.
    pub db_conn: &'a rusqlite::Connection,
}

// Methods for the data type
impl<'a, 'b> UpvlMqttSessionData<'a, 'b> {
    pub fn new(db_conn: &'a rusqlite::Connection) -> UpvlMqttSessionData<'a, 'b> {
        UpvlMqttSessionData {
            pub_count: 0,
            rec_count: 0,
            rec_drops: 0,
            subscribers: None,
            db_conn,
        }
    }
}

/// Pretty-printer for UpvlMqttSessionData, so that the whole struct
/// can be printed without accessing the fields in the println!
/// statement.
impl<'a, 'b> fmt::Display for UpvlMqttSessionData<'a, 'b> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        // The newline is also printed.
        write!(
            f,
            "Published {} messages.
Received {} messages of which {} were dropped.",
            self.pub_count, self.rec_count, self.rec_drops
        )
    }
}

/// The subscribers returned by mosquitto_client when we subscribe to
/// a topic.  We only use this to access the topic-matching function.
pub struct UpvlSubscribers<'a> {
    pub update_subscriber: mosq::TopicMatcher<'a>,
    pub remove_subscriber: mosq::TopicMatcher<'a>,
}

/// Type to hold a connection to the broker.
#[derive(std::clone::Clone)]
pub struct UpvlMqttSession {
    /// We use the mosquitto_client crate for the connection to the broker.
    mcl: mosq::Mosquitto,
    qos: u32,
    port: u32,
    broker: String,
}

/// Methods for the UpvlMqttSession type
impl UpvlMqttSession {
    /// Constructor
    /// Set up the connection to the broker
    pub fn new(
        broker: &str,
        port: u32,
        cafile: &str,
        certfile: &str,
        keyfile: &str,
    ) -> UpvlMqttSession {
        // Create a new mosquitto client instance and assign it to an
        // immutable variable.
        let mcl = mosq::Mosquitto::new("upvl-1");

        let passphrase: Option<&str> = Some("");

        if !cafile.is_empty() && !certfile.is_empty() && !keyfile.is_empty() {
            let cafilepath = Path::new(cafile);
            let certfilepath = Path::new(certfile);
            let keyfilepath = Path::new(keyfile);

            if !cafilepath.exists() || !certfilepath.exists() || !keyfilepath.exists() {
                upvl_log::log_critical(
                    LOG_TAG,
                    "One or more of the certificate files does not exist \n".to_string(),
                );
                panic!("Application terminated!");
            }
            match mcl.tls_set(&cafilepath, &certfilepath, &keyfilepath, passphrase) {
                //    .expect();
                Ok(_) => {
                    upvl_log::log_info(LOG_TAG, "Succesfully set the TLS certifcate.".to_string())
                }
                Err(err) => {
                    upvl_log::log_critical(
                        LOG_TAG,
                        format!("Cannot setup TLS certificates, Error description: {}", err),
                    );
                    panic!("Application terminated!");
                }
            }
        } else {
            if cafile.is_empty() || certfile.is_empty() || keyfile.is_empty() {
                upvl_log::log_warning(
                    LOG_TAG,
                    "One or more of the certificate file path is blank\n".to_string(),
                );
            }

            upvl_log::log_warning(
                LOG_TAG,
                "Trying non-TLS connection to the broker\n".to_string(),
            );
        }
        // Connect the mosquitto client to localhost, port 1883
        // If connection fails, stop the process with the error
        // message "Cannot connect ..."
        // TODO: replace this with proper error handling
        match mcl.connect(broker, port, 5) {
            //    .expect("Cannot connect to broker");
            Ok(_) => upvl_log::log_info(
                LOG_TAG,
                format!("Connected to broker on {} port {}", &broker, port),
            ),
            Err(err) => {
                upvl_log::log_critical(
                    LOG_TAG,
                    format!("Cannot connect to broker, Error description: {}", err),
                );
                panic!("Application terminated!");
            }
        }
        // TODO set up a will?

        // Return the session
        UpvlMqttSession {
            mcl,
            qos: 0,
            broker: broker.to_string(),
            port,
        }
    }

    /// Subscribe to our topics.
    pub fn subscribe(&self) -> UpvlSubscribers {
        upvl_log::log_info(LOG_TAG, "Subscribing".to_string());

        // Tell mcl to subscribe to ucl/SmartStart with QoS qos.  If this
        // fails, stop the process with an error message.
        UpvlSubscribers {
            update_subscriber: self
                .mcl
                .subscribe("ucl/SmartStart/List/Update/#", self.qos)
                .expect("Cannot subscribe to provisioning entries"),
            remove_subscriber: self
                .mcl
                .subscribe("ucl/SmartStart/List/Remove/#", self.qos)
                .expect("Cannot subscribe to provisioning entries"),
        }
    }

    /// Spawn a thread that handles publishing of Updates.
    ///
    /// Note: this was used for the demo.
    ///
    /// To use it, we currently need a running broker.
    ///
    ///To use it in automatic tests, it might be possible to modify
    /// "publish" to queue up the generated topics/JSON messages in a
    /// shared queue, and then have the "subscribers" read from that.
    // To run the demo, comment out the #[cfg(test)], here and above, and
    // call this method from main.
    #[cfg(test)]
    #[allow(dead_code)]
    pub fn spawn_test_thread(&self) -> std::thread::JoinHandle<()> {
        use serde_json::json;

        // Create a Rust clone of mcl, so that we can use it in a thread.
        let mclt = self.mcl.clone();

        // We don't need to clone qos, since it is just an integer,
        // but we cannot use self in the thread, so we need a local
        // qos.
        let qos = self.qos;

        // Spawn a thread that will do the publishing.
        // The keyword 'move' moves mclt into the thread, so that is is
        // owned by the thread, not the parent.
        // Rust will 'move' all the variables used in the thread.
        thread::spawn(move || {
            let timeout = time::Duration::from_millis(2000);

            // Demonstrate how to create a json string by building the
            // corresponding structure.
            // We want to test creating a provision by publishing to
            // the Update topic.
            // We create the struct corresponding to the JSON
            // representation of a provision and let serde_json handle
            // the rest.
            let mut update_msg = upvl_json::SmartStartEntry {
                DSK: "24859-64107-46202-12845-60475-62452-54892-59867".to_string(),
                Payload: json!({
                    "DSK": "24859-64107-46202-12845-60475-62452-54892-59867",
                    "Include": true,
                    "ProtocolControllerUnid": "",
                    "Unid":""
                }),
            };

            // Test removing the provision again.  The spec uses
            // another type to represent the provision in Remove.  So
            // we have to create another JSON struct to test Remove.

            let remove_msg = r#"{
                "DSK": "24859-64107-46202-12845-60475-62452-54892-59867"
            }"#;

            let mut topic = "ucl/SmartStart/List/Update";

            // Iterate 3 times, doing slightly different things.
            for ii in 0..3 {
                // Publish one message per iteration
                let msg_str = match ii {
                    0 => serde_json::to_string(&update_msg.to_string())
                        .expect("Cannot build JSON from this structure."),
                    1 => {
                        update_msg.Payload["Include"] = serde_json::Value::Bool(false);
                        serde_json::to_string(&update_msg.to_string())
                            .expect("Cannot build JSON from this structure.")
                    }
                    2 => {
                        topic = "ucl/SmartStart/List/Remove";
                        serde_json::to_string(&remove_msg)
                            .expect("Cannot build JSON from this structure.")
                    }
                    // If we don't care about the value, we can use
                    // the '_' placeholder instead of a variable.
                    _ => panic!("Implement me for {}", ii),
                };

                // Convert the data structure to a publishable
                // sequence of chars with msg_str.into_bytes();
                mclt.publish(
                    topic,
                    &msg_str.into_bytes(),
                    // QoS setting QoS, retain false
                    qos,
                    false,
                )
                .unwrap();
                println!("Going to sleep");
                thread::sleep(timeout);
            }

            mclt.disconnect().unwrap();
        })
    }

    /// Get the callbacks handler for the mqtt event callbacks, so that
    /// you can install callbacks.
    pub fn get_callbacks<T>(&self, ses_data: T) -> mosq::Callbacks<T> {
        self.mcl.callbacks(ses_data)
    }

    /// Function to publish a SmartStart List from a JSON UpvlList.
    /// #Panics
    ///
    /// This will **panic** if the json_value cannot be converted to a string.
    ///
    pub fn publish_list(&self, json_value: Vec<serde_json::Value>) {
        let mut json_object = serde_json::Map::new();
        json_object.insert("value".to_string(), serde_json::Value::Array(json_value));

        let to_pub = serde_json::to_string(&json_object)
            .expect("Cannot build JSON from this structure.")
            .into_bytes();

        self.mcl
            .publish(
                "ucl/SmartStart/List",
                &to_pub,
                // QoS, retain false
                self.qos,
                true,
            )
            .unwrap_or_else(|err| {
                upvl_log::log_error(
                    LOG_TAG,
                    format!(
                        "MQTT publication failed with error {} {}",
                        err,
                        mosq::Error::error(&err)
                    ),
                );
                0
            });
    }

    /// Run the MQTT loop a fixed number of times.  Planned to use
    /// this in testing, but didn't get that far.
    #[cfg(test)]
    #[allow(dead_code)]
    pub fn repeat_loop(&self, repeats: i32, timeout: i32) {
        for nn in 1..repeats {
            println!("Iter {}", nn);
            self.mcl.do_loop(timeout).expect("Error in loop");
        }
    }

    /// Disconnect from this end.
    ///
    /// Catch the "not connected" error and convert it to Ok.
    pub fn disconnect(&self) -> mosq::Result<()> {
        //    pub fn disconnect(&self) {
        match self.mcl.disconnect() {
            Ok(()) => Ok(()),
            Err(err) => {
                if err.error() == mosquitto_client::sys::MOSQ_ERR_NO_CONN {
                    Ok(())
                } else {
                    upvl_log::log_error(
                        LOG_TAG,
                        format!(
                            "Disconnect failed with code: {}, message: {}",
                            err.error(),
                            err.to_string()
                        ),
                    );
                    Err(err)
                }
            }
        }
    }

    /// Run the mqtt_client main loop forever.
    pub fn loop_forever(&self) {
        self.mcl.loop_forever(200).expect("broken loop");
    }

    pub fn run(&self, ses_data: UpvlMqttSessionData) {
        // Get the callbacks handler for the mqtt event callbacks (see
        // upvl_mqtt.rs)
        let mut mc = self.get_callbacks(ses_data);

        // Create and install a closure to handle incoming message
        // The |data, msg| is the arguments.
        // The { bla,bla } is the body.
        // In other languages, this could be
        // (lambda (data, msg) (bla bla)) or
        // lambda data, msg: bla,bla
        mc.on_message(|data, msg| {
            // msg contains both the topic and the payload.
            upvl_log::log_info(
                LOG_TAG,
                format!("{}Received upvl topic {:?}", SUBS_PREFIX, msg.topic()),
            );
            // TODO: handle this error
            let subscribers = data.subscribers.as_ref().unwrap();

            // There is also a msg.text() method, but it will panic
            // if the message is illegal, so we don't want to use that.
            //TODO: pull out the message handling below.
            match str::from_utf8(msg.payload()) {
                Ok(payload) => {
                    let f = || -> std::result::Result<bool, serde_json::Error> {
                        // Check the topic
                        if subscribers.remove_subscriber.matches(&msg) {
                            // Since this is a Remove, from_str() returns
                            // type Result<upvl_json::UpvlDSK,_>
                            //
                            // If the payload is not legal json, it is an Err. If
                            // the payload is legal json, but not a UpvlDSK, it
                            // is also an Err.
                            //
                            // The error is passed to the outer context,
                            // but we handle the Ok() branch here, since
                            // it has a different JSON type than Update.
                            let json: serde_json::Value = serde_json::from_str(payload)?;

                            let provision: upvl_json::SmartStartEntry =
                                upvl_json::SmartStartEntry::from(json);

                            // Update the data store from the received message.
                            // TODO: put this in a business logic module?
                            //
                            // Currently, the interface between upvl_mqtt and
                            // upvl_db uses the upvl_json types.
                            if upvl_db::db_remove_entry(data.db_conn, &provision) == 1 {
                                // Return true to say: Publish an updated List
                                Ok(true)
                            } else {
                                Ok(false)
                            }
                        } else if subscribers.update_subscriber.matches(&msg) {
                            // Return type of from_str is Result<UpvlUpdate,_> here.
                            // We pass on the error to the outer context with ?
                            let json: serde_json::Value = serde_json::from_str(payload)?;
                            let provision: upvl_json::SmartStartEntry =
                                upvl_json::SmartStartEntry::from(json);

                            // Update the data store from the received message/topic.
                            // TODO: put this in a business logic module?
                            //
                            // TODO: If we only want to publish a List
                            // if there are changes in the database,
                            // upsert() should return whether the
                            // provision was a duplicate.
                            upvl_db::db_upsert_entry(data.db_conn, provision);
                            Ok(true)
                        } else {
                            // This shouldn't happen, since we only
                            // subscribe to these two topics.  Maybe refactor?
                            Ok(false) // Don't publish a List
                        }
                    };

                    // Apply the function we created above and publish the
                    // List topic only when everything is fine.
                    // TODO: put this in a business logic module?
                    //
                    // Doing things this way is still pretty ugly, but it
                    // reduces some of the nesting in the error handling
                    // because we can use the same handler for the two
                    // different result types of from_str().
                    match f() {
                        Ok(true) => {
                            data.rec_count += 1;
                            // Create an updated list and hand over the
                            // list to publish_list
                            let list = upvl_db::db_list_provisions(data.db_conn);
                            self.publish_list(list);
                        }
                        Ok(false) => data.rec_drops += 1,
                        Err(err) => {
                            // This is a json parse error
                            data.rec_drops += 1;
                            upvl_log::log_error(
                                LOG_TAG,
                                format!("Cannot convert payload to JSON. Error: {}", err),
                            );
                        }
                    }
                }
                Err(err) => {
                    // This is a utf8 conversion error
                    data.rec_drops += 1;
                    upvl_log::log_error(
                        LOG_TAG,
                        format!("Cannot convert payload to string. Error: {}", err),
                    );
                }
            }
        });

        // Create and install a closure to handle publish confirmation.
        // _ is used for an argument when it is not used in the body.
        // This way, the Rust compiler knows not to allocate space for it.
        mc.on_publish(|data, mid| {
            data.pub_count += 1;
            upvl_log::log_info(
                LOG_TAG,
                format!("{}Published message with id: {}", PUB_PREFIX, mid),
            );
        });

        // TODO: look into reconnect_delay_set, which has an
        // "exponential_backoff" flag :-)
        mc.on_disconnect(|data, status_code| {
            upvl_log::log_info(
                LOG_TAG,
                format!(
                    "Disconnected from MQTT broker. Status: {}.\n{}\n",
                    status_code, data
                ),
            );
        });

        // Set up subscriptions when the UPVL is (re-)connected to the broker.
        mc.on_connect(|data, status| {
            if status == 0 {
                upvl_log::log_info(LOG_TAG, "(Re-)connected to broker".to_string());

                //Subscribe to our topics.
                // We need to re-subscribe, since the broker may not
                // remember us.
                data.subscribers = Some(self.subscribe());
            } else {
                upvl_log::log_info(
                    LOG_TAG,
                    format!(
                        "Cannot connect to broker: {}",
                        mosquitto_client::sys::connect_error(status)
                    ),
                );
            }
        });

        // The MQTT client event loop is the main loop in UPVL.
        // It will automatically re-connect.
        //
        // TODO: add the back-off strategy to re-connect.
        // TODO: implement graceful shutdown, eg, from a signal. (not needed for
        //       M1, I think)
        self.loop_forever();
    }
}
