use crate::{cache::Cache, mqtt_client::*};
use std::collections::HashSet;
use std::rc::Rc;
use uic_log::log_error;
use uic_log::log_debug;

const TAG: &str = "mqtt_handler";

/// MqttHandler handles subscription messages related to UIC groups.
/// It subscribes to listen to the correct topics and is to be passed to the mosquitto client
pub struct MqttHandler<'a> {
    /// Reference counted to the mosquitto-client. Notice that its lifetime spans as long as mqttHandler itself.
    /// e.g. the mqtt_client is locked for immutablilty as long as the MqttHandler exists.
    mqtt_client: Rc<dyn MqttClient<'a>>,
    /// Lookup cache for groups and unids
    cache: Cache,
    // matchers that are used to scan incoming mqtt messages
    reported_group_list_matcher: TopicMatcher<'a>,
    reported_names_matcher: TopicMatcher<'a>,
    supported_commands: TopicMatcher<'a>,
}

impl<'a> MqttMessageReceived for MqttHandler<'a> {
    /// public function to be called by the mosquitto fwk on a received message.
    fn on_message(&mut self, msg: MosqMessage) {
        if let Err(e) = self.process_message(msg) {
            log_error(TAG, e);
        }
    }
}

impl<'a> MqttHandler<'a> {
    /// crate new handler to be passed into a MqttClient. The native mqtt client is passed as reference counted.
    /// Reference counted objects can be shared and borrowed over multiple instances. Acquirring mutablity is only possible
    /// when the ref count == 1. since this rc is shared between `MqttClient` and `MqttHandler`, reference count can never be == 1.
    /// hence mutable cannot be accuired for mqtt client.
    pub fn new(
        client: Rc<dyn MqttClient<'a>>,
        matchers: (TopicMatcher<'a>, TopicMatcher<'a>, TopicMatcher<'a>),
    ) -> Self {
        MqttHandler {
            mqtt_client: client,
            cache: Cache::new(),
            reported_group_list_matcher: matchers.0,
            reported_names_matcher: matchers.1,
            supported_commands: matchers.2,
        }
    }

    fn process_message(&mut self, msg: MosqMessage) -> std::result::Result<(), String> {
        if msg.text().is_empty() {
            // Ignore unretain message
            log_debug(TAG,  format!("Unretain of {}", msg.topic()).to_string() );
            Ok(())
        } else {
            let json = json::parse(msg.text())
            .map_err(|e| format!("could not parse body of topic '{}' : {}", msg.topic(), e))?;

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
    }

    /// parse group topic message and extract unid and endpoint id
    fn get_unid_ep_of_topic(topic: &str) -> std::result::Result<(String, u16), String> {
        let list = topic.split_terminator('/').collect::<Vec<&str>>();
        let unid = list[2];
        let stripped = list[3]
            .strip_prefix("ep")
            .ok_or(format!("invalid ep formatting of {}", list[3]))?;
        let ep_number = stripped
            .parse::<u16>()
            .map_err(|e| format!("could not parse {} to integer. {}", stripped, e))?;

        Ok((unid.to_string(), ep_number))
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
        let (unid, ep) = MqttHandler::get_unid_ep_of_topic(topic)?;
        let value = &json["value"];
        let group_list: Vec<u16> = value.members().filter_map(|v| v.as_u16()).collect();
        let changes = self.cache.set_group_list_for_node(&unid, ep, &group_list);

        for change in changes {
            let topic = format!("ucl/by-group/{}/NodeList/{}", change.group_id, &change.unid);
            let mut payload = Vec::new();
            if !change.is_unretain() {
                let json_object = json::object! {
                    "value" => json::JsonValue::from( change.endpoints ),
                };
                payload = json::stringify(json_object).as_bytes().to_vec();
            }
            self.mqtt_client
                .publish(&topic, &payload, 0, true)
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
                .mqtt_client
                .publish(&topic, data.dump().as_bytes(), 0, false)
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
            .mqtt_client
            .publish(&topic, data.dump().as_bytes(), 0, true)
            .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;

        Ok(())
    }

    fn on_supported_commands(
        &mut self,
        topic: &str,
        json: json::JsonValue,
    ) -> std::result::Result<(), String> {
        let (unid, ep) = MqttHandler::get_unid_ep_of_topic(topic)?;
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
                    .get_endpoint_cluster_supported_commands(&endpoint, cluster_name)
                {
                    command_sets.push(commands);
                }
            }
            if let Some((first, rest)) = command_sets.split_first() {
                // Find the intersection of all the sets in our vector.
                let common_commands: Vec<String> = rest
                    .into_iter()
                    .fold(first.clone(), |acc, s| {
                        acc.intersection(&s).cloned().collect()
                    })
                    .into_iter()
                    .collect();

                let json_object = json::object! {
                    "value" => json::JsonValue::from( common_commands ),
                };


                let pub_topic =
                    format!("ucl/by-group/{}/{}/SupportedCommands", group, cluster_name);
                let _msg_id = self
                    .mqtt_client
                    .publish(
                        &pub_topic,
                        json::stringify(json_object).as_bytes(),
                        0,
                        true,
                    )
                    .map_err(|e| format!("error publishing to '{}' : {}", pub_topic, e))?;
            }
        }

        Ok(())
    }
}

#[cfg(test)]
/// Implementing mocks for MqttClient will result in lifetime issues. MqttClient has a limited lifetime,
/// because of the borrows inside of the native mosquitto client itself. it can be moved while
// the borrow still exists in the MqttHandler. Solution would be to declare the mqttclient-mock static with
// interior mutability (see RefCell), maybe a solution with Pinning can prohibit the instance to be moved and
// therefore garuantuee to be valid for the whole lifetime of the mqtthandler
mod tests {}
