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
use crate::cache::Cache;
use unify_application_monitoring_sys::unify_application_monitoring_init;
use std::collections::BTreeMap;
use std::collections::HashSet;
use unify_log_sys::*;
use unify_mqtt_sys::{
    sl_status_t, MosqMessage, MqttClientCallbacksTrait, MqttClientTrait, TopicMatcherType,
};
use unify_validator_sys::*;

const EMPTY_ARRAY_VALUE: &str = "{\"value\":[]}";
/// MqttGroupHandler handles subscription messages related to Unify groups.
/// It subscribes to listen to the correct topics and is to be passed to the mosquitto client
pub struct MqttGroupHandler<T: MqttClientTrait> {
    client: T,
    /// Lookup cache for groups and unids
    cache: Cache,
    // matchers that are used to scan incoming mqtt messages
    reported_group_list_matcher: TopicMatcherType,
    reported_names_matcher: TopicMatcherType,
    supported_commands: TopicMatcherType,
}

declare_app_name!("mqtt_handler");

impl<T: MqttClientTrait> MqttClientCallbacksTrait for MqttGroupHandler<T> {
    /// public function to be called by the mosquitto fwk on a received message.
    fn on_message(&mut self, msg: MosqMessage) {
        if let Err(e) = self.process_message(msg) {
            if !e.is_empty() {
                log_error!("{}", e);
            }
        }
    }
    fn before_disconnect(&mut self) {}
    fn after_connect(&mut self) {
        unify_application_monitoring_init();
    }
}

impl<T: MqttClientTrait> MqttGroupHandler<T> {
    /// crate new handler to be passed into a MqttClient. The native mqtt client is passed as reference counted.
    /// Reference counted objects can be shared and borrowed over multiple instances. Acquirring mutablity is only possible
    /// when the ref count == 1. since this rc is shared between `MqttClient` and `MqttGroupHandler`, reference count can never be == 1.
    /// hence mutable cannot be accuired for mqtt client.
    pub fn new(client: T) -> Result<Self, sl_status_t> {
        let reported_group_list =
            client.subscribe("ucl/by-unid/+/+/Groups/Attributes/GroupList/Reported")?;
        let reported_names =
            client.subscribe("ucl/by-unid/+/+/Groups/Attributes/+/Name/Reported")?;
        let supported_commands = client.subscribe("ucl/by-unid/+/+/+/SupportedCommands")?;

        Ok(MqttGroupHandler {
            client: client,
            cache: Cache::new(),
            reported_group_list_matcher: reported_group_list,
            reported_names_matcher: reported_names,
            supported_commands: supported_commands,
        })
    }

    fn process_message(&mut self, msg: MosqMessage) -> std::result::Result<(), String> {
        log_debug!(
            "Incoming MQTT Publication - {} - Payload : {}",
            msg.topic(),
            msg.text()
        );
        let payload: &str;

        // Unretained messages means no group data. (empty group list/name/commands)
        // In this case, we create a substitute payload to clear states.
        if msg.text().is_empty() {
            log_debug!("Unretain of {}", msg.topic());
            if (self.reported_group_list_matcher.matches(&msg))
                || (self.supported_commands.matches(&msg))
            {
                payload = EMPTY_ARRAY_VALUE;
            } else {
                log_debug!("No update performed for name unretain publication.");
                return Ok(());
            }
        } else {
            // If not empty, take the payload from the publication.
            payload = msg.text();
        }

        // Get our payload into a JSON object.
        let json: json::JsonValue = json::parse(payload).map_err(|e| {
            format!(
                "Could not parse payload for topic '{}' : {}",
                msg.topic(),
                e
            )
        })?;

        // Now dispatch the payload/topic to the handler
        if self.reported_group_list_matcher.matches(&msg) {
            self.on_group_list_reported(msg.topic(), json)
        } else if self.reported_names_matcher.matches(&msg) {
            self.on_name_reported(msg.topic(), json)
        } else if self.supported_commands.matches(&msg) {
            self.on_supported_commands(msg.topic(), json)
        } else {
            Err(format!("no handler for subscribed topic {}", msg.topic()))
        }
    }

    /// process a ucl/by-unid/+/+/Groups/Attributes/GroupList/Reported topic
    /// stores a list of groups for an unid and publishes its endpoints on the
    /// ucl/by-group/<groupid>/NodeList/<unid>
    /// expects endpoint to be prefixed with ep. e.g. ep0
    fn on_group_list_reported(
        &mut self,
        topic: &str,
        json: json::JsonValue,
    ) -> std::result::Result<(), String> {
        let (unid, ep) = get_unid_ep_of_topic(topic)?;
        if false == validate_unid(unid.as_str()) {
            return Err(String::from("Invalid UNID provided"));
        }
        let value = &json["value"];
        let group_list: Vec<u16> = value.members().filter_map(|v| v.as_u16()).collect();
        let changes = self.cache.set_group_list_for_node(&unid, ep, &group_list);

        for change in changes {
            let mut payload = Vec::new();
            if !change.is_unretain() {
                let json_object = json::object! {
                    "value" => json::JsonValue::from( change.endpoints ),
                };
                payload = json::stringify(json_object).as_bytes().to_vec();
            } else {
                if self.cache.get_endpoints_for_group(&change.group_id) == None {
                    self.cache.remove_group_name(change.group_id);
                    let topic = format!("ucl/by-group/{}/GroupName", &change.group_id);
                    self.client
                        .publish(&topic, &payload, true)
                        .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
                }
                if let Some(supported_commands) = self
                    .cache
                    .get_endpoints_supported_commands_per_cluster(vec![(unid.clone(), ep)])
                {
                    self.publish_commands_by_cluster(&change.group_id, supported_commands, true)?;
                }
            }
            let topic = format!(
                "ucl/by-group/{}/NodeList/{}",
                &change.group_id, &change.unid
            );
            self.client
                .publish(&topic, &payload, true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
            if let Some(supported_commands) = self
                .cache
                .get_group_supported_commands_per_cluster(&change.group_id)
            {
                self.publish_commands_by_cluster(&change.group_id, supported_commands, false)?;
            }
        }
        Ok(())
    }

    fn publish_commands_by_cluster(
        &mut self,
        group_id: &u16,
        supported_commands: BTreeMap<String, HashSet<String>>,
        is_unretain: bool,
    ) -> std::result::Result<(), String> {
        for (cluster, commands) in supported_commands {
            let mut payload = Vec::new();
            let topic = format!("ucl/by-group/{}/{}/SupportedCommands", &group_id, cluster);
            if !is_unretain {
                let vec_commands = commands.into_iter().collect::<Vec<String>>();
                let json_object = json::object! {
                    "value" => json::JsonValue::from(vec_commands),
                };
                payload = json::stringify(json_object).as_bytes().to_vec();
            }
            self.client
                .publish(&topic, &payload, true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
        }
        Ok(())
    }

    /// handler for topic ucl/by-unid/+/+/Groups/Attributes/+/Name/Reported.
    /// overwrites name for group id.
    fn on_name_reported(
        &mut self,
        topic: &str,
        json: json::JsonValue,
    ) -> std::result::Result<(), String> {
        if let json::JsonValue::Object(obj) = json {
            let val_obj = obj.get("value").ok_or(format!(
                "dropping {}. \"value\" key not found in body of topic",
                topic
            ))?;
            let name = val_obj
                .as_str()
                .ok_or(format!("value of \"value\" key not a string {}", topic))?;
            let group_str = topic.split_terminator('/').collect::<Vec<&str>>()[6];
            let group_id = group_str
                .parse::<u16>()
                .map_err(|e| format!("could not parse {} to integer. {}", group_str, e))?;

            self.send_add_group_if_required(group_id, name)?;

            if self.cache.update_group_name(group_id, name) {
                self.send_group_name(group_id, name)?;
            }
            Ok(())
        } else {
            Err(format!("could not parse body of topic {}", topic))
        }
    }
    /// add_group is only send when an update of an groupname is reported
    fn send_add_group_if_required(
        &self,
        group_id: u16,
        group_name: &str,
    ) -> std::result::Result<(), String> {
        if group_name.is_empty() {
            return Ok(());
        }

        if let Some(name) = self.cache.get_group_name(group_id) {
            if name == group_name {
                return Ok(());
            }

            let topic = format!("ucl/by-group/{}/Groups/Commands/AddGroup", &group_id);
            let data = json::object! {
                "GroupId" => group_id,
                "GroupName" => group_name,
            };

            let _msg_id = self
                .client
                .publish(&topic, data.dump().as_bytes(), false)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
        }
        Ok(())
    }

    fn send_group_name(&self, group_id: u16, group_name: &str) -> std::result::Result<(), String> {
        let topic = format!("ucl/by-group/{}/GroupName", &group_id);
        let data = json::object! {
            "value" => group_name,
        };

        let _msg_id = self
            .client
            .publish(&topic, data.dump().as_bytes(), true)
            .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;

        Ok(())
    }

    fn on_supported_commands(
        &mut self,
        topic: &str,
        json: json::JsonValue,
    ) -> std::result::Result<(), String> {
        let (unid, ep) = get_unid_ep_of_topic(topic)?;
        if false == validate_unid(unid.as_str()) {
            return Err(String::from("Invalid UNID provided"));
        }
        let cluster_name = topic.split_terminator('/').collect::<Vec<&str>>()[4];
        let value = &json["value"];
        let command_names: HashSet<String> = value.members().map(|c| c.to_string()).collect();
        self.cache.set_endpoint_cluster_supported_commands(
            (unid.clone(), ep),
            cluster_name,
            command_names,
        );

        for group in self.cache.get_group_list_for_node(&unid, ep) {
            let mut command_sets: Vec<HashSet<String>> = Vec::new();
            for endpoint in self.cache.get_endpoints_for_group(&group).iter().flatten() {
                if let Some(commands) = self
                    .cache
                    .get_endpoint_cluster_supported_commands(endpoint, cluster_name)
                {
                    command_sets.push(commands);
                }
            }
            if let Some((first, rest)) = command_sets.split_first() {
                // Find the intersection of all the sets in our vector.
                let common_commands: Vec<String> = rest
                    .iter()
                    .fold(first.clone(), |acc, s| {
                        acc.intersection(s).cloned().collect()
                    })
                    .into_iter()
                    .collect();

                let json_object = json::object! {
                    "value" => json::JsonValue::from( common_commands ),
                };

                let pub_topic =
                    format!("ucl/by-group/{}/{}/SupportedCommands", group, cluster_name);
                let _msg_id = self
                    .client
                    .publish(&pub_topic, json::stringify(json_object).as_bytes(), true)
                    .map_err(|e| format!("error publishing to '{}' : {}", pub_topic, e))?;
            }
        }

        Ok(())
    }
}

/// parse group topic message and extract unid and endpoint id
fn get_unid_ep_of_topic(topic: &str) -> std::result::Result<(String, u16), String> {
    let list = topic.split_terminator('/').collect::<Vec<&str>>();
    let unid = list[2];
    let stripped = list[3].strip_prefix("ep").ok_or("")?;
    let ep_number = stripped
        .parse::<u16>()
        .map_err(|e| format!("could not parse {} to integer. {}", stripped, e))?;

    Ok((unid.to_string(), ep_number))
}

#[cfg(test)]
mod tests {
    use super::*;
    use mockall::predicate;
    use mockall::predicate::eq;
    use unify_mqtt_sys::MockMqttClientTrait;
    use unify_mqtt_sys::MockTopicMatcherTrait;

    fn create_mocked_mqtt_handler() -> MqttGroupHandler<MockMqttClientTrait> {
        let mut mqtt_client_mock = MockMqttClientTrait::new();
        let group_list_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/by-unid/+/+/Groups/Attributes/GroupList/Reported"))
            .return_once_st(|_| group_list_topic_matcher_mock);
        let name_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/by-unid/+/+/Groups/Attributes/+/Name/Reported"))
            .return_once_st(|_| name_topic_matcher_mock);
        let supported_commands_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/by-unid/+/+/+/SupportedCommands"))
            .return_once_st(|_| supported_commands_topic_matcher_mock);

        MqttGroupHandler::new(mqtt_client_mock).unwrap()
    }
    #[test]
    fn test_get_unid_ep_of_topic() {
        {
            let test_topic = "ucl/by-unid/zw-33/ep0/Groups/Attributes/GroupList/Reported";
            let (unid_test, ep_test) = get_unid_ep_of_topic(test_topic).unwrap();
            assert_eq!(unid_test, "zw-33".to_string());
            assert_eq!(ep_test, 0);
        }
        {
            let test_topic = "ucl/by-unid/zw-33/invalid_ep_id/Groups/Attributes/GroupList/Reported";
            assert_eq!(get_unid_ep_of_topic(test_topic), Err(String::from("")));
        }
        {
            let test_topic =
                "ucl/by-unid/zw-33/epinvaliddecimanvalue/Groups/Attributes/GroupList/Reported";
            assert_eq!(
                get_unid_ep_of_topic(test_topic),
                Err(String::from(
                    "could not parse invaliddecimanvalue to integer. invalid digit found in string"
                ))
            );
        }
    }

    #[test]
    fn teston_group_list_reported() {
        let mut mqtt_group_handler = create_mocked_mqtt_handler();
        let mock_topic = "ucl/by-unid/zw-33/ep0/Groups/Attributes/GroupList/Reported";
        let json_object = json::object! {
            "value" => json::JsonValue::from(vec![1]),
        };
        let json_object_exp = predicate::function(|x| {
            x == json::object! {
            "value" => json::JsonValue::from(vec![0])}
            .dump()
            .as_bytes()
        });

        mqtt_group_handler
            .client
            .expect_publish()
            .with(
                eq("ucl/by-group/1/NodeList/zw-33"),
                json_object_exp,
                eq(true),
            )
            .return_once_st(|_, _, _| Ok(()));
        mqtt_group_handler
            .on_group_list_reported(mock_topic, json_object)
            .unwrap();
    }
    #[test]
    fn test_publish_commands_by_cluster() {
        let mut mqtt_group_handler = create_mocked_mqtt_handler();

        let mut supported_commands_per_cluster: BTreeMap<String, HashSet<String>> = BTreeMap::new();
        let supported_commands = supported_commands_per_cluster
            .entry("OnOff".to_string())
            .or_insert(HashSet::new());
        supported_commands.insert("On".to_string());
        let group_id_test: u16 = 1;

        let json_object_list_commands_exp = predicate::function(|x| {
            x == json::object! {
            "value" => json::JsonValue::from(vec!["On"])}
            .dump()
            .as_bytes()
        });

        mqtt_group_handler
            .client
            .expect_publish()
            .with(
                eq("ucl/by-group/1/OnOff/SupportedCommands"),
                json_object_list_commands_exp,
                eq(true),
            )
            .return_once_st(|_, _, _| Ok(()));

        mqtt_group_handler
            .publish_commands_by_cluster(&group_id_test, supported_commands_per_cluster, false)
            .unwrap();
    }
    #[test]
    fn test_on_name_reported() {
        let mut mqtt_group_handler = create_mocked_mqtt_handler();
        let topic_test = "ucl/by-unid/zw-001/ep0/Groups/Attributes/1/Name/Reported";
        let json_object = json::object! {
            "value" => json::JsonValue::from("Kichen Group Name"),
        };

        let payload_exp = predicate::function(|x| {
            let test_group_name: &str = "Kichen Group Name";
            x == json::object! {
                "value" => test_group_name,
            }
            .dump()
            .as_bytes()
        });
        mqtt_group_handler
            .client
            .expect_publish()
            .with(eq("ucl/by-group/1/GroupName"), payload_exp, eq(true))
            .return_once_st(|_, _, _| Ok(()));

        mqtt_group_handler
            .on_name_reported(topic_test, json_object)
            .unwrap();
    }

    #[test]
    fn test_send_group_name() {
        let mut mqtt_group_handler = create_mocked_mqtt_handler();
        let test_group_name: &str = "Kichen Group Name";
        let test_group_id: u16 = 3;

        let payload_exp = predicate::function(|x| {
            x == json::object! {
                "value" => "Kichen Group Name",
            }
            .dump()
            .as_bytes()
        });
        mqtt_group_handler
            .client
            .expect_publish()
            .with(eq("ucl/by-group/3/GroupName"), payload_exp, eq(true))
            .return_once_st(|_, _, _| Ok(()));

        mqtt_group_handler
            .send_group_name(test_group_id, test_group_name)
            .unwrap();
    }
    #[test]
    fn test_send_add_group_if_required() {
        let mut mqtt_group_handler = create_mocked_mqtt_handler();
        let test_group_name: &str = "Living Group Name";
        let test_group_id: u16 = 5;
        // mocking goup_id 5 with empty group name
        mqtt_group_handler.cache.update_group_name(5, " ");

        let payload_exp = predicate::function(|x| {
            x == json::object! {
                "GroupId" => 5,
                "GroupName" => "Living Group Name",
            }
            .dump()
            .as_bytes()
        });
        mqtt_group_handler
            .client
            .expect_publish()
            .with(
                eq("ucl/by-group/5/Groups/Commands/AddGroup"),
                payload_exp,
                eq(false),
            )
            .return_once_st(|_, _, _| Ok(()));

        mqtt_group_handler
            .send_add_group_if_required(test_group_id, test_group_name)
            .unwrap();
    }
    #[test]
    fn test_on_supported_commands() {
        let mut mqtt_group_handler = create_mocked_mqtt_handler();
        let topic_test = "ucl/by-unid/zw-ZWCDEF-001/ep0/Groups/SupportedCommands";
        let json_object = json::object! {
            "value" => json::JsonValue::from(vec!["AddGroup"]),
        };
        mqtt_group_handler
            .cache
            .set_group_list_for_node("zw-ZWCDEF-001", 0, &[3]);

        let payload_exp = predicate::function(|x| {
            x == json::object! {
                "value" => json::JsonValue::from(vec!["AddGroup"]),
            }
            .dump()
            .as_bytes()
        });
        mqtt_group_handler
            .client
            .expect_publish()
            .with(
                eq("ucl/by-group/3/Groups/SupportedCommands"),
                payload_exp,
                eq(true),
            )
            .return_once_st(|_, _, _| Ok(()));

        mqtt_group_handler
            .on_supported_commands(topic_test, json_object)
            .unwrap();
    }
    #[test]
    fn test_on_supported_commands_invalid_unid() {
        let mut mqtt_group_handler = create_mocked_mqtt_handler();
        let topic_invalid_unid = "ucl/by-unid/-_-/ep0/Groups/SupportedCommands";
        let json_object = json::object! {
            "value" => json::JsonValue::from(vec!["AddGroup"]),
        };

        assert_eq!(
            mqtt_group_handler.on_supported_commands(topic_invalid_unid, json_object),
            Err(String::from("Invalid UNID provided"))
        );
    }
}
